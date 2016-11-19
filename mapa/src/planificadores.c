#include "funciones.h"

void srdf() {
	int i;
	while (1) {
		sem_wait(&contadorEntrenadoresListos);
		if (!queue_is_empty(listos)) {
			int * turno;
			int * movimientos = malloc(sizeof(int));
			turno = entrenadorMasCercano(movimientos);
			log_info(logger,"el turno es: %d", *turno);
			log_info(logger,"movimientos es: %d", *movimientos);
			int quedoBloqueado = 0;
			for (i = 0; i < *movimientos; i++) {
				pthread_mutex_lock(&mutex);
				jugada(turno, &quedoBloqueado, &i, *movimientos);
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
		if (!queue_is_empty(listos)) {
			int * turno;
			turno = (int *) queue_pop(listos);
			int quedoBloqueado = 0;
			for (i = 0; i < configuracion->quantum; i++) {
				pthread_mutex_lock(&mutex);
				jugada(turno, &quedoBloqueado, &i, configuracion->quantum);
				pthread_mutex_unlock(&mutex);
			}
			if (!quedoBloqueado) {
				queue_push(listos, (void *) turno);
				sem_post(&contadorEntrenadoresListos);
			}
		}
	}
}

void atraparPokemon() {
	while (1) {
		sem_wait(&contadorEntrenadoresBloqueados);
		if (!queue_is_empty(bloqueados)) {
			t_entrenadorBloqueado * entrenador = (t_entrenadorBloqueado*) queue_pop(bloqueados);
			int * numeroPokemon = malloc(sizeof(int));
			int * indice = malloc(sizeof(int));

			//me fijo si hay pokemones disponibles
			if (pokemonDisponible(devolverIndicePokenest(entrenador->identificadorPokemon), entrenador->identificadorPokemon,
					numeroPokemon, indice)) {

				pthread_mutex_lock(&mutexDeadlock);
				//aviso a entrenador que hay pokemones
				enviarHeader(entrenador->socket, pokemonesDisponibles);
				if (enviarTodo(entrenador->socket, numeroPokemon, sizeof(int))) {
					//error, se desconecto
					desconectadoOFinalizado(entrenador->socket);
				} else {

					int header = recibirHeader(entrenador->socket);
					log_info(logger, "el header es: %d",header);
					//entrenador avisa que ya lo copio
					if (header == entrenadorListo) {
						log_info(logger, "entro vieja: %c",entrenador->identificadorPokemon);
						//hago cosas correspondientes a que lo atrapo
						t_duenioPokemon * pokemon =(t_duenioPokemon *) list_get(pokemones, *indice);
						pokemon->socketEntrenador = entrenador->socket;
						restarRecursoDisponible(devolverIndicePokenest(entrenador->identificadorPokemon));
						restarPokemon(entrenador->identificadorPokemon);
						sumarAsignadosMatriz(devolverIndiceEntrenador(entrenador->socket),devolverIndicePokenest(entrenador->identificadorPokemon));
						int * elTurno = malloc(sizeof(int));
						*elTurno = entrenador->socket;
						queue_push(listos,(void *) elTurno);
						sem_post(&contadorEntrenadoresListos);
						log_info(logger, "Pasa a listo el entrenador del socket: %d", entrenador->socket);
						dibujar(nombreMapa);

						//en caso de terminar el mapa devolver todos los recursos
					} else if (header == finalizoMapa) {
						//desconectadoOFinalizado(entrenador->socket);
						log_info(logger, "Termino el mapa el entrenador del socket: ", entrenador->socket);
					} else {
						//problema en el header, asumo que se desconecto
						log_error(logger, "Se desconecto el entrenador del socket: ", entrenador->socket);
						desconectadoOFinalizado(entrenador->socket);
					}
					pthread_mutex_unlock(&mutexDeadlock);
					free(entrenador);
				}
			} else {
				queue_push(bloqueados,(void *) entrenador);
				sem_post(&contadorEntrenadoresBloqueados);
			}
			free(numeroPokemon);
			free(indice);
		}
	}
}
int hayEntrenadorSinDistancia(int * indice) {
	int i;
	for (i = 0; i < list_size(Entrenadores); i++) {
		t_datosEntrenador * entrenador = list_get(Entrenadores, i);
		if (entrenador->distanciaAPokenest == 0) {
			*indice = i;
			log_error(logger,"el indice es: %d",*indice);
			return 1;
			break;
		}
	}
	return 0;
}

int * entrenadorMasCercano(int * movimientos) {
	int i;
	int menor = 100;
	int * indice = malloc(sizeof(int));
	int * socketMasCercano;
	if (hayEntrenadorSinDistancia(indice)) {
		t_datosEntrenador * entrenador = (t_datosEntrenador *) list_get(Entrenadores, *indice);
		free(indice);
		log_info(logger,"el socket es: %d", entrenador->socket);
		*movimientos = 1;
		return &entrenador->socket;
	} else {
		for (i = 0; i < list_size(Entrenadores); i++) {
			t_datosEntrenador * entrenador = list_get(Entrenadores, i);
			if (entrenador->distanciaAPokenest < menor) {
				menor = entrenador->distanciaAPokenest;
				*indice = i;
			}
		}
		*movimientos = menor;
		t_datosEntrenador * entrenador = list_get(Entrenadores, *indice);
		for (i = 0; i < queue_size(listos); i++) {
			int * sockete = queue_pop(listos);
			if (*sockete == entrenador->socket) {
				socketMasCercano = sockete;
				break;
			} else {
				queue_push(listos,(void *) sockete);
			}
		}
		free(indice);
		return socketMasCercano;
	}
}

void jugada(int * turno, int * quedoBloqueado, int * i, int total) {

	int miTurno = *turno;
	log_info(logger, "pase por aqui turno antes de header: %d", miTurno);
	int header = recibirHeader(miTurno);

	switch (header) {
	case datosPokenest:
		;
		log_error(logger, "mi turno: %d", miTurno);
		identificadorPokenest = malloc(sizeof(char));
			if (recibirTodo(miTurno, &prueba, sizeof(char))) {
				log_error(logger, "mi pokenest: %c", prueba);


			//error al enviar, supongo que se desconecto
			log_error(logger, "error al recibir identificador de pokenest");
			desconectadoOFinalizado(*turno);
		}
		t_metadataPokenest * pokenest = devolverPokenest(&prueba);
		if (enviarCoordPokenest(miTurno, pokenest)) {
			log_error(logger, "error al enviar las coordenadas de la pokenest");
			//error al enviar, supongo que se desconecto
			desconectadoOFinalizado(*turno);
		}
		t_datosEntrenador * entrenador = devolverEntrenador(miTurno);
		if (recibirTodo(miTurno, &entrenador->distanciaAPokenest, sizeof(int))) {
			log_error(logger, "error al recibir la distancia a la pokenest");
			//error al enviar, supongo que se desconecto
			desconectadoOFinalizado(*turno);
		}
		log_error(logger, "mi pokenest antes del free: %c", prueba);
		free(identificadorPokenest);
		log_error(logger, "mi pokenest despues del free: %c", prueba);
		*i = *i - 1;
		break;

	case posicionEntrenador:
		;
		int * posX = malloc(sizeof(int));
		int * posY = malloc(sizeof(int));
		if (recibirTodo(*turno, posX, sizeof(int))) {
			log_error(logger, "error al recibir la posicion X");
			//error al enviar, supongo que se desconecto
			desconectadoOFinalizado(*turno);
		}
		if (recibirTodo(*turno, posY, sizeof(int))) {
			log_error(logger, "error al recibir la posicion Y");
			//error al enviar, supongo que se desconecto
			desconectadoOFinalizado(*turno);
		}
		if (movimientoValido(*turno, *posX, *posY)) {
			log_error(logger, "movimiento invalido");
			enviarHeader(*turno, movimientoInvalido);
		} else {
			moverEntrenador(*devolverEntrenador(*turno));
			enviarHeader(*turno, movimientoAceptado);
			dibujar(nombreMapa);
		}
		free(posX);
		free(posY);
		sleep(configuracion->retardo/1000);
		break;

	case capturarPokemon:
		;
		t_entrenadorBloqueado * entrenadorBloqueado = malloc(sizeof(t_entrenadorBloqueado));
		entrenadorBloqueado->socket = miTurno;
		if (recibirTodo(miTurno, &entrenadorBloqueado->identificadorPokemon, sizeof(char))) {
			log_error(logger, "error al recibir el identificador de la pokenest");
			//error al enviar, supongo que se desconecto
			desconectadoOFinalizado(*turno);
		} else {
			log_info(logger, "Quedo bloqueado el entrenador del socket: ", *turno);
			queue_push(bloqueados, (void *) entrenadorBloqueado);
			sumarPedidosMatriz(devolverIndiceEntrenador(miTurno),
					devolverIndicePokenest(entrenadorBloqueado->identificadorPokemon));
			*i = total;
			*quedoBloqueado = 1;
			sem_post(&contadorEntrenadoresBloqueados);
		}
		break;
	}
}
