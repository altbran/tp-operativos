/*
 ============================================================================
 Name        : Entrenador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "funcionesE.h"


int main(int argc, char** argv){

	if(argc != 3){
		printf("Numero de parametros incorrectos");
		log_error(logger,"Numero de parametros incorrectos",mensaje);
		return 1;
	}
	char rutaMetadata[30];
	char nombreRuta[15];
	//FILE* fp;
	strcpy(nombreRuta,argv[1]);
	strcpy(rutaMetadata,argv[2]);

	strcat(strcat(ruta,"/Entrenadores/"), nombre);

	//fp = fopen(rutaMetadata,"r");
	t_config* metaDataEntrenador = config_create(rutaMetadata);
	entrenador.nombre  = config_get_string_value(metaDataEntrenador, "nombre");

	char* simbolo = config_get_string_value(metaDataEntrenador, "simbolo");
	entrenador.simbolo = simbolo[0];
	entrenador.vidas = config_get_int_value(metaDataEntrenador, "vidas");

	entrenador.reintentos = config_get_int_value(metaDataEntrenador, "reintentos");


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
