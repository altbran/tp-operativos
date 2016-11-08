#include "funciones.h"

void srdf() {
	int i;
	while (1) {
		sem_wait(&contadorEntrenadoresListos);
		if (!queue_is_empty(listos)) {
			int * turno;
			int * movimientos = malloc(sizeof(int));
			turno = entrenadorMasCercano(&movimientos);
			int quedoBloqueado = 0;
			for (i = 0; i < *movimientos; i++) {
				pthread_mutex_lock(&mutex);
				jugada(turno, &quedoBloqueado, &i, *movimientos);
				pthread_mutex_unlock(&mutex);
			}
			free(movimientos);
			if (!quedoBloqueado) {
				queue_push(listos, &turno);
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
			turno = queue_pop(listos);
			int quedoBloqueado = 0;
			for (i = 0; i < configuracion->quantum; i++) {
				pthread_mutex_lock(&mutex);
				jugada(turno, &quedoBloqueado, &i, configuracion->quantum);
				pthread_mutex_unlock(&mutex);
			}
			if (!quedoBloqueado) {
				queue_push(listos, &turno);
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
				if (enviarTodo(entrenador->socket, &numeroPokemon, sizeof(int))) {
					//error, se desconecto
					desconectadoOFinalizado(entrenador->socket);
				} else {

					int header = recibirHeader(entrenador->socket);

					//entrenador avisa que ya lo copio
					if (header == entrenadorListo) {

						//hago cosas correspondientes a que lo atrapo
						t_duenioPokemon * pokemon = list_get(pokemones, *indice);
						pokemon->socketEntrenador = entrenador->socket;
						restarRecursoDisponible(devolverIndicePokenest(entrenador->identificadorPokemon));
						restarPokemon(&entrenador->identificadorPokemon);
						sumarAsignadosMatriz(devolverIndiceEntrenador(entrenador->socket),devolverIndicePokenest(entrenador->identificadorPokemon));
						queue_push(listos, &entrenador->socket);
						sem_post(&contadorEntrenadoresListos);
						log_info(logger,"Pasa a listo el entrenador del socket: ",entrenador->socket);

						//en caso de terminar el mapa devolver todos los recursos
					} else if (header == finalizoMapa) {
						desconectadoOFinalizado(entrenador->socket);
						log_info(logger,"Termino el mapa el entrenador del socket: ",entrenador->socket);
					} else {
						//problema en el header, asumo que se desconecto
						log_error(logger,"Se desconecto el entrenador del socket: ",entrenador->socket);
						desconectadoOFinalizado(entrenador->socket);
					}
					pthread_mutex_unlock(&mutexDeadlock);
					free(entrenador);
				}
			} else {
				queue_push(bloqueados, entrenador);
				sem_post(&contadorEntrenadoresBloqueados);
			}
			free(numeroPokemon);
			free(indice);
		}
	}
}

int * entrenadorMasCercano(int * movimientos) {
	int i;
	int menor = 100;
	int indice;
	int * socketMasCercano;
	for (i = 0; i < list_size(Entrenadores); i++) {
		t_datosEntrenador * entrenador = list_get(Entrenadores, i);
		if (entrenador->distanciaAPokenest < menor) {
			menor = entrenador->distanciaAPokenest;
			indice = i;
		}
	}
	*movimientos = menor;
	t_datosEntrenador * entrenador = list_get(Entrenadores, indice);
	for (i = 0; i < queue_size(listos); i++) {
		int * sockete = queue_pop(listos);
		if (*sockete == entrenador->socket) {
			socketMasCercano = sockete;
			break;
		} else {
			queue_push(listos, sockete);
		}
	}
	return socketMasCercano;
}

void jugada(int * turno, int * quedoBloqueado, int * i, int total) {

	switch (recibirHeader(*turno)) {

	case datosPokenest:
		;
		char * identificadorPokenest = malloc(sizeof(char));
		if (recibirTodo(*turno, &identificadorPokenest, sizeof(char))) {
			//error al enviar, supongo que se desconecto
			log_error(logger, "error al recibir identificador de pokenest");
			desconectadoOFinalizado(*turno);
		}
		t_metadataPokenest * pokenest = devolverPokenest(identificadorPokenest);
		if (enviarCoordPokenest(*turno, pokenest)) {
			log_error(logger, "error al enviar las coordenadas de la pokenest");
			//error al enviar, supongo que se desconecto
			desconectadoOFinalizado(*turno);
		}
		t_datosEntrenador * entrenador = devolverEntrenador(*turno);
		if (recibirTodo(*turno, &entrenador->distanciaAPokenest, sizeof(int))) {
			log_error(logger, "error al recibir la distancia a la pokenest");
			//error al enviar, supongo que se desconecto
			desconectadoOFinalizado(*turno);
		}
		free(identificadorPokenest);
		*i = *i - 1;
		break;

	case posicionEntrenador:
		;
		int * posX = malloc(sizeof(int));
		int * posY = malloc(sizeof(int));
		if (recibirTodo(*turno, &posX, sizeof(int))) {
			log_error(logger, "error al recibir la posicion X");
			//error al enviar, supongo que se desconecto
			desconectadoOFinalizado(*turno);
		}
		if (recibirTodo(*turno, &posY, sizeof(int))) {
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
		break;

	case capturarPokemon:
		;
		t_entrenadorBloqueado * entrenadorBloqueado = malloc(sizeof(t_entrenadorBloqueado));
		entrenadorBloqueado->socket = *turno;
		if (recibirTodo(*turno, &entrenadorBloqueado->identificadorPokemon, sizeof(char))) {
			log_error(logger, "error al recibir el identificador de la pokenest");
			//error al enviar, supongo que se desconecto
			desconectadoOFinalizado(*turno);
		} else {
			log_info(logger,"Quedo bloqueado el entrenador del socket: ",*turno);
			queue_push(bloqueados, &entrenadorBloqueado);
			sumarPedidosMatriz(devolverIndiceEntrenador(*turno),devolverIndicePokenest(entrenadorBloqueado->identificadorPokemon));
			*i = total;
			*quedoBloqueado = 1;
			sem_post(&contadorEntrenadoresBloqueados);
		}
		break;
	}
}
