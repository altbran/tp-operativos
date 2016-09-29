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
#include "commons/string.h"
#include <string.h>

int PUERTO_MAPA_SERVIDOR ;

char* mensaje;
char* nombre;

t_log* logger;
int socketCliente;
int servidorMapa;
t_metadataEntrenador entrenador;
typedef struct{
	char* mapa;
	t_list* objetivos;
}t_objetivosPorMapa;


#endif /* FUNCIONESE_H_ */

