/*
 ============================================================================
 Name        : Entrenador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <netdb.h>
#include <unistd.h>
#include "commons/log.h"
#include <src/sockets.h>
#include <src/structs.h>*/
#include "funcionesE.h"

int main(int argc, char** argv){

	nombre = argv[1];
	ruta = argv[2];

	if(argc != 3){
		printf("Numero de parametros incorrectos");
		log_error(logger,"Numero de parametros incorrectos",mensaje);
		return 1;
	}

	char* IP_MAPA_SERVIDOR = getenv("IP_MAPA_SERVIDOR");
	logger = log_create("Entrenador.log", "ENTRENADOR", 0, log_level_from_string("INFO"));

	if(crearSocket(&socketCliente)){
		printf("No se pudo crear el socket.");
		return 1;
	}
	if(conectarA(servidorMapa, IP_MAPA_SERVIDOR, PUERTO_MAPA_SERVIDOR)){
		printf("No se puede conectar al servidor.");
		log_error(logger, "Fallo al conectarse al servidor.", mensaje);
		return 1;
	}

	log_info(logger, "Se ha iniciado conexion con el servidor", mensaje);

	if(responderHandshake(servidorMapa, IDENTRENADOR, IDMAPA)){
		printf("Error al responder handshake");
		log_error(logger, "No se pudo responder handshake", mensaje);
		return 1;
	}
	log_info(logger, "Conexion establecida", mensaje);
	return EXIT_SUCCESS;
}
