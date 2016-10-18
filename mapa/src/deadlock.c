/*
 * deadlock.c
 *
 *  Created on: 15/10/2016
 *      Author: utnso
 */

#include "deadlock.h"

void detectarDeadlock() {
	cantidadDeEntrenadores = list_size(Entrenadores);
	cantidadDePokemones = list_size(Pokenests);

	inicializarMatrices();
	inicializarVectores();

	noTieneAsignadosOPedidos();

	algoritmo();

}

void inicializarMatrices() {
	int i;
	pedidosMatriz = (int **) malloc(cantidadDeEntrenadores * sizeof(int*));
	for (i = 0; i < cantidadDeEntrenadores; i++)
		pedidosMatriz[i] = (int *) malloc(cantidadDePokemones * sizeof(int));

	int j;
	asignadosMatriz = (int **) malloc(cantidadDeEntrenadores * sizeof(int*));
	for (j = 0; j < cantidadDeEntrenadores; j++)
		asignadosMatriz[j] = (int *) malloc(cantidadDePokemones * sizeof(int));
}

void inicializarVectores() {
	disponiblesVector = calloc(cantidadDePokemones, sizeof(int*));
	recursosVector = calloc(cantidadDePokemones, sizeof(int*));
	entrenadoresEnDeadlock = calloc(cantidadDeEntrenadores, sizeof(int*));
}

void noTieneAsignadosOPedidos(){
	int marcar = 0;
	int tienePedido;
	int tieneAsignado;
	int j;
	int i;
	for(i = 0; i < cantidadDeEntrenadores; i++){
		j = 0;
		tienePedido = 0;
		tieneAsignado = 0;
		while(j < cantidadDePokemones && (tienePedido == 0 || tieneAsignado == 0)){
			if (pedidosMatriz[i][j] != 0){
				tienePedido= 1;
			}
			if(asignadosMatriz[i][j] != 0){
				tieneAsignado = 1;
			}
			j++;
		}

		if((tienePedido == 0 || tieneAsignado == 0)){
			entrenadoresEnDeadlock[i] = 1;
		}
		marcar = 0;
	}
}

void algoritmo(){

	int marcar;
	int j;
	int i;
	int k;
	marcar = 0;
	for(i = 0; i < cantidadDeEntrenadores; i++){
		if(entrenadoresEnDeadlock[i] == 0){
			j = 0;
			while(j < cantidadDePokemones && marcar == 0){
				if (pedidosMatriz[i][j] > disponiblesVector[j]){
					marcar = 1;
				}
				j++;
			}

			if(marcar == 0){
				entrenadoresEnDeadlock[i] = 1;
				for(k = 0;k < cantidadDePokemones; k++){
					disponiblesVector[k] = disponiblesVector[k] + asignadosMatriz[i][k];
				}

			}
			marcar = 0;
		}
		else {}
	}
}
