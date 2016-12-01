#include "funciones.h"

void srdf() {
	int i;
	while (1) {
		sem_wait(&contadorEntrenadoresListos);
		if (!queue_is_empty(listos)) {
			int * turno = malloc(sizeof(int));
			int * movimientos = malloc(sizeof(int));
			*turno = entrenadorMasCercano(movimientos);
			log_info(logger, "el turno es: %d", *turno);
			log_info(logger, "movimientos es: %d", *movimientos);
			int quedoBloqueado = 0;
			for (i = 0; i < *movimientos; i++) {
				pthread_mutex_lock(&mutex);
				jugada(*turno, &quedoBloqueado, &i, *movimientos);
				pthread_mutex_unlock(&mutex);
			}
			free(movimientos);
			if (!quedoBloqueado) {
				queue_push(listos, (void *) turno);
				sem_post(&contadorEntrenadoresListos);
			}
		}
	}
}

void roundRobin() {
	int i;
	while (1) {
		sem_wait(&contadorEntrenadoresListos);
		log_info(logger, "corre hilo planificador");
		if (!queue_is_empty(listos)) {
			int * turno;
			turno = (int *) queue_pop(listos);
			int quedoBloqueado = 0;
			for (i = 0; i < configuracion->quantum; i++) {
				pthread_mutex_lock(&mutex);
				jugada(*turno, &quedoBloqueado, &i, configuracion->quantum);
				pthread_mutex_unlock(&mutex);
			}
			if (!quedoBloqueado) {
				queue_push(listos, (void *) turno);
				sem_post(&contadorEntrenadoresListos);
			} else {
				free(turno);
			}
		}
	}
}

void hiloPokenest(void * parametros) {

	//parametros de la pokenest
	t_metadataPokenest * pokenest = (t_metadataPokenest *) parametros;

	while (1) {
		sem_wait(pokenest->semaforoPokenest);
		if (!queue_is_empty(pokenest->colaPokenest)) {
			int * socketEntrenador = (int *) queue_pop(pokenest->colaPokenest);
			int * numeroPokemon = malloc(sizeof(int));
			int * indice = malloc(sizeof(int));

			//me fijo si hay pokemones disponibles
			sem_wait(pokenest->disponiblesPokenest);
			log_info(logger, "corre hilo pokenest: %c", pokenest->identificador);
			if (pokemonDisponible(devolverIndicePokenest(pokenest->identificador), pokenest->identificador, numeroPokemon,indice)) {
				//aviso a entrenador que hay pokemones
				if (enviarHeader(*socketEntrenador, pokemonesDisponibles)) {
					//error, se desconecto
					desconectadoOFinalizado(*socketEntrenador);
				}else if (enviarTodo(*socketEntrenador, numeroPokemon, sizeof(int))) {
					//error, se desconecto
					desconectadoOFinalizado(*socketEntrenador);
				} else {

					int header = recibirHeader(*socketEntrenador);
					log_info(logger, "el header es: %d", header);
					//entrenador avisa que ya lo copio
					switch (header) {
					case entrenadorListo:
						;
						log_info(logger, "atrapo el pokemon: %c", pokenest->identificador);
						//hago cosas correspondientes a que lo atrapo
						t_duenioPokemon * pokemon = (t_duenioPokemon *) list_get(pokemones, *indice);
						pokemon->socketEntrenador = *socketEntrenador;
						restarRecursoDisponible(devolverIndicePokenest(pokenest->identificador));
						//todo restarPokemon(pokenest->identificador);
						sumarAsignadosMatriz(devolverIndiceEntrenador(*socketEntrenador),devolverIndicePokenest(pokenest->identificador));
						int * elTurno = malloc(sizeof(int));
						*elTurno = *socketEntrenador;
						queue_push(listos, (void *) elTurno);
						sem_post(&contadorEntrenadoresListos);
						log_info(logger, "Pasa a listo el entrenador del socket: %d", *socketEntrenador);
						//todo dibujar(nombreMapa);
						break;

					case finalizoMapa:
						;
						//atrapa el pokemon
						t_duenioPokemon * Mipokemon = (t_duenioPokemon *) list_get(pokemones, *indice);
						Mipokemon->socketEntrenador = *socketEntrenador;
						restarRecursoDisponible(devolverIndicePokenest(pokenest->identificador));
						//todo restarPokemon(pokenest->identificador);
						sumarAsignadosMatriz(devolverIndiceEntrenador(*socketEntrenador),devolverIndicePokenest(pokenest->identificador));
						//todo dibujar(nombreMapa);
						//devuelvo todos sus recursos
						desconectadoOFinalizado(*socketEntrenador);
						log_info(logger, "Termino el mapa el entrenador del socket: %d", *socketEntrenador);
						break;

					default:
						//problema en el header, asumo que se desconecto
						log_error(logger, "Se desconecto el entrenador del socket: ", *socketEntrenador);
						sem_post(pokenest->disponiblesPokenest);
						desconectadoOFinalizado(*socketEntrenador);
						break;
					}
					free(socketEntrenador);
				}
			} else {
				queue_push(pokenest->colaPokenest, (void *) socketEntrenador);
				sem_post(pokenest->disponiblesPokenest);
				sem_post(pokenest->semaforoPokenest);
			}
			free(numeroPokemon);
			free(indice);
		}
	}
}
int hayEntrenadorSinDistancia(int * indice) {
	int i;
	for (i = 0; i < list_size(Entrenadores); i++) {
		t_datosEntrenador * entrenador = (t_datosEntrenador *) list_get(Entrenadores, i);
		if (entrenador->distanciaAPokenest == 0) {
			*indice = i;
			log_error(logger, "el entrenador sin distancia es: %s", entrenador->nombre);
			return 1;
			break;
		}
	}
	return 0;
}

