#include "funciones.h"

void sjfs(){}

void roundRobin() {
	int i;
	while (1) {
		pthread_mutex_lock(&mutex);
		int turno = *(int*) (queue_pop(listos));
		int quedoBloqueado = 1;
		for (i = 0; i < configuracion->quantum; i++) {
			switch (recibirHeader(turno)) {

			case datosPokenest:
				;
				char identificadorPokenest;
				recibirTodo(turno, &identificadorPokenest, sizeof(char));
				t_metadataPokenest pokenest = devolverPokenest(&identificadorPokenest);
				enviarCoordPokenest(turno, &pokenest);
				i = i - 1;
				break;

			case posicionEntrenador:
				;
				int posX;
				int posY;
				recibirTodo(turno, &posX, sizeof(int));
				recibirTodo(turno, &posY, sizeof(int));
				if (movimientoValido(turno, posX, posY)) {
					moverEntrenador(devolverEntrenador(turno));
					enviarHeader(turno, movimientoAceptado);
					dibujar(nombreMapa);
				} else {
					log_info(logger, "movimiento invalido");
					//todo responder invalido
				}
				break;

			case capturarPokemon:
				;
				t_entrenadorBloqueado entrenadorBloqueado;
				entrenadorBloqueado.socket = turno;
				recibirTodo(turno, entrenadorBloqueado.identificadorPokemon, sizeof(char));
				queue_push(bloqueados, &entrenadorBloqueado);
				sumarPedidosMatriz(devolverIndiceEntrenador(turno),
						devolverIndicePokenest(entrenadorBloqueado.identificadorPokemon));
				i = configuracion->quantum;
				quedoBloqueado = 0;
				//todo poner mutex para atrapar pokemon
				break;
			}
		}
		if (!quedoBloqueado) {
			queue_push(listos, &turno);
		}
		pthread_mutex_unlock(&mutex);
	}
}

void atraparPokemon() {
	while (1) {
		//todo poner mutex si hay entrenadores bloqueados
		if (!queue_is_empty(bloqueados)) {
			t_entrenadorBloqueado entrenador = *(t_entrenadorBloqueado*) queue_pop(bloqueados);
			int numeroPokemon;
			int indice;
			if (pokemonDisponible(devolverIndicePokenest(entrenador.identificadorPokemon),entrenador.identificadorPokemon,numeroPokemon,indice)) {
				enviarHeader(entrenador.socket, pokemonesDisponibles);
				send(entrenador.socket,numeroPokemon,sizeof(int),0);
				int header = recibirHeader(entrenador.socket);
				if (header == entrenadorListo) {
					t_pokemon * pokemon = list_get(pokemones,indice);
					pokemon.socketEntrenador = entrenador.socket;
					//todo consultar a juan si hay que hacer replace
					list_replace(pokemones,indice,pokemon);
					restarRecursoDisponible(devolverIndicePokenest(entrenador.identificadorPokemon));
					restarPokemon(entrenador.identificadorPokemon);
					sumarAsignadosMatriz(devolverIndiceEntrenador(entrenador.socket),devolverIndicePokenest(entrenador.identificadorPokemon));
					queue_push(listos, &entrenador.socket);
				} else if (header == finalizoMapa) {
					list_remove(Entrenadores, devolverIndiceEntrenador(entrenador.socket));
					//todo devolver todos los recursos
				}
			}
		}
	}
}
