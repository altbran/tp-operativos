/*
 * funciones.h
 *
 *  Created on: 6/9/2016
 *      Author: utnso
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_

#include "commons/log.h"
#include <src/sockets.h>
#include <src/structs.h>
#include <unistd.h>
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

t_log* logger;
char *texto;

int clientePokeDex;
fd_set bolsaDeSockets;
fd_set bolsaAuxiliar;

#endif /* FUNCIONES_H_ */
