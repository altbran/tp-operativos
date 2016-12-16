/*
 * deadlock.c
 *
 *  Created on: 15/10/2016
 *      Author: utnso
 */

#include "deadlock.h"

void detectarDeadlock() {

	char* nombreLogDeadlock = string_new();
	string_append(&nombreLogDeadlock, nombreMapa);
	string_append(&nombreLogDeadlock, "Deadlock.log");
	logDeadlock = log_create(nombreLogDeadlock, "deadlock", 0, log_level_from_string("INFO"));

	inicializarMatrices();
	inicializarVectores();

	cantidadDePokemones = list_size(Pokenests);
	cargarDisponiblesVector();
	entrenadoresEnDeadlockLista = list_create();
	entrenadoresClonada = list_create();

	fabrica = create_pkmn_factory();

	while (1) {
		//sem_wait(&contadorEntrenadoresBloqueados);
		sleep(configuracion->tiempoChequeoDeadlock / 1000);
		if (list_size(Entrenadores) > 1) {

			pthread_mutex_lock(&miMutex);

			cantidadDeEntrenadores = list_size(Entrenadores);
			cantidadDeEntrenadoresClonada = cantidadDeEntrenadores;
			cargarMatrices();
			list_clean(entrenadoresEnDeadlockLista);
			list_clean(entrenadoresClonada);
			copiarEntrenadores();
			pthread_mutex_unlock(&miMutex);

			inicializarAlgoritmoVector();

			inicalizarEntrenadoresEnDeadlock();

			if (configuracion->batalla == 1)
				batallaActivada = true;
			else
				batallaActivada = false;

			hayDeadlock = false;
			noTieneAsignadosOPedidos();
			algoritmo();

			if (hayDeadlock) {

				log_info(logger, "hay deadlock\0");
				log_info(logDeadlock, "Hay deadlock\0");
				log_info(logDeadlock, "Matriz de asignados\0");
				mostrarMatriz(asignadosMatrizClonada);
				log_info(logDeadlock, "Matriz de pedidos\0");
				mostrarMatriz(pedidosMatrizClonada);
				log_info(logDeadlock, "Entrenadores en deadlock\0");
				mostrarEntrenadoresEnDeadlock();
				resolverDeadlock();

			}
			free(pedidosMatrizClonada);
			free(asignadosMatrizClonada);

		}

	}
}
void copiarEntrenadores() {
	int a;
	for (a = 0; a < list_size(Entrenadores); a++) {
		t_datosEntrenador * entrenador = (t_datosEntrenador*) (list_get(Entrenadores, a));
		list_add(entrenadoresClonada, entrenador);
	}
}

void cargarDisponiblesVector() {
	int i;
	t_metadataPokenest* pokenest;
	for (i = 0; i < cantidadDePokemones; i++) {
		pokenest = (t_metadataPokenest*) list_get(Pokenests, i);
		disponiblesVector[i] = pokenest->cantidad;
	}
}

void cargarMatrices() {
	pedidosMatrizClonada = (int **) malloc(cantidadDeEntrenadoresClonada * sizeof(int*));
	asignadosMatrizClonada = (int **) malloc(cantidadDeEntrenadoresClonada * sizeof(int*));
	int a;
	for (a = 0; a < cantidadDeEntrenadoresClonada; a++) {
		pedidosMatrizClonada[a] = calloc(cantidadDePokemones, sizeof(int*));
		asignadosMatrizClonada[a] = calloc(cantidadDePokemones, sizeof(int*));
	}
	int i;
	int j;
	int aa;
	int p;
	for (i = 0; i < cantidadDeEntrenadoresClonada; i++) {
		for (j = 0; j < cantidadDePokemones; j++) {
			aa = asignadosMatriz[i][j];
			p = pedidosMatriz[i][j];
			asignadosMatrizClonada[i][j] = asignadosMatriz[i][j];
			pedidosMatrizClonada[i][j] = pedidosMatriz[i][j];
		}
	}
}

void inicializarMatrices() {
	pedidosMatriz = (int **) malloc(cantidadDeEntrenadores * sizeof(int*));
	asignadosMatriz = (int **) malloc(cantidadDeEntrenadores * sizeof(int*));

}

void inicializarEntrenadorEnMatrices(int indice) {
	pedidosMatriz[indice] = calloc(cantidadDePokemones, sizeof(int*));
	asignadosMatriz[indice] = calloc(cantidadDePokemones, sizeof(int*));
}

