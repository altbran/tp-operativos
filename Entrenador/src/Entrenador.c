#include "funcionesE.h"


int main(int argc, char** argv){

	//creo e inicio rutas
	char* rutaMetadata = string_new();
	char* rutaDirBill= string_new();
	char* ruta = string_new();


	if(argc != 3){

	  	string_append(&rutaMetadata, "/home/utnso/tp-2016-2c-A-cara-de-rope/Entrenador/Entrenadores/Ash/MetadataEntrenador.txt");
	  	string_append(&rutaDirBill, "/home/utnso/tp-2016-2c-A-cara-de-rope/Entrenador/Entrenadores/Ash/Directorio' 'de' 'Bill");
	//	log_error(logger,"Numero de parametros incorrectos");
	//	return 1;

	}
	else{

		string_append(&rutaMetadata, argv[2]);
		string_append(&rutaMetadata,"/Entrenadores/");
		string_append(&rutaMetadata, argv[1]);
		string_append(&ruta, rutaMetadata);

		string_append(&rutaMetadata, "/MetadataEntrenador.txt");
		string_append(&rutaDirBill, ruta);
		string_append(&rutaDirBill, "/Directorio' 'de' 'Bill");


	}


	//creo el config y el log
	t_config* metaDataEntrenador = config_create(rutaMetadata);
	logger = log_create("Entrenador.log", "ENTRENADOR", 0, LOG_LEVEL_INFO);


	//leo del config mis datos y lo destruyo
	cargarDatos(metaDataEntrenador);
	config_destroy(metaDataEntrenador);


	log_info(logger,"Entrenador leera sus atributos de la ruta %s", rutaMetadata);


	//seniales recibir y quitar vida
	signal(SIGUSR1,senialRecibirVida);
	signal(SIGTERM,senialQuitarVida);

	//momento de inicio
	tiempoDeInicio = temporal_get_string_time();
	tiempoBloqueo = "00:00:00:000";

	int i;
	for(i=0;i < list_size(entrenador.hojaDeViaje); i++){ //comienzo a leer los mapas de la hoja de viaje


		volverAEmpezar = 5;
		int murio;
		t_list* pokemonesAtrapados = list_create();		//POKEMONES ATRAPADOS en este mapa, LISTA CON STRUCTS METADATA POKEMON
		t_objetivosPorMapa* elemento = malloc(sizeof(t_objetivosPorMapa));//reservo memoria p/ leer el mapa con sus objetivos
		elemento = list_get(entrenador.hojaDeViaje,i);//le asigno al contenido del puntero, el mapa con sus objetivos
		nombreMapa = string_new();
		nombreMapa = elemento->mapa;
		log_info(logger, "Mapa a completar: %s", nombreMapa);

		//creo la ruta del metadata mapa
		char* rutaMetadataMapa = string_new();
		//string_append(&rutaMetadataMapa,"mnt/pokedex/Mapas/");
		//string_append(&rutaMetadataMapa,"mnt/pokedex/Mapas/");
		string_append(&rutaMetadataMapa,"/home/utnso/tp-2016-2c-A-cara-de-rope/mapa/Mapas/");
		string_append(&rutaMetadataMapa,nombreMapa);
		string_append(&rutaMetadataMapa,"/metadata");

		log_info(logger, "ruta metadata del mapa: %s", rutaMetadataMapa);

		//cargo posicion en (0;0) y ultimo movimiento = 'y'
		reestablecerDatos();


		//creo el config del metadata mapa
		t_config* metadataMapa = config_create(rutaMetadataMapa);


		//leo los datos del mapa q me interesan
		IP_MAPA_SERVIDOR = config_get_string_value(metadataMapa, "IP");
		PUERTO_MAPA_SERVIDOR = config_get_int_value(metadataMapa, "Puerto");


		//elimino el config del metadata mapa
		config_destroy(metadataMapa);


		//me conecto con el mapa
		if(crearSocket(&servidorMapa)){
				log_error(logger, "No se pudo crear socket cliente");
				return 1;
				}
				log_info(logger, "Socket mapa creado");


		if(conectarA(servidorMapa, "127.0.0.1", 8000)){
				log_error(logger, "Fallo al conectarse al servidor.");
				return 1;
			}
		log_info(logger, "Se ha iniciado conexion con el servidor mapa %s", nombreMapa);

		if(responderHandshake(servidorMapa, IDENTRENADOR, IDMAPA)){
				log_error(logger, "No se pudo responder handshake");
				return 1;
			}
			log_info(logger, "Conexion establecida");


		//le paso mis datos
		enviarMisDatos(servidorMapa);

		int j;

		//EMPIEZO A BUSCAR POKEMONES
		for(j=0; j < list_size(elemento->objetivos);j++){

			char* puntero = (char*)list_get(elemento->objetivos,j);
			char pkm = *puntero;
			t_pokemon* pokemon = malloc(sizeof(t_metadataPokemon));
			t_metadataPokenest* pokenestProxima = malloc(sizeof(t_metadataPokenest));
			pokenestProxima->identificador = pkm;
			pokemon->mapa = nombreMapa;

			char resultado;				//resultado, en caso de no tener mas vidas y elejir entre seguir jugando o no
			int estado = 0; 			//representa el estado en q se encuentra de la captura de un pokemon el entrenador

			char* tiempoSolicitoAtraparPkm;
			char* tiempoAtrapePkm;

			while (estado != 4){
				switch (estado){

		//solicito la ubicacion de la pokenest, guardo la ubicacion pknest y el nombre del pkm y envio la cantidad de movs a pokenest
						case 0:

							solicitarUbicacionPokenest(servidorMapa, pkm);
							recibirYAsignarCoordPokenest(servidorMapa, *pokenestProxima,pokemon->nombre);
							log_info(logger,"nombre del pokemon: %s",pokemon->nombre);
							//recibirNombrePkm(servidorMapa,pokemon->nombre);
							enviarCantidadDeMovsAPokenest(*pokenestProxima,servidorMapa);
							estado = 1;

						break;

						//le pido al mapa permiso para moverme y chequeo si llegue a la pokenest
						case 1:

							solicitarMovimiento(servidorMapa,*pokenestProxima);
							if(recibirHeader(servidorMapa) == movimientoAceptado){
								if(llegoAPokenest(*pokenestProxima)){
									estado = 2;
									log_info(logger, "Entrenador alcanza pokenest de %s", pokemon->nombre);
								}
							}

						break;

						case 2://en este estado solicito atrapar pokemon y chequeo si entro en estado deadlock

							solicitarAtraparPkm(pkm,servidorMapa);
							tiempoSolicitoAtraparPkm = temporal_get_string_time();

							switch (recibirHeader(servidorMapa)){

										case notificarDeadlock:

											cantidadDeadlocks++;
											log_info(logger, "Entrenador entra en Deadlock");
											enviarPokemonMasFuerte(pokemonesAtrapados,servidorMapa);
											estado = 3;
											break;

										case pokemonesDisponibles:

											recibirTodo(servidorMapa,&pokemon->numero,sizeof(int));
											tiempoAtrapePkm = temporal_get_string_time();
											sumarTiempos(&tiempoBloqueo, diferenciaDeTiempo(tiempoSolicitoAtraparPkm, tiempoAtrapePkm));
											log_info(logger,"Entrenador capturó correctamente pokemon %s", pokemon->nombre);
											estado = 4;
											break;
							}

							break;

						case 3://estado deadlock, abarca el caso perdedor y ganador

							if(recibirHeader(servidorMapa) == entrenadorGanador){
								log_info(logger, "Entrenador sale vencedor de la batalla");
								estado = 2;
							}
							else
								if(recibirHeader(servidorMapa) == entrenadorPerdedor){

									printf("Entrenador salio perdedor de la batalla y por eso morira, perdera todos sus pokemons atrapados en el mapa y"
												"se desconectara del mismo/n");

									log_info(logger, "Entrenador sale perdedor de la batalla");

									murio = 1;
									muertes++;

									// si no le quedan vidas al entrenador, tiene la opcion de volver a empezar
									if(entrenador.vidas <= 0){

										printf("Vidas insuficientes.\nCantidad de reintentos: %d\n "
												"Desea volver a jugar? Ingrese 's' para si, 'n' para no",entrenador.reintentos);

										scanf("%c", &resultado);

										//opcion si
										if(resultado == 's')
											volverAEmpezar = 1;
										else{
											//opcion no
											if (resultado == 'n')
												volverAEmpezar = 0;
										}

										list_clean(pokemonesAtrapados);
										break;
									}

									entrenador.vidas --;

									j = 0;//empieza a leer los objs desde 0
									i--;//vuelve a conectarse al mismo mapa

									//elimino los archivos de los pokemones copiados de este mapa y  los borro de la lista de atrapados
									eliminarArchivosPokemones(list_filter(pokemonesAtrapados,(void*) filtrarMapa), ruta);
									pokemonesAtrapados = list_filter(pokemonesAtrapados,(void*) distintoMapa);

									//fixme CHEQUEAR
									log_info(logger,"El Entrenador pierde una vida y se vuelve a conectar al mapa");
									desconectarseDe(servidorMapa);
								}
							break;
							}
			}//termina while estado


			//primero chequeo si se murio
			if(murio)
				break;

			//creo la ruta del metadata pkm
			char* numeroPokemon = obtenerNumero(pokemon->numero);
			char* rutaPokemon = string_new();
			rutaPokemon =  armarRutaPokemon(nombreMapa,pokemon->nombre, numeroPokemon);

			//creo el config para leerlo
			t_config* metadataPokemon = config_create(rutaPokemon);

			//leo el nivel del pokemon
			pokemon->nivel = config_get_int_value(metadataPokemon, "nivel");

			//agrego el pkm a la lista de pokemones atrapados
			list_add(pokemonesAtrapados, pokemon);


			//copio el archivo pkm en mi directorio Bill
			char* comando = string_new();
			comando = copiarArchivo(rutaPokemon,rutaDirBill);
			system(comando);


			// AVISO QUE COPIE EL POKEMON
			enviarHeader(servidorMapa,entrenadorListo);

			free(pokenestProxima);
			free(pokemon);

		}//aca cierra el for de atrapar pokemones



		//switch de : a)volver a empezar la ruta de viaje  b)dejar de jugar  c)haber atrapado  pokemon

		switch(volverAEmpezar){

		case 0:
			log_info(logger,"El Entrenador abandona el juego");
			break;

		case 1:
				i = -1; //gracias a esto empieza desde cero su ruta de viaje
				entrenador.reintentos ++;
				t_config* config = config_create(rutaMetadata);
				entrenador.vidas = config_get_int_value(metaDataEntrenador, "vidas");
				config_destroy(config);
				list_clean(pokemonesAtrapados);
				removerMedallas(entrenador.nombre);
				removerPokemones(entrenador.nombre);
				log_info(logger,"El Entrenador vuelve a empezar el juego desde cero");
			break;

		case 5:
				copiarMedalla(nombreMapa);
				free(elemento);
				enviarHeader(servidorMapa,finalizoMapa);
				desconectarseDe(servidorMapa);
				log_info(logger,"El Entrenador finalizo el mapa %s",nombreMapa);
			break;

	}


	if(!volverAEmpezar)
		break;

	}


	if(volverAEmpezar){

		tiempoFinal = temporal_get_string_time();

		printf("Felicidades!! Te has convertido en Maestro Pokemon.\nDatos de la aventura:\n"
				"Cantidad de veces involucrado en Deadlocks: %d\nCantidad de muertes: %d\nTiempo total de la aventura: %s\nTiempo"
				"total bloqueado en Pokenests(hh:mm:ss:mmmm): %s",
				cantidadDeadlocks, muertes, diferenciaDeTiempo(tiempoDeInicio,tiempoFinal),tiempoBloqueo);

		log_info(logger,"El Entrenador se convirtio en Maestro Pokemon");

	}



	return EXIT_SUCCESS;
	}
