#include "funciones.h"

void srdf() {
	int i;
	while (1) {
		if (!queue_is_empty(listos)) {
			pthread_mutex_lock(&mutex);
			int * turno;
			int * movimientos = malloc(sizeof(int));
			*turno = entrenadorMasCercano(&movimientos);
			int quedoBloqueado = 0;
			for (i = 0; i < *movimientos; i++) {
				jugada(turno, &quedoBloqueado, &i, *movimientos);
			}
			free(movimientos);
			if (quedoBloqueado) {
				queue_push(listos, &turno);
			}
			pthread_mutex_unlock(&mutex);
		}
	}
}

void roundRobin() {
	int i;
	while (1) {
		if (!queue_is_empty(listos)) {
			pthread_mutex_lock(&mutex);
			int * turno;
			turno = queue_pop(listos);
			int quedoBloqueado = 0;
			for (i = 0; i < configuracion->quantum; i++) {
				jugada(turno, &quedoBloqueado, &i, configuracion->quantum);
			}
			if (quedoBloqueado) {
				queue_push(listos, &turno);
			}
			pthread_mutex_unlock(&mutex);
		}
	}
}

void atraparPokemon() {
	while (1) {
		//todo poner mutex si hay entrenadores bloqueados
		if (!queue_is_empty(bloqueados)) {
			t_entrenadorBloqueado entrenador = *(t_entrenadorBloqueado*) queue_pop(bloqueados);
			int numeroPokemon;
			int indice;
			if (pokemonDisponible(devolverIndicePokenest(entrenador.identificadorPokemon), *entrenador.identificadorPokemon,
					&numeroPokemon, &indice)) {
				enviarHeader(entrenador.socket, pokemonesDisponibles);
				send(entrenador.socket, &numeroPokemon, sizeof(int), 0);
				int header = recibirHeader(entrenador.socket);
				if (header == entrenadorListo) {
					t_duenioPokemon * pokemon = list_get(pokemones, indice);
					pokemon->socketEntrenador = entrenador.socket;
					//todo consultar a juan si hay que hacer replace
					list_replace(pokemones, indice, pokemon);
					restarRecursoDisponible(devolverIndicePokenest(entrenador.identificadorPokemon));
					restarPokemon(entrenador.identificadorPokemon);
					sumarAsignadosMatriz(devolverIndiceEntrenador(entrenador.socket),
							devolverIndicePokenest(entrenador.identificadorPokemon));
					queue_push(listos, &entrenador.socket);
				} else if (header == finalizoMapa) {
					list_remove(Entrenadores, devolverIndiceEntrenador(entrenador.socket));
					//todo devolver todos los recursos
				}
			}
		}
	}
}

int entrenadorMasCercano(int * movimientos) {
	int i;
	int menor = 100;
	int indice;
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
			return entrenador->socket;
		} else {
			queue_push(listos, sockete);
		}
	}
}

void jugada(int * turno, int * quedoBloqueado, int * i, int total) {

	switch (recibirHeader(*turno)) {

	case datosPokenest:
		;
		char identificadorPokenest;
		recibirTodo(*turno, &identificadorPokenest, sizeof(char));
		t_metadataPokenest pokenest = devolverPokenest(&identificadorPokenest);
		enviarCoordPokenest(*turno, &pokenest);
		t_datosEntrenador entrenador = devolverEntrenador(*turno);
		recibirTodo(*turno, &entrenador.distanciaAPokenest, sizeof(int));
		*i = *i - 1;
		break;

	case posicionEntrenador:
		;
		int posX;
		int posY;
		recibirTodo(*turno, &posX, sizeof(int));
		recibirTodo(*turno, &posY, sizeof(int));
		if (movimientoValido(*turno, posX, posY)) {
			log_info(logger, "movimiento invalido");
			//todo responder invalido
		} else {
			moverEntrenador(devolverEntrenador(*turno));
			enviarHeader(*turno, movimientoAceptado);
			dibujar(nombreMapa);
		}
		break;

	case capturarPokemon:
		;
		t_entrenadorBloqueado entrenadorBloqueado;
		entrenadorBloqueado.socket = *turno;
		recibirTodo(*turno, entrenadorBloqueado.identificadorPokemon, sizeof(char));
		queue_push(bloqueados, &entrenadorBloqueado);
		sumarPedidosMatriz(devolverIndiceEntrenador(*turno), devolverIndicePokenest(entrenadorBloqueado.identificadorPokemon));
		*i = total;
		*quedoBloqueado = 1;
		//todo poner mutex para atrapar pokemon
		break;
	}
}