void agregarEntrenadorEnMatrices() {
	pthread_mutex_lock(&miMutex);
	cantidadDeEntrenadores++;
	pedidosMatriz = (int **) realloc(pedidosMatriz, cantidadDeEntrenadores * sizeof(int*));
	asignadosMatriz = (int **) realloc(asignadosMatriz, cantidadDeEntrenadores * sizeof(int*));
	inicializarEntrenadorEnMatrices(cantidadDeEntrenadores - 1);
	pthread_mutex_unlock(&miMutex);
}

void inicializarVectores() {
	disponiblesVector = calloc(cantidadDePokemones, sizeof(int*));
	algoritmoVector = calloc(cantidadDePokemones, sizeof(int*));
	recursosVector = calloc(cantidadDePokemones, sizeof(int*));
	inicalizarEntrenadoresEnDeadlock();
}

void inicalizarEntrenadoresEnDeadlock() {
	entrenadoresEnDeadlock = calloc(cantidadDeEntrenadoresClonada, sizeof(int*));
}

void inicializarAlgoritmoVector() {
	/*int b;
	 int j;
	 for (b = 0; b < cantidadDePokemones; b++) {
	 j = disponiblesVector[b];
	 t_metadataPokenest * pokenest = list_get(Pokenests, b);
	 log_info(logDeadlock, "Disponibles de %s : %d", pokenest->nombre, j);
	 algoritmoVector[b] = disponiblesVector[b];
	 }*/

	vectorAuxiliar = calloc(cantidadDePokemones, sizeof(int*));
	int j;
	int i;
	t_metadataPokenest* pokenest;

	for (i = 0; i < cantidadDeEntrenadoresClonada; i++) {
		for (j = 0; j < cantidadDePokemones; j++) {
			vectorAuxiliar[j] = vectorAuxiliar[j] + asignadosMatrizClonada[i][j];
		}
	}

	int c;
	for (c = 0; c < cantidadDePokemones; c++) {
		pokenest = (t_metadataPokenest*) list_get(Pokenests, i);
		algoritmoVector[c] = pokenest->cantidad - vectorAuxiliar[c];
	}

	free(vectorAuxiliar);

}

void mostrarMatriz(int** matriz) {
	if (cantidadDeEntrenadores == 0) {
		log_info(logDeadlock, "No hay entrenadores\0");
	} else {
		int i;
		int j;
		char* vector = calloc(cantidadDePokemones, sizeof(int*));
		for (i = 0; i < cantidadDeEntrenadoresClonada; i++) {
			for (j = 0; j < cantidadDePokemones; j++) {
				vector[j] = matriz[i][j] + '0';
			}
			log_info(logDeadlock, "%s", vector);

		}
	}
}

void mostrarEntrenadoresEnDeadlock() {
	int p;
	for (p = 0; p < list_size(entrenadoresEnDeadlockLista); p++) {
		t_datosEntrenador* entrenador = (t_datosEntrenador*) (list_get(entrenadoresEnDeadlockLista, p));
		log_info(logDeadlock, "%s", entrenador->nombre);
		log_info(logDeadlock, "%d", p);
		notificarDeadlockAEntrenador(p);
	}
}

void notificarDeadlockAEntrenador(int indice) {
	t_datosEntrenador* entrenador = (t_datosEntrenador*) (list_get(entrenadoresEnDeadlockLista, indice));
	if (enviarHeader(entrenador->socket, notificarDeadlock)) {
		log_info(logDeadlock, "Error al notificar deadlock al entrenador %s: \0", entrenador->nombre);
		desconectadoOFinalizado(entrenador->socket);
	}
}

void notificarMuerteAEntrenador(int sockete) {
	//sem_wait(&binarioDeLaMuerte);
	if (enviarHeader(sockete, entrenadorMuerto)) {
		log_error(logDeadlock, "error al enviar que se murio al entrenador del socket %d: \0", sockete);
		desconectadoOFinalizado(sockete);
	} else {
		close(sockete);
	}
}

