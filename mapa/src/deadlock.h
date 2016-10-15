/*
 * deadlock.h
 *
 *  Created on: 15/10/2016
 *      Author: utnso
 */

#ifndef DEADLOCK_H_
#define DEADLOCK_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include "commons/log.h"
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <curses.h>
#include <nivel.h>
#include <src/sockets.h>
#include <src/structs.h>
#include <src/protocolo.h>
#include "dibujador.h"
#include <signal.h>
#include <pthread.h>

int** pedidosMatriz;
int** asignadosMatriz;
int* recursosVector;
int* disponiblesVector;
int* entrenadoresEnDeadlock;
int cantidadDeEntrenadores;
int cantidadDePokemones;
int hayDeadlock;

void inicializarMatrices();
void inicializarVectores();
void noTieneAsignadosOPedidos();
void algoritmo();

#endif /* DEADLOCK_H_ */
