#include "funcionesE.h"


t_list* asignarHojaDeViajeYObjetivos(t_config* metadata){
	//log = log_create("MetadataEntrenador.log", "LEER_HOJADEVIAJE", 0, LOG_LEVEL_INFO);
	t_list* hojaDeViaje = list_create();
	char** arrayHojaDeViaje = config_get_array_value(metadata, "hojaDeViaje");
	//log_info(log,"longitud hoja de viaje: %d", sizeof(arrayHojaDeViaje));
	int i,j;
	for(i=0; arrayHojaDeViaje[i]!= NULL; i++){ //recorro todos los mapas en la hoja de viaje
		t_objetivosPorMapa* objetivosPorMapa = malloc(sizeof(t_objetivosPorMapa*)); //reservo memoria para objetivosxmapa
		objetivosPorMapa->mapa = arrayHojaDeViaje[i]; //le asigno el nombre del mapa q este leyendo
		objetivosPorMapa->objetivos = list_create(); //creo la lista con sus objetivos
		char* metadataObjetivo = string_new(); //aca se va aguardar lo q tendra q leer del archivo config
		string_append_with_format(&metadataObjetivo, "obj[%s]", objetivosPorMapa->mapa);
		char** objetivos = config_get_array_value(metadata, metadataObjetivo); //leo del config los objetivos del mapa
		for(j=0; objetivos[j]!= NULL; j++){
			if(j>0)if(string_starts_with(objetivos[j], objetivos[j-1])) return NULL; //logica para que no pida conseguir dos veces seguidas el mismo pkm
			//log_info(log, "Para el mapa %d, %s, objetivos: %d -> %s", i, objetivosPorMapa ->mapa, j, objetivos[j]);//para tal mapa,cuales son sus objetivos
			list_add(objetivosPorMapa->objetivos, objetivos[j]);
			}			//agrego el objetivo leido al atributo de la struct objetivosPorMapa
			list_add(hojaDeViaje, objetivosPorMapa);//agrego el ObjetivosPorMapa a la lista hoja de viaje
	}
	//log_destroy(log);
	return hojaDeViaje;
	}//todavia no anda


int main(int argc, char** argv){

	if(argc != 3){
		printf("Numero de parametros incorrectos\n");
		log_error(logger,"Numero de parametros incorrectos\n",mensaje);
		return 1;
	}

	t_objetivosPorMapa hola;
	char rutaMetadata[30];
	char nombreEntrenador[15];

	strcpy(nombreEntrenador,argv[1]);
	strcpy(rutaMetadata,argv[2]);
	strcat(strcat(rutaMetadata,"/Entrenadores/"), nombreEntrenador);

	t_config* metaDataEntrenador = config_create(rutaMetadata);

	entrenador.nombre  = config_get_string_value(metaDataEntrenador, "nombre");
	entrenador.hojaDeViaje = asignarHojaDeViajeYObjetivos(metaDataEntrenador);
	char* simbolo = config_get_string_value(metaDataEntrenador, "simbolo");
	entrenador.simbolo = simbolo[0];
	entrenador.vidas = config_get_int_value(metaDataEntrenador, "vidas");
	entrenador.reintentos = config_get_int_value(metaDataEntrenador, "reintentos");



	char* IP_MAPA_SERVIDOR = getenv("IP_MAPA_SERVIDOR");
	logger = log_create("Entrenador.log", "ENTRENADOR", 0, LOG_LEVEL_INFO);

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

