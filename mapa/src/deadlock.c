/*
 * deadlock.c
 *
 *  Created on: 15/10/2016
 *      Author: utnso
 */

#include "deadlock.h"

void detectarDeadlock() {


	logDeadlock = log_create("Deadlock.log", "deadlock", 0, log_level_from_string("INFO"));

	sleep(configuracion.tiempoChequeoDeadlock / 1000);

	hayDeadlock = EXIT_FAILURE;

	cantidadDeEntrenadores = list_size(Entrenadores);
	cantidadDePokemones = list_size(Pokenests);

	inicializarMatrices();
	inicializarVectores();

	inicializarAlgoritmoVector();

	noTieneAsignadosOPedidos();

	algoritmo();

	if(hayDeadlock == EXIT_SUCCESS){
		log_info(logDeadlock, "Hay deadlock");
		log_info(logDeadlock, "Matriz de asignados");
		mostrarMatriz(asignadosMatriz);
		log_info(logDeadlock, "Matriz de pedidos");
		mostrarMatriz(pedidosMatriz);
		log_info(logDeadlock, "Entrenadores en deadlock");
		mostrarEntrenadoresEnDeadlock();
	}

}

void inicializarMatrices() {
	int i;
	pedidosMatriz = (int **) malloc(cantidadDeEntrenadores * sizeof(int*));
	asignadosMatriz = (int **) malloc(cantidadDeEntrenadores * sizeof(int*));
	for (i = 0; i < cantidadDeEntrenadores; i++){
		inicializarEntrenadorEnMatrices(i);
	}
}

void inicializarEntrenadorEnMatrices(int indice){
	pedidosMatriz[indice] = (int *) malloc(cantidadDePokemones * sizeof(int));
	asignadosMatriz[indice] = (int *) malloc(cantidadDePokemones * sizeof(int));
}

void agregarEntrenadorEnMatrices(){
	pedidosMatriz = (int **) realloc(pedidosMatriz, cantidadDeEntrenadores * sizeof(int*));
	asignadosMatriz = (int **) realloc(asignadosMatriz, cantidadDeEntrenadores * sizeof(int*));
	inicializarEntrenadorEnMatrices(cantidadDeEntrenadores - 1);

}

void inicializarVectores() {
	disponiblesVector = calloc(cantidadDePokemones, sizeof(int*));
	recursosVector = calloc(cantidadDePokemones, sizeof(int*));
	entrenadoresEnDeadlock = calloc(cantidadDeEntrenadores, sizeof(int*));
}

void inicializarAlgoritmoVector(){
	int b;
	for(b=0;b<cantidadDePokemones;b++){
		algoritmoVector[b] = disponiblesVector[b];
	}
}

void mostrarMatriz(int** matriz) {
	int i;
	int j;
	char* vector = calloc(cantidadDePokemones, sizeof(int*));
	for (i = 0; i < cantidadDeEntrenadores; i++) {
		for(j=0; j<cantidadDePokemones; j++){
			vector[j] = matriz[i][j] + '0';
		}
		log_info(logDeadlock,"%s" ,vector);

	}
}

void mostrarEntrenadoresEnDeadlock(){
	int p;
	for(p=0;p<cantidadDeEntrenadores;p++){
		if(entrenadoresEnDeadlock[p] == 0){
			//log_info(logDeadlock,"%s", list_get(Entrenadores,p).nombre)
			log_info(logDeadlock,"%d", p);
		}
	}
}


void noTieneAsignadosOPedidos(){
	int tienePedido;
	int tieneAsignado;
	int j;
	int i;
	for(i = 0; i < cantidadDeEntrenadores; i++){
		j = 0;
		tienePedido = EXIT_FAILURE;
		tieneAsignado = EXIT_FAILURE;
		while(j < cantidadDePokemones && (tienePedido ==  EXIT_FAILURE|| tieneAsignado ==  EXIT_FAILURE)){
			if (pedidosMatriz[i][j] != 0){
				tienePedido= EXIT_SUCCESS;
			}
			if(asignadosMatriz[i][j] != 0){
				tieneAsignado = EXIT_SUCCESS;
			}
			j++;
		}

		if((tienePedido ==  EXIT_FAILURE || tieneAsignado ==  EXIT_FAILURE)){
			entrenadoresEnDeadlock[i] = 1;
		}
	}
}
void algoritmo(){
	int marcar;
	int j;
	int i;
	int k;
	marcar = EXIT_SUCCESS;
	for(i = 0; i < cantidadDeEntrenadores; i++){
		if(entrenadoresEnDeadlock[i] == 0){
			j = 0;
			while(j < cantidadDePokemones && marcar ==  EXIT_SUCCESS){
				if (pedidosMatriz[i][j] > algoritmoVector[j]){
					marcar = EXIT_FAILURE;
				}
				j++;
			}

			if(marcar ==  EXIT_SUCCESS){
				entrenadoresEnDeadlock[i] = 1;
				for(k = 0;k < cantidadDePokemones; k++){
					algoritmoVector[k] = algoritmoVector[k] + asignadosMatriz[i][k];
				}
				algoritmo();

			}
			else {
				hayDeadlock = EXIT_SUCCESS;
			}
			marcar = EXIT_SUCCESS;
		}
		else {}
	}
}

void sumarPedidosMatriz(int indiceEntrenador, int indicePokenest){
	pedidosMatriz[indiceEntrenador][indicePokenest] = pedidosMatriz[indiceEntrenador][indicePokenest] + 1;
}

void sumarAsignadosMatriz(int indiceEntrenador, int indicePokenest){
	asignadosMatriz[indiceEntrenador][indicePokenest] = asignadosMatriz[indiceEntrenador][indicePokenest] + 1;
	pedidosMatriz[indiceEntrenador][indicePokenest] = pedidosMatriz[indiceEntrenador][indicePokenest] - 1;
}

void restarAsignadosMatriz(int indiceEntrenador, int indicePokenest){
	asignadosMatriz[indiceEntrenador][indicePokenest] = asignadosMatriz[indiceEntrenador][indicePokenest] - 1;
}

void liberarRecursosEntrenador(int indiceEntrenador){
	int i;
	int j;
	for(i = indiceEntrenador; i < cantidadDeEntrenadores-1;i++){
		for(j = 0; j < cantidadDePokemones;j++){
			asignadosMatriz[i][j] = asignadosMatriz[i+1][j];
		}
	}
}

