#include "funcionesE.h"

int main(int argc, char** argv) {

	//creo e inicio rutas
	char* rutaMetadata = string_new();
	rutaDirBill = string_new();
	char* ruta = string_new();
	rutaMedallas = string_new();

	if (argc != 3) {

		string_append(&rutaMetadata,
				"/home/utnso/tp-2016-2c-A-cara-de-rope/mimnt/Entrenadores/Colo/metadata.txt");
		string_append(&rutaDirBill, "/home/utnso/tp-2016-2c-A-cara-de-rope/mimnt/Entrenadores/Colo/Dir' 'de' 'Bill");
		rutaMontaje = "/home/utnso/tp-2016-2c-A-cara-de-rope/mimnt";
		//	log_error(logger,"Numero de parametros incorrectos");
		//	return 1;

	} else {
		rutaMontaje = argv[2];
		string_append(&rutaMetadata, rutaMontaje);
		string_append(&rutaMetadata, "/Entrenadores/");
		string_append(&rutaMetadata, argv[1]);
		string_append(&ruta, rutaMetadata);

		string_append(&rutaMetadata, "/metadata.txt");
		string_append(&rutaDirBill, ruta);
		string_append(&rutaDirBill, "/Dir' 'de' 'Bill");

	}
	string_append(&rutaMedallas, rutaDirBill);
	string_append(&rutaMedallas, "/medallas/");

	//creo el config y el log
	char* nombreLog = string_new();
	string_append(&nombreLog, argv[1]);
	string_append(&nombreLog, ".log");
	t_config* metaDataEntrenador = config_create(rutaMetadata);
	logger = log_create(nombreLog, "ENTRENADOR", 0, LOG_LEVEL_INFO);

	//leo del config mis datos y lo destruyo
	cargarDatos(metaDataEntrenador);

	log_info(logger, "Entrenador leera sus atributos de la ruta %s", rutaMetadata);

	//seniales recibir y quitar vida
	signal(SIGUSR1, senialRecibirVida);
	signal(SIGTERM, senialQuitarVida);

	//momento de inicio
	diferencia = 0;
	tiempoDeInicio = temporal_get_string_time();
	pokemonesAtrapados = list_create();		//POKEMONES ATRAPADOS en este mapa, LISTA CON STRUCTS METADATA POKEMON
	muertes = 0;
	medallas = 0;

	int i;
	i = 0;
	for (; i < list_size(entrenador.hojaDeViaje); i++) { //comienzo a leer los mapas de la hoja de viaje

		volverAEmpezar = 5;
		murio = 0;
		elemento = malloc(sizeof(t_objetivosPorMapa)); //reservo memoria p/ leer el mapa con sus objetivos
		elemento = list_get(entrenador.hojaDeViaje, i); //le asigno al contenido del puntero, el mapa con sus objetivos
		nombreMapa = string_new();
		nombreMapa = elemento->mapa;
		log_info(logger, "Mapa a completar: %s", nombreMapa);

		//creo la ruta del metadata mapa
		rutaMetadataMapa = string_new();

		string_append(&rutaMetadataMapa, rutaMontaje);
		string_append(&rutaMetadataMapa, "/Mapas/");
		string_append(&rutaMetadataMapa, nombreMapa);
		string_append(&rutaMetadataMapa, "/metadata");

		log_info(logger, "ruta metadata del mapa: %s", rutaMetadataMapa);

		//cargo posicion en (0;0) y ultimo movimiento = 'y'
		reestablecerDatos();

		//creo el config del metadata mapa
		metadataMapa = config_create(rutaMetadataMapa);

		//leo los datos del mapa q me interesan
		IP_MAPA_SERVIDOR = config_get_string_value(metadataMapa, "IP");
		PUERTO_MAPA_SERVIDOR = config_get_int_value(metadataMapa, "Puerto");

		//me conecto con el mapa
		if (crearSocket(&servidorMapa)) {
			log_error(logger, "No se pudo crear socket cliente");
			return 1;
		}
		log_info(logger, "Socket mapa creado");

		if (conectarA(servidorMapa, IP_MAPA_SERVIDOR, PUERTO_MAPA_SERVIDOR)) {
			log_info(logger, "ip: %s puerto: %d", IP_MAPA_SERVIDOR, PUERTO_MAPA_SERVIDOR);
			log_error(logger, "Fallo al conectarse al servidor.");
			return 1;
		}
		log_info(logger, "Se ha iniciado conexion con el servidor mapa %s", nombreMapa);

		if (responderHandshake(servidorMapa, IDENTRENADOR, IDMAPA)) {
			log_error(logger, "No se pudo responder handshake");
			return 1;
		}
		log_info(logger, "Conexion establecida");

		//elimino el config del metadata mapa
		config_destroy(metadataMapa);

		//le paso mis datos
		enviarMisDatos(servidorMapa);

		//EMPIEZO A BUSCAR POKEMONES
		for (j = 0; j < list_size(elemento->objetivos); j++) {

			t_pokemon* pokemon = malloc(sizeof(t_pokemon));
			pokenestProxima = malloc(sizeof(t_metadataPokenest));

			puntero = (char*) list_get(elemento->objetivos, j);
			pkm = *puntero;

			pokenestProxima->identificador = pkm;
			pokemon->mapa = nombreMapa;

			estado = 0; 			//representa el estado en q se encuentra de la captura de un pokemon el entrenador

			while (estado != 5) {
				switch (estado) {

				//solicito la ubicacion de la pokenest, guardo la ubicacion pknest y el nombre del pkm y envio la cantidad de movs a pokenest
				case 0:

					solicitarUbicacionPokenest(servidorMapa, pkm);
					//pokemon->nombre[18] = '\0';

					recibirYAsignarCoordPokenest(servidorMapa, pokenestProxima, pokemon->nombre);

					enviarCantidadDeMovsAPokenest(pokenestProxima, servidorMapa);
					estado = 1;

					break;

					//le pido al mapa permiso para moverme y chequeo si llegue a la pokenest
				case 1:

					//solicitarMovimiento(servidorMapa,*pokenestProxima);
					solicitarMovimiento(servidorMapa, *pokenestProxima);
					if (recibirHeader(servidorMapa) == movimientoAceptado) {
						if (llegoAPokenest(*pokenestProxima)) {
							estado = 2;
							log_info(logger, "Entrenador alcanza pokenest de %s", pokemon->nombre);
						}
					}
					else{
						desconectarseDe(servidorMapa);
						log_error(logger,"error al solicitar movimiento");
					}

					break;

				case 2: 			//en este estado solicito atrapar pokemon y chequeo si entro en estado deadlock

					solicitarAtraparPkm(pkm, servidorMapa);
					solicitoAtraparPkm = time(NULL);

					switch (recibirHeader(servidorMapa)) {

					case notificarDeadlock:

						if (mejorPokemon == recibirHeader(servidorMapa)) {
							cantidadDeadlocks++;
							log_info(logger, "Entrenador entra en Deadlock");
							enviarPokemonMasFuerte(pokemonesAtrapados, servidorMapa);
							estado = 3;
						}
						else{
							log_error(logger,"error al entrar recibir header mejorPokemon");
							desconectarseDe(servidorMapa);
						}
						break;

					case pokemonesDisponibles:

						if(recibirTodo(servidorMapa, &pokemon->numero, sizeof(int))){
							log_error(logger,"error al recibir numero del pokemon");
							desconectarseDe(servidorMapa);
						}

						atrapePkm = time(NULL);
						diferencia += difftime(atrapePkm, solicitoAtraparPkm);
						log_info(logger, "Entrenador capturó correctamente pokemon %s", pokemon->nombre);

						estado = 5;

						break;

					default:

						log_error(logger,"error al recibir header para atrapar pokemon");
						desconectarseDe(servidorMapa);
					}

					break;

				case 3: 			//estado deadlock, abarca el caso perdedor y ganador
					;
					int headercito = recibirHeader(servidorMapa);

					switch(headercito){
					case entrenadorGanador:
						log_info(logger, "Entrenador sale vencedor de la batalla");
						estado = 4;
						break;

					case entrenadorMuerto:

						desconectarseDe(servidorMapa);

						printf("Entrenador salio perdedor de la batalla y por eso morira, "
								"perdera todos sus pokemons atrapados en el mapa y "
								"se desconectara del mismo\n");

						log_info(logger, "Entrenador sale perdedor de la batalla");

						murio = 1;

						muertes++;
						entrenador.vidas--;
						// si no le quedan vidas al entrenador, tiene la opcion de volver a empezar
						if (entrenador.vidas <= 0) {

							printf("Vidas insuficientes.\nCantidad de reintentos: %d\n"
									"Desea volver a jugar? Ingrese 's' para si, 'n' para no\n", entrenador.reintentos);

							scanf("%c", &resultado);

							//opcion si
							if (resultado == 's'){
								volverAEmpezar = 1;
								log_info(logger,"Eligio la opcion de volver a jugar");
							}
							else {
								//opcion no
								if (resultado == 'n'){
									volverAEmpezar = 0;
									log_info(logger,"Eligio la opcion de abandonar el juego");
								}

							}
							estado = 5;
							break;
						}

						j = 0; 			//empieza a leer los objs desde 0
						i--; 			//vuelve a conectarse al mismo mapa

						//elimino los archivos de los pokemones copiados de este mapa y  los borro de la lista de atrapados
						eliminarArchivosPokemones(list_filter(pokemonesAtrapados, (void*) filtrarMapa));
						pokemonesAtrapados = list_filter(pokemonesAtrapados, (void*) distintoMapa);

						estado = 5;
						volverAEmpezar = 6;

						log_info(logger, "El Entrenador pierde una vida y se vuelve a conectar al mapa");
						break;

					default:
						log_error(logger,"Error en estado deadlock");
						desconectarseDe(servidorMapa);
					}
					break;

				case 4:
					switch (recibirHeader(servidorMapa)) {

					case notificarDeadlock:

						if (mejorPokemon == recibirHeader(servidorMapa)) {
							cantidadDeadlocks++;
							log_info(logger, "Entrenador entra en Deadlock");
							enviarPokemonMasFuerte(pokemonesAtrapados, servidorMapa);
							estado = 3;
						}
						break;

					case pokemonesDisponibles:

						recibirTodo(servidorMapa, &pokemon->numero, sizeof(int));

						atrapePkm = time(NULL);
						diferencia += difftime(atrapePkm, solicitoAtraparPkm);
						log_info(logger, "Entrenador capturó correctamente pokemon %s", pokemon->nombre);

						estado = 5;

						break;
					default:
						log_error(logger,"error al solicitar atrapar pokemon, estado 4");
					}
				}
			} 			//termina while estado

			//primero chequeo si se murio
			if (murio) {
				log_info(logger, "cantidad de pokemones atrapados: %d", list_size(pokemonesAtrapados));
				break;
			}

			//creo la ruta del metadata pkm
			numeroPokemon = obtenerNumero(pokemon->numero);

			rutaPokemon = armarRutaPokemon(nombreMapa, pokemon->nombre, numeroPokemon);
			//string_append(&rutaPokemon,armarRutaPokemon(nombreMapa,pokemon->nombre, numeroPokemon));

			//creo el config para leerlo
			metadataPokemon = config_create(rutaPokemon);

			log_info(logger, "metadata del pokemon: %s", rutaPokemon);
			//leo el nivel del pokemon
			pokemon->nivel = config_get_int_value(metadataPokemon, "nivel");

			//agrego el pkm a la lista de pokemones atrapados
			list_add(pokemonesAtrapados, (void*) pokemon);
			log_info(logger, "cantidad de pokemones atrapados: %d", list_size(pokemonesAtrapados));

			//copio el archivo pkm en mi directorio Bill
			comando = string_new();
			comando = copiarArchivo(rutaPokemon, rutaDirBill);
			system(comando);

			// AVISO QUE COPIE EL POKEMON
			if (j == (list_size(elemento->objetivos) - 1)){
				if(enviarHeader(servidorMapa, finalizoMapa)){
					log_error(logger,"error al enviar header finalzoMapa");
				}
			}
			else
				if(enviarHeader(servidorMapa, entrenadorListo)){
					log_error(logger,"error al enviar header entrenadorListo");
				}

		} 			//aca cierra el for de atrapar pokemones

		free(pokenestProxima);

		//switch de : a)volver a empezar la ruta de viaje  b)dejar de jugar  c)haber atrapado  pokemon

		switch (volverAEmpezar) {

		case 0:
			log_info(logger, "El Entrenador abandona el juego");
			if(medallas > 0)
				removerMedallas(entrenador.nombre);
			eliminarArchivosPokemones(pokemonesAtrapados);
			printf("Entrenador abandona el juego\n");
			break;

		case 1:
			i = -1; //gracias a esto empieza desde cero su ruta de viaje
			entrenador.reintentos++;
			metaDataEntrenador = config_create(rutaMetadata);
			entrenador.vidas = config_get_int_value(metaDataEntrenador, "vidas");
			if(medallas > 0)
				removerMedallas(entrenador.nombre);
			eliminarArchivosPokemones(pokemonesAtrapados);
			list_clean(pokemonesAtrapados);
			log_info(logger, "El Entrenador vuelve a empezar el juego desde cero");
			break;

		case 5:
			copiarMedalla(nombreMapa);
			medallas++;
			desconectarseDe(servidorMapa);
			log_info(logger, "El Entrenador finalizo el mapa %s", nombreMapa);
			break;

		case 6:
			break;

		}

		if (!volverAEmpezar)
			break;

	}

	if (volverAEmpezar) {

		tiempoFinal = temporal_get_string_time();

		printf(
				"Felicidades!! Te has convertido en Maestro Pokemon.\nDatos de la aventura:\n"
						"Cantidad de veces involucrado en Deadlocks: %d\nCantidad de muertes: %d\nTiempo total de la aventura(hh:mm:ss:mmmm): %s\nTiempo"
						" total bloqueado en Pokenests(s): %.f\n", cantidadDeadlocks, muertes,
				diferenciaDeTiempo(tiempoDeInicio, tiempoFinal), diferencia);

		log_info(logger, "El Entrenador se convirtio en Maestro Pokemon");

	}
	config_destroy(metaDataEntrenador);

	return EXIT_SUCCESS;
}
