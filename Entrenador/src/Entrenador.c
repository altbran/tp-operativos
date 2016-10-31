#include "funcionesE.h"


int main(int argc, char** argv){

	/*if(argc != 3){
		printf("Numero de parametros incorrectos\n");
		log_error(logger,"Numero de parametros incorrectos\n",mensaje);
		return 1;
	}
*/

	char* rutaMetadata = string_new();
	//strcpy(rutaMetadata, argv[2]);
	//char* nombreEntrenador = string_new();
	string_append(&rutaMetadata, "/home/utnso/tp-2016-2c-A-cara-de-rope/Entrenador/Entrenadores/Ash/MetadataEntrenador.txt");
	//string_append(&rutaMetadata, argv[1]);
	//string_append(&rutaMetadata, nombreEntrenador);
	//string_append(&rutaMetadata, "/MetadataEntrenador.txt");


	t_config* metaDataEntrenador;
	metaDataEntrenador = config_create(rutaMetadata);


	logger = log_create("Entrenador.log", "ENTRENADOR", 0, LOG_LEVEL_INFO);

	log_info(logger,"Entrenador leera sus atributos de la ruta %s\n", rutaMetadata);

	cargarDatos(metaDataEntrenador);//cargo metadata


	config_destroy(metaDataEntrenador);//YA LEI LO QUE QUE QUERIA, AHORA DESTRUYO EL CONFIG  DE ENTRENADOR

	//signal(SIGUSR1,senialRecibirVida);
	//signal(SIGTERM,senialQuitarVida);


	logger = log_create("Entrenador.log", "ENTRENADOR", 0, LOG_LEVEL_INFO);

	//creo el socket mapa
	if(crearSocket(&servidorMapa)){
		printf("no se pudo crear socket cliente");
		log_error(logger, "No se pudo crear socket cliente");
		return 1;
	}
	log_info(logger, "Socket mapa creado");

	reestablecerDatos(); //cargo posicion y ultimo movimiento
	/*t_config* metadataMapa = config_create(rutaMetadataMapa);

	IP_MAPA_SERVIDOR = config_get_string_value(metadataMapa, "ip");
	PUERTO_MAPA_SERVIDOR = config_get_int_value(metadataMapa, "puerto");

	config_destroy(metadataMapa);
	if(conectarA(servidorMapa, IP_MAPA_SERVIDOR, PUERTO_MAPA_SERVIDOR)){
			log_error(logger, "Fallo al conectarse al servidor.");
			return 1;
		}*/

	char * ip = "127.0.0.1";

	if(conectarA(servidorMapa, ip, 8080)){
				printf("no se pudo conectar\n");
				log_error(logger, "Fallo al conectarse al servidor.");
				return 1;
			}

	log_info(logger, "Se ha iniciado conexion con el servidor");
	printf("conexion establecida\n");
	if(responderHandshake(servidorMapa, IDENTRENADOR, IDMAPA)){
			printf("no se pudo handshake\n");
			log_error(logger, "No se pudo responder handshake");
			return 1;
		}
		log_info(logger, "Conexion establecida");

	printf("handshake correcto\n");

	enviarMisDatos(servidorMapa);



	sleep(60000000);
	//char* tiempoDeInicio = temporal_get_string_time();
	int i;
	for(i=0;i <= list_size(entrenador.hojaDeViaje); i++){ //comienzo a leer los mapas de la hoja de viaje
		t_objetivosPorMapa *elemento = malloc(sizeof(t_objetivosPorMapa));//reservo memoria p/ leer el mapa con sus objetivos
		elemento = list_get(entrenador.hojaDeViaje,i);//le asigno al contenido del puntero, el mapa con sus objetivos
														// segun indice
		char* nombreMapa = string_new();
		nombreMapa = elemento->mapa;
		char* rutaMetadataMapa = string_new();
		string_append(&rutaMetadataMapa,"mnt/pokedex/Mapas/");
		string_append(&rutaMetadataMapa,nombreMapa);			//CREO EL PATH DE METADATA MAPA


		reestablecerDatos(); //cargo posicion y ultimo movimiento
		t_config* metadataMapa = config_create(rutaMetadataMapa);

		IP_MAPA_SERVIDOR = config_get_string_value(metadataMapa, "ip");
		PUERTO_MAPA_SERVIDOR = config_get_int_value(metadataMapa, "puerto");

		if(crearSocket(&servidorMapa)){
		printf("no se pudo crear socket cliente");
		log_error(logger, "No se pudo crear socket cliente");
		return 1;
		}
		log_info(logger, "Socket mapa creado");

		config_destroy(metadataMapa);
		if(conectarA(servidorMapa, IP_MAPA_SERVIDOR, PUERTO_MAPA_SERVIDOR)){
				log_error(logger, "Fallo al conectarse al servidor.");
				return 1;
			}
		log_info(logger, "Se ha iniciado conexion con el servidor");

		if(responderHandshake(servidorMapa, IDENTRENADOR, IDMAPA)){
				log_error(logger, "No se pudo responder handshake");
				return 1;
			}
			log_info(logger, "Conexion establecida");


		enviarMisDatos(servidorMapa);//LE ENVIO MIS DATOS A ENTRENADOR


		int j;
		for(j=0; j < list_size(elemento->objetivos);j++){ //EMPIEZO A BUSCAR POKEMONES

			char pkm = list_get(elemento->objetivos,j);
			//char* nombrePokemon = obtenerNombre(pkm);//TODO HACER FUNCION obtener el nombre de mi pokemon
			t_metadataPokemon pokemon;
			//strcpy(pokemon.nombre,nombrePokemon);

			int estado = 0;
			t_list* pokemones = list_create();
			t_metadataPokenest* pokenestProxima = malloc(sizeof(t_metadataPokenest));
			pokenestProxima->identificador = pkm;
			while (estado != 3){
				switch (estado){
						case 0:
							solicitarUbicacionPokenest(servidorMapa, pkm);
							recibirYAsignarCoordPokenest(servidorMapa, *pokenestProxima);
							enviarCantidadDeMovsAPokenest(*pokenestProxima,servidorMapa);
							estado = 1;
						break;

						case 1:
							solicitarMovimiento(servidorMapa,*pokenestProxima);//le pido al mapa permiso para moverme
							hastaQueNoReciba(movimientoAceptado, servidorMapa);// <-funcion con bucle infinito,
							if(llegoAPokenest(*pokenestProxima)){			   //hasta que reciba el header
								estado = 2;									   //solicitado
								log_info(logger, "Entrenador alcanza pokenest del pokemon %c", pkm);
							}
						break;

						case 2:
							solicitarAtraparPkm(pkm,servidorMapa);
							//aca puede ser elegido como victima, si lo es, mostrara en pantalla
							//los motivos, borrara los archivos en su directorio de bill, se
							//desconectara del mapa y perdera todos los pkms
							//si le quedan vidas disponibles, se le descuenta, si no,
							//tiene la opcion de volver a empezar, aumentando el contador de
							//reintentos
							// atrape pokemon,
							hastaQueNoReciba(capturarPokemon, servidorMapa);
							//todo HACER CONFIG PARA OBTENER EL NIVEL DEL POKEMON
							list_add(pokemones, pkm);
							estado = 3;
							break;
							}
				char* rutaPokemon = string_new();
				//rutaPokemon = crearRutaPkm(nombreMapa, nombrePknest, numeroPokemon);//TODO leo el nombre del pkm
																					// de un path ??
				char* rutaDirBill= string_new();
				rutaDirBill = crearRutaDirBill(rutaMetadata);

				char* comando = string_new();
				comando = crearComando(rutaPokemon,rutaDirBill);
				system(comando);
				}

		free(pokenestProxima);

		}
		//solicitarYCopiarMedallaMapa(nombreMapa, servidorMapa);
		config_destroy(metadataMapa);
		free(elemento);
		desconectarseDe(servidorMapa);
		}
	//SE CONVIRTIO EN MAESTRO POKEMON, NOTIFICAR POR PANTALLA, INFORMAR TIEMPO TOTAL,
	//CUANTO TIEMPO PASO BLOQUEADO EN LAS POKENESTS, EN CUANTOS DEADBLOCKS ESTUVO INVOLUCRADO
	//Y CUANTAS VECES MURIO
	//free(rutaMetadata); ??VA*/


	return EXIT_SUCCESS;
	}
