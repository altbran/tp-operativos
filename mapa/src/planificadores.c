#include "funciones.h"

void roundRobin() {
	int i;
	while (1) {
		int turno = *(int*)(queue_pop(listos));
		int quedoBloqueado = 1;
		for (i = 0; i < configuracion.quantum; i++) {
			switch (recibirHeader(turno)) {

			case datosPokenest: ;
				char identificadorPokenest;
				recibirTodo(turno, identificadorPokenest, sizeof(char));
				enviarCoordPokenest(turno, devolverPokenest(identificadorPokenest));
				i = -1;
				break;

			case posicionEntrenador: ;
				int posX;
				int posY;
				recibirTodo(turno, posX, sizeof(int));
				recibirTodo(turno, posY, sizeof(int));
				if (movimientoValido(turno, posX, posY)) {
					//todo actualizar mapa
				} else {
					//todo responder invalido
				}
				break;

			case capturarPokemon: ;
				t_entrenadorBloqueado entrenadorBloqueado;
				entrenadorBloqueado.socket = turno;
				recibirTodo(turno, entrenadorBloqueado.identificadorPokemon, sizeof(char));
				queue_push(bloqueados, &entrenadorBloqueado);
				i = configuracion.quantum;
				quedoBloqueado = 0;
				//todo poner mutex para atrapar pokemon
				break;
			}
			dibujar();
		}
		if (!quedoBloqueado) {
			queue_push(listos, &turno);
		}

	}
}

void atraparPokemon() {
	while (1) {
		//todo poner mutex si hay entrenadores bloqueados
		if (!queue_is_empty(bloqueados)) {
			t_entrenadorBloqueado entrenador = *(t_entrenadorBloqueado*)queue_pop(bloqueados);
			if (pokemonDisponible(entrenador.identificadorPokemon)) {
				enviarHeader(entrenador.socket, pokemonesDisponibles);
				if (recibirHeader(entrenador.socket) == entrenadorListo) {
					queue_push(listos, &entrenador.socket);
				} else {
					//todo completo el mapa?
				}
			}
		}
	}
}
