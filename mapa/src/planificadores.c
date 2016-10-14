#include "funciones.h"

void roundRobin() {
	int i;
	while (1) {
		int turno = queue_pop(listos);
		for (i = 0; i < configuracion.quantum; i++) {
			switch (recibirHeader(turno)) {

			case datosPokenest:
				char identificador;
				recibirTodo(turno, identificador, sizeof(char));
				enviarCoordPokenest(turno, devolverPokenest(identificador));
				i = -1;
				break;
			case posicionEntrenador:

				recibirTodo(turno, identificador, sizeof(char));
				break;

			case capturarPokemon:
				//todo hacer cosas
				break;
			}
		}
		queue_push(listos,turno);
	}
}