int entrenadorMasCercano(int * movimientos) {
	int i;
	int menor = 200;
	int indice;
	t_datosEntrenador * entrenadorADevolver;
	//me fijo si hay uno sin distancia
	if (hayEntrenadorSinDistancia(&indice)) {
		t_datosEntrenador * entrenador = (t_datosEntrenador *) list_get(Entrenadores, indice);
		log_info(logger, "el socket es: %d", entrenador->socket);
		*movimientos = 1;
		return entrenador->socket;
	} else {
		//busco el mas cercano
		for (i = 0; i < list_size(Entrenadores); i++) {
			t_datosEntrenador * entrenador = (t_datosEntrenador *) list_get(Entrenadores, i);
			if (entrenador->distanciaAPokenest < menor) {
				menor = entrenador->distanciaAPokenest;
				entrenadorADevolver = entrenador;
			}
		}
		//lo saco de la cola de listos
		for (i = 0; i < queue_size(listos); i++) {
			int * sockete = (int *) queue_pop(listos);
			if (*sockete == entrenadorADevolver->socket) {
				free(sockete);
				break;
			} else {
				queue_push(listos, (void *) sockete);
			}
		}
		*movimientos = entrenadorADevolver->distanciaAPokenest;
		return entrenadorADevolver->socket;
	}
}

void jugada(int miTurno, int * quedoBloqueado, int * i, int total) {

	log_info(logger, "pase por aqui turno antes de header: %d", miTurno);
	int header = recibirHeader(miTurno);
	if (header == 0) {
		log_error(logger, "error al recibir header");
		*i = total;
		*quedoBloqueado = 1;
		desconectadoOFinalizado(miTurno);
	} else {
		switch (header) {
		case datosPokenest:
			;
			char identificadorPokenest;
			if (recibirTodo(miTurno, &identificadorPokenest, sizeof(char))) {
				//error al enviar, supongo que se desconecto
				log_error(logger, "error al recibir identificador de pokenest");
				desconectadoOFinalizado(miTurno);
			}
			t_metadataPokenest * pokenest = devolverPokenest(&identificadorPokenest);
			if (enviarCoordPokenest(miTurno, pokenest)) {
				log_error(logger, "error al enviar las coordenadas de la pokenest");
				//error al enviar, supongo que se desconecto
				desconectadoOFinalizado(miTurno);
			}
			t_datosEntrenador * entrenador = devolverEntrenador(miTurno);
			if (recibirTodo(miTurno, &entrenador->distanciaAPokenest, sizeof(int))) {
				log_error(logger, "error al recibir la distancia a la pokenest");
				//error al enviar, supongo que se desconecto
				desconectadoOFinalizado(miTurno);
			}
			log_error(logger, "mi pokenest : %c", identificadorPokenest);
			*i = *i - 1;
			break;

		case posicionEntrenador:
			;
			int * posX = malloc(sizeof(int));
			int * posY = malloc(sizeof(int));
			if (recibirTodo(miTurno, posX, sizeof(int))) {
				log_error(logger, "error al recibir la posicion X");
				//error al enviar, supongo que se desconecto
				desconectadoOFinalizado(miTurno);
			}
			if (recibirTodo(miTurno, posY, sizeof(int))) {
				log_error(logger, "error al recibir la posicion Y");
				//error al enviar, supongo que se desconecto
				desconectadoOFinalizado(miTurno);
			}
			if (movimientoValido(miTurno, *posX, *posY)) {
				log_error(logger, "movimiento invalido");
				enviarHeader(miTurno, movimientoInvalido);
			} else {
				//todo moverEntrenador(*devolverEntrenador(miTurno));
				enviarHeader(miTurno, movimientoAceptado);
				//todo dibujar(nombreMapa);
			}
			free(posX);
			free(posY);
			//sleep(configuracion->retardo/1000);
			break;

		case capturarPokemon:
			;
			char identificadorPokemon;
			if (recibirTodo(miTurno, &identificadorPokemon, sizeof(char))) {
				log_error(logger, "error al recibir el identificador de la pokenest");
				//error al enviar, supongo que se desconecto
				desconectadoOFinalizado(miTurno);
			} else {
				log_info(logger, "Quedo bloqueado el entrenador del socket: %d", miTurno);
				t_metadataPokenest * pokenest = devolverPokenest(&identificadorPokemon);
				int * socketEntrenador = malloc(sizeof(int));
				*socketEntrenador = miTurno;
				queue_push(pokenest->colaPokenest, (void *) socketEntrenador);
				sumarPedidosMatriz(devolverIndiceEntrenador(miTurno), devolverIndicePokenest(pokenest->identificador));
				*i = total;
				*quedoBloqueado = 1;
				sem_post(pokenest->semaforoPokenest);
			}
			break;
		}
	}

}