void noTieneAsignadosOPedidos() {
	bool tienePedido;
	bool tieneAsignado;
	int j;
	int i;
	for (i = 0; i < cantidadDeEntrenadoresClonada; i++) {
		j = 0;
		tienePedido = false;
		tieneAsignado = false;
		while ((j < cantidadDePokemones) && (!tienePedido || !tieneAsignado)) {
			if (pedidosMatrizClonada[i][j] != 0) {
				tienePedido = true;
			}
			if (asignadosMatrizClonada[i][j] != 0) {
				tieneAsignado = true;
			}
			j++;
		}

		if ((!tienePedido || !tieneAsignado)) {
			entrenadoresEnDeadlock[i] = 1;
		}
	}
}
void algoritmo() {
	int cantidadDeEntrenadoresEnDlck = 0;
	hayDeadlock = false;
	bool marcar;
	int j;
	int i;
	int k;
	marcar = true;
	for (i = 0; i < cantidadDeEntrenadoresClonada; i++) {
		//log_info(logDeadlock, "%d", entrenadoresEnDeadlock[i]);
		if (entrenadoresEnDeadlock[i] == 0) {
			j = 0;
			while ((j < cantidadDePokemones) && marcar) {
				if (pedidosMatrizClonada[i][j] > algoritmoVector[j]) {
					marcar = false;
				}
				j++;
			}

			if (marcar) {
				entrenadoresEnDeadlock[i] = 1;
				for (k = 0; k < cantidadDePokemones; k++) {
					algoritmoVector[k] = algoritmoVector[k] + asignadosMatrizClonada[i][k];
				}
				list_clean(entrenadoresEnDeadlockLista);
				algoritmo();
				return;

			} else {
				cantidadDeEntrenadoresEnDlck++;
				hayDeadlock = true;
				t_datosEntrenador * entrenador = (t_datosEntrenador*) (list_get(entrenadoresClonada, i));
				list_add(entrenadoresEnDeadlockLista, entrenador);
			}
			marcar = true;
		}
	}
	if (cantidadDeEntrenadoresEnDlck == 1) {
		hayDeadlock = false;
	}
}

void resolverDeadlock() {

	if (batallaActivada) {
		mejoresPokemones = list_create();

		crearPokemones();

		t_pokemon* pokemonPerdedor = list_get(mejoresPokemones, 0);
		int indiceDeEntrenadorPerdedor = 0;
		int h = 1;
		t_pokemon* pokemonPerdedorAnterior = pokemonPerdedor;
		while (h < list_size(entrenadoresEnDeadlockLista)) {
			pokemonPerdedor = batallaPokemon(pokemonPerdedor, list_get(mejoresPokemones, h), indiceDeEntrenadorPerdedor, h);
			if (pokemonPerdedor != pokemonPerdedorAnterior) {
				pokemonPerdedorAnterior = pokemonPerdedor;
				indiceDeEntrenadorPerdedor = h;
			}
			h++;
		}
		//indiceDeEntrenadorPerdedor = traerIndiceEntrenadorPerdedor(indiceDeEntrenadorPerdedor);
		int socketPerdedor = notificarGanadoresEntrenadores(indiceDeEntrenadorPerdedor);
		notificarMuerteAEntrenador(socketPerdedor);
		//free(algoritmoVector);

		free(entrenadoresEnDeadlock);
		list_destroy_and_destroy_elements(mejoresPokemones, free);
	}
}

int notificarGanadoresEntrenadores(int indiceDeEntrenadorPerdedor) {
	int i = 0;

	//aviso a los que ganaron
	while (i < list_size(entrenadoresEnDeadlockLista)) {
		if (i != indiceDeEntrenadorPerdedor) {
			notificarResultadoBatalla(i, true);
		}
		i++;
	}
	//borro al entrenador que pierde
	t_datosEntrenador * perdedor = (t_datosEntrenador*) (list_get(entrenadoresEnDeadlockLista, indiceDeEntrenadorPerdedor));
	int socketPerdedor = perdedor->socket;
	strcpy(&ultimoPerdedor, &perdedor->nombre);
	//log_info(logger, "corre borrador del deadlock borrando el socket: %d", socketPerdedor);
	elMuertoDelDeadlock(socketPerdedor);
	sem_post(&semaforoMuerto);
	return socketPerdedor;
}

int traerIndiceEntrenadorPerdedor(int indice) {

	int i;
	int j = 0;
	for (i = 0; i < cantidadDeEntrenadoresClonada; i++) {
		if (entrenadoresEnDeadlock[i] == 0) {
			if (j == indice)
				return i;
			else
				j++;
		}
	}

}

void resolverDeadlockAMiManera() {
	int indiceEntrenadorMuerto;
	indiceEntrenadorMuerto = obtenerPrimerEntrenadorEnDeadlock();
	notificarMuerteAEntrenador(indiceEntrenadorMuerto);
}

int obtenerPrimerEntrenadorEnDeadlock() {
	int i;
	for (i = 0; i < cantidadDeEntrenadoresClonada; i++) {
		if (entrenadoresEnDeadlock[i] == 0)
			return i;
	}
}

t_pokemon* batallaPokemon(t_pokemon* pkmnA, t_pokemon* pkmnB, int indiceA, int indiceB) {
	t_pokemon* pokemonPerdedor = pkmn_battle(pkmnA, pkmnB);
	return pokemonPerdedor;
}

