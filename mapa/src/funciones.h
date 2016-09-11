/*
 * funciones.h
 *
 *  Created on: 6/9/2016
 *      Author: utnso
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "commons/log.h"
#include <commons/config.h>
#include <commons/collections/list.h>
#include <curses.h>
#include <nivel.h>
#include <src/sockets.h>
#include <src/structs.h>
#include <src/protocolo.h>

t_log* logger;
char *texto;

int clientePokeDex;
int servidorMapa;
fd_set bolsaDeSockets;
fd_set bolsaAuxiliar;

t_list* items;

#endif /* FUNCIONES_H_ */
