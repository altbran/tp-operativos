#include "funciones.h"

void srdf() {
	int i;
	while (1) {
		sem_wait(&contadorEntrenadoresListos);
		if (!queue_is_empty(listos)) {
			int * turno = malloc(sizeof(int));
			int * movimientos = malloc(sizeof(int));
			*turno = entrenadorMasCercano(movimientos);
			log_info(logPlanificador, "el turno es: %d", *turno);
			log_info(logPlanificador, "movimientos es: %d", *movimientos);
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
		log_info(logPlanificador, "corre hilo planificador round");
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

	t_log* logPokenest = log_create(concat(2, &pokenest->identificador, ".log"), "MAPA", 0, log_level_from_string("INFO"));

	while (1) {
		//me fijo si hay pokemones disponibles
		sem_wait(pokenest->disponiblesPokenest);
		log_info(logPokenest, "corre hilo pokenest (hay pokemons)");
		int * numeroPokemon = malloc(sizeof(int));
		int * indice = malloc(sizeof(int));
		if (pokemonDisponible(devolverIndicePokenest(pokenest->identificador), pokenest->identificador, numeroPokemon, indice)) {

			//semaforo de entrenadores
			sem_wait(pokenest->semaforoPokenest);
			if (!queue_is_empty(pokenest->colaPokenest)) {
				int * socketEntrenador = (int *) queue_pop(pokenest->colaPokenest);
				log_info(logPokenest, "Entro al hilo el socket: %d", *socketEntrenador);
				//log_info(logger, "en la pokenest: %c", pokenest->identificador);

				//aviso a entrenador que hay pokemones
				if (enviarHeader(*socketEntrenador, pokemonesDisponibles)) {
					log_error(logPokenest, "Error al enviar pokemon disp pokenest");
					//error, se desconecto
					desconectadoOFinalizado(*socketEntrenador);
				} else if (enviarTodo(*socketEntrenador, numeroPokemon, sizeof(int))) {
					log_error(logPokenest, "Error al enviar el numero de pokemon disp pokenest");
					//error, se desconecto
					desconectadoOFinalizado(*socketEntrenador);
				} else {

					int header = recibirHeader(*socketEntrenador);
					//log_info(logger, "recibido del socket %d en la pokenest %s",*socketEntrenador, pokenest->identificador);
					log_info(logPokenest, "el header es: %d", header);
					log_info(logPokenest, "del socket: %d", *socketEntrenador);
					//entrenador avisa que ya lo copio
					switch (header) {
					case entrenadorListo:
						;
						log_info(logPokenest, "atrapo el pokemon");
						//hago cosas correspondientes a que lo atrapo
						t_duenioPokemon * pokemon = (t_duenioPokemon *) list_get(pokemones, *indice);
						pokemon->socketEntrenador = *socketEntrenador;
						restarRecursoDisponible(devolverIndicePokenest(pokenest->identificador));
						//todo restarPokemon(pokenest->identificador);
						sumarAsignadosMatriz(devolverIndiceEntrenador(*socketEntrenador),
								devolverIndicePokenest(pokenest->identificador));
						int * elTurno = malloc(sizeof(int));
						*elTurno = *socketEntrenador;
						queue_push(listos, (void *) elTurno);
						sem_post(&contadorEntrenadoresListos);
						log_info(logPokenest, "Pasa a listo el entrenador");
						//todo dibujar(nombreMapa);
						break;

					case finalizoMapa:
						;
						//atrapa el pokemon
						t_duenioPokemon * Mipokemon = (t_duenioPokemon *) list_get(pokemones, *indice);
						Mipokemon->socketEntrenador = *socketEntrenador;
						restarRecursoDisponible(devolverIndicePokenest(pokenest->identificador));
						//todo restarPokemon(pokenest->identificador);
						sumarAsignadosMatriz(devolverIndiceEntrenador(*socketEntrenador),
								devolverIndicePokenest(pokenest->identificador));
						//todo dibujar(nombreMapa);
						//devuelvo todos sus recursos
						desconectadoOFinalizado(*socketEntrenador);
						log_info(logPokenest, "Termino el mapa el entrenador del socket: %d", *socketEntrenador);
						break;

					default:
						//problema en el header, asumo que se desconecto
						log_error(logPokenest, "Se desconecto el entrenador del socket: %d", *socketEntrenador);
						sem_post(pokenest->disponiblesPokenest);
						desconectadoOFinalizado(*socketEntrenador);
						break;
					}
					free(socketEntrenador);
				}

			}else{
				sem_post(pokenest->disponiblesPokenest);
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
			log_info(logPlanificador, "el entrenador sin distancia es: %s", entrenador->nombre);
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

	//log_info(logPlanificador, "pase por aqui turno antes de header: %d", miTurno);
	int header = recibirHeader(miTurno);
	if (header == 0) {
		log_error(logPlanificador, "error al recibir header");
		log_error(logPlanificador, "socket %d: ", miTurno);
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
				log_error(logPlanificador, "error al recibir identificador de pokenest");
				desconectadoOFinalizado(miTurno);
				break;
			}
			t_metadataPokenest * pokenest = devolverPokenest(&identificadorPokenest);
			if (enviarCoordPokenest(miTurno, pokenest)) {
				log_error(logPlanificador, "error al enviar las coordenadas de la pokenest");
				//error al enviar, supongo que se desconecto
				desconectadoOFinalizado(miTurno);
				break;
			}
			t_datosEntrenador * entrenador = devolverEntrenador(miTurno);
			if (recibirTodo(miTurno, &entrenador->distanciaAPokenest, sizeof(int))) {
				log_error(logPlanificador, "error al recibir la distancia a la pokenest");
				//error al enviar, supongo que se desconecto
				desconectadoOFinalizado(miTurno);
				break;
			}
			log_info(logPlanificador, "mi pokenest : %c", identificadorPokenest);
			*i = *i - 1;
			break;

		case posicionEntrenador:
			;
			int * posX = malloc(sizeof(int));
			int * posY = malloc(sizeof(int));
			if (recibirTodo(miTurno, posX, sizeof(int))) {
				log_error(logPlanificador, "error al recibir la posicion X");
				//error al enviar, supongo que se desconecto
				desconectadoOFinalizado(miTurno);
				break;
			}
			if (recibirTodo(miTurno, posY, sizeof(int))) {
				log_error(logPlanificador, "error al recibir la posicion Y");
				//error al enviar, supongo que se desconecto
				desconectadoOFinalizado(miTurno);
				break;
			}
			if (movimientoValido(miTurno, *posX, *posY)) {
				log_error(logPlanificador, "movimiento invalido");
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
				log_error(logPlanificador, "error al recibir el identificador de la pokenest");
				//error al enviar, supongo que se desconecto
				desconectadoOFinalizado(miTurno);
				break;
			} else {
				log_info(logPlanificador, "Quedo bloqueado el entrenador del socket: %d", miTurno);
				t_metadataPokenest * pokenest = devolverPokenest(&identificadorPokemon);
				t_datosEntrenador * entrenador = devolverEntrenador(miTurno);
				entrenador->identificadorPokenest = identificadorPokemon;
				int * socketEntrenador = malloc(sizeof(int));
				*socketEntrenador = miTurno;
				sumarPedidosMatriz(devolverIndiceEntrenador(miTurno), devolverIndicePokenest(pokenest->identificador));
				*i = total;
				*quedoBloqueado = 1;
				queue_push(pokenest->colaPokenest, (void *) socketEntrenador);
				sem_post(pokenest->semaforoPokenest);
			}
			break;
		}
	}

}
