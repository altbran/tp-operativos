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
	//entrenador.hojaDeViaje = asignarHojaDeViajeYObjetos(metaDataEntrenador);
	char* simbolo = config_get_string_value(metaDataEntrenador, "simbolo");
	entrenador.simbolo = simbolo[0];
	entrenador.vidas = config_get_int_value(metaDataEntrenador, "vidas");
	entrenador.reintentos = config_get_int_value(metaDataEntrenador, "reintentos");
	/*
	t_list* asignarHojaDeViajeYObjetivos(t_config* metadata){
	t_log* log = log_create("logs/metadata.log", "Leer_metadata_HojaDeViaje", false, LOG_LEVEL_INFO);
	t_list* hojaDeViaje = list_create();
	char** arrayHojaDeViaje = config_get_array_value(metadata, "hojaDeViaje");
	log_info(log,"longitud hoja de viaje: %d", sizeof(arrayHojaDeViaje));
	int i,j;
	for(i=0; arrayHojaDeViaje[i]!= NULL; i++){
		t_objetivosPorMapa* objetivosPorMapa = malloc(sizeof(t_objetivosPorMapa*));
		objetivosPorMapa->mapa = arrayHojaViaje[i];
		objetivosPorMapa->objetivos = list_create();
		char* metadataObjetivo = string_new();
		string_append_with_format(&metadataObjetivo, "obj[%s]", objetivosPorMapa->mapa);
		char** objetivos = config_get_array_value(metadata, metadataObjetivo);
		for(j=0; objetivos[j]!= NULL; j++){
			if(j>0)if(string_starts_with(objetivos[j], objetivos[j-1])) return NULL;
			log_info(log, "Para el mapa %d, %s, objetivos: %d -> %s", i, objetivosPorMapa ->mapa, j, objetivos[j]);//para tal mapa, tales objetivos
			list_add(objetivosPorMapa->objetivos, objetivos[j]);
			}
			list_add(hojaDeViaje, objetivosPorMapa);
	}
	log_destroy(log);
	return hojaDeViaje;
	}
	*/


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