void notificarResultadoBatalla(int indice, bool gano) {
	t_datosEntrenador* entrenador = (t_datosEntrenador*) (list_get(entrenadoresEnDeadlockLista, indice));
	if (gano) {
		if (enviarHeader(entrenador->socket, entrenadorGanador)) {
			log_error(logDeadlock, "error al enviar el header de ganador al entrenador %s: \0", entrenador->nombre);
			desconectadoOFinalizado(entrenador->socket);
		}
	} else {
		if (enviarHeader(entrenador->socket, entrenadorPerdedor)) {
			log_error(logDeadlock, "error al enviar el header de perdedor al entrenador %s: \0", entrenador->nombre);
			desconectadoOFinalizado(entrenador->socket);
		}
		//close(entrenador->socket);
	}
}

void crearPokemones() {

	int i;
	cantidadDeEntrenadoresEnDeadlock = 0;
	//log_info(logDeadlock, "cantidad de entrenadores en deadlock: %d",list_size(entrenadoresEnDeadlockLista));
	for (i = 0; i < list_size(entrenadoresEnDeadlockLista); i++) {
		cantidadDeEntrenadoresEnDeadlock++;
		//todo obtengo el indice, el socket, y le pido el pokemon mas fuerte
		int errores = 0;
		t_pokemon* pokemonAGuardar;
		t_metadataPokemon* pokemon = malloc(sizeof(t_metadataPokemon));

		t_datosEntrenador * entrenador = (t_datosEntrenador*) (list_get(entrenadoresEnDeadlockLista, i));
		errores = enviarHeader(entrenador->socket, mejorPokemon);
		errores += recibirTodo(entrenador->socket, &pokemon->nivel, sizeof(int));
		errores += recibirTodo(entrenador->socket, &pokemon->nombre, 18);
		if (errores) {
			log_error(logDeadlock, "error al recibir pokemon mas fuerte del entrenador %s: %d\0", entrenador->nombre,
					entrenador->socket);
			elMuertoDelDeadlock(entrenador->socket);
		} else {
			pokemonAGuardar = create_pokemon(fabrica, &pokemon->nombre, pokemon->nivel);
			list_add(mejoresPokemones, pokemonAGuardar);
		}

	}
}

void sumarPedidosMatriz(int indiceEntrenador, int indicePokenest) {
	pthread_mutex_lock(&miMutex);
	pedidosMatriz[indiceEntrenador][indicePokenest] = pedidosMatriz[indiceEntrenador][indicePokenest] + 1;
	pthread_mutex_unlock(&miMutex);
}

void sumarAsignadosMatriz(int indiceEntrenador, int indicePokenest) {
	pthread_mutex_lock(&miMutex);
	asignadosMatriz[indiceEntrenador][indicePokenest] = asignadosMatriz[indiceEntrenador][indicePokenest] + 1;
	pedidosMatriz[indiceEntrenador][indicePokenest] = pedidosMatriz[indiceEntrenador][indicePokenest] - 1;
	disponiblesVector[indicePokenest] = disponiblesVector[indicePokenest] - 1;
	pthread_mutex_unlock(&miMutex);
}

void restarAsignadosMatriz(int indiceEntrenador, int indicePokenest) {
	pthread_mutex_lock(&miMutex);
	asignadosMatriz[indiceEntrenador][indicePokenest] = asignadosMatriz[indiceEntrenador][indicePokenest] - 1;
	disponiblesVector[indicePokenest] = disponiblesVector[indicePokenest] + 1;
	pthread_mutex_unlock(&miMutex);
}

void sumarDisponibles(int indicePokenest) {
	pthread_mutex_lock(&miMutex);
	disponiblesVector[indicePokenest] = disponiblesVector[indicePokenest] + 1;
	pthread_mutex_unlock(&miMutex);
}

void liberarRecursosEntrenador(int indiceEntrenador) {
	pthread_mutex_lock(&miMutex);
	int i;
	int j;
	for (i = indiceEntrenador; i < cantidadDeEntrenadores - 1; i++) {
		for (j = 0; j < cantidadDePokemones; j++) {
			asignadosMatriz[i][j] = asignadosMatriz[i + 1][j];
			pedidosMatriz[i][j] = pedidosMatriz[i + 1][j];
		}
	}
	cantidadDeEntrenadores--;
	if (cantidadDeEntrenadores != 0) {
		pedidosMatriz = (int **) realloc(pedidosMatriz, cantidadDeEntrenadores * sizeof(int*));
		asignadosMatriz = (int **) realloc(asignadosMatriz, cantidadDeEntrenadores * sizeof(int*));
	}
	pthread_mutex_unlock(&miMutex);
}
