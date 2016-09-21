/*
 * funcionesE.h
 *
 *  Created on: 14/9/2016
 *      Author: utnso
 */

#ifndef FUNCIONESE_H_
#define FUNCIONESE_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "commons/log.h"
#include <curses.h>
#include <nivel.h>
#include <src/sockets.h>
#include <src/structs.h>
#include <src/protocolo.h>
#include <signal.h>
#include <pthread.h>
#include "commons/config.h"
#include <string.h>

int PUERTO_MAPA_SERVIDOR = 6650;

char* mensaje;
char* nombre;
char* ruta;
t_log* logger;
int socketCliente;
int servidorMapa;
t_metadataEntrenador entrenador;

#endif /* FUNCIONESE_H_ */

