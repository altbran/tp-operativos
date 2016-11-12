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



	cargarDatos(metaDataEntrenador);//cargo metadata


	config_destroy(metaDataEntrenador);

	log_info(logger,"Entrenador leera sus atributos de la ruta %s", rutaMetadata);



	//signal(SIGUSR1,senialRecibirVida);
	//signal(SIGTERM,senialQuitarVida);

	//char* tiempoDeInicio = temporal_get_string_time();

	int i;
	for(i=0;i < list_size(entrenador.hojaDeViaje); i++){ //comienzo a leer los mapas de la hoja de viaje

		int murio;
		t_list* pokemonesAtrapados = list_create();		//POKEMONES ATRAPADOS en este mapa, LISTA CON STRUCTS METADATA POKEMON
		t_objetivosPorMapa* elemento = malloc(sizeof(t_objetivosPorMapa));//reservo memoria p/ leer el mapa con sus objetivos
		elemento = list_get(entrenador.hojaDeViaje,i);//le asigno al contenido del puntero, el mapa con sus objetivos
		char* nombreMapa = string_new();
		nombreMapa = elemento->mapa;
		log_info(logger, "Mapa a completar: %s", nombreMapa);
		char* rutaMetadataMapa = string_new();
		string_append(&rutaMetadataMapa,"mnt/pokedex/Mapas/");
		string_append(&rutaMetadataMapa,nombreMapa);			//CREO EL PATH DE METADATA MAPA
		string_append(&rutaMetadataMapa,"/MetadataMapa.txt");

		log_info(logger, "ruta metadata del mapa: %s", rutaMetadataMapa);
		reestablecerDatos(); //cargo posicion en (0;0) y ultimo movimiento = 'y' para que empiece moviendose horizontalmente
		t_config* metadataMapa = config_create(rutaMetadataMapa);

		IP_MAPA_SERVIDOR = config_get_string_value(metadataMapa, "ip");
		PUERTO_MAPA_SERVIDOR = config_get_int_value(metadataMapa, "puerto");

		if(crearSocket(&servidorMapa)){
		log_error(logger, "No se pudo crear socket cliente");
		return 1;
		}
		log_info(logger, "Socket mapa creado");

		config_destroy(metadataMapa);

		if(conectarA(servidorMapa, IP_MAPA_SERVIDOR, PUERTO_MAPA_SERVIDOR)){
				log_error(logger, "Fallo al conectarse al servidor.");
				return 1;
			}
		log_info(logger, "Se ha iniciado conexion con el servidor mapa %s", nombreMapa);

		if(responderHandshake(servidorMapa, IDENTRENADOR, IDMAPA)){
				log_error(logger, "No se pudo responder handshake");
				return 1;
			}
			log_info(logger, "Conexion establecida");


		enviarMisDatos(servidorMapa);	//LE ENVIO MIS DATOS ENTRENADOR


		int j;
		for(j=0; j < list_size(elemento->objetivos);j++){ 		//EMPIEZO A BUSCAR POKEMONES
			char* puntero = list_get(elemento->objetivos,j);
			char pkm = *puntero;
			t_metadataPokemon pokemon;
			strcpy(pokemon.nombre,"");	// no se por que, pero para que no reciba basura tengo que poner esto
			char resultado;				//resultado, en caso de no tener mas vidas y elejir entre seguir jugando o no
			int estado = 0; 			//representa el estado en q se encuentra de la captura de un pokemon el entrenador
			t_metadataPokenest* pokenestProxima = malloc(sizeof(t_metadataPokenest));
			pokenestProxima->identificador = pkm;
			int numeroPkm;

			//dependiendo del estado, solicito empezar a jugar, moverme o atrapar un pokemon
			while (estado != 4){
				switch (estado){
						case 0:
							solicitarUbicacionPokenest(servidorMapa, pkm);	//solicito la ubicacion de la pokenest
							recibirYAsignarCoordPokenest(servidorMapa, *pokenestProxima);	//recibo y asigno las coordenadas de la pkn
							recibirNombrePkm(servidorMapa,pokemon.nombre);			//recibo y asigno el nombre del pokemon
							enviarCantidadDeMovsAPokenest(*pokenestProxima,servidorMapa);	//le envio la distancia al mapa hasta la pokenest
							estado = 1;
						break;

						case 1:
							solicitarMovimiento(servidorMapa,*pokenestProxima);//le pido al mapa permiso para moverme
							if(recibirHeader(servidorMapa) == movimientoAceptado){
							if(llegoAPokenest(*pokenestProxima)){
								estado = 2;
								log_info(logger, "Entrenador alcanza pokenest de %s", pokemon.nombre);
							}
							}
						break;

						case 2://en este estado solicito atrapar pokemon y chequeo si entro en estado deadlock

							solicitarAtraparPkm(pkm,servidorMapa);
							switch (recibirHeader(servidorMapa)){
							case notificarDeadlock:
								cantidadDeadlocks++;
								log_info(logger, "Entrenador entra en Deadlock");
								enviarPokemonMasFuerte(pokemonesAtrapados,servidorMapa);
								estado = 3;
								break;

							case pokemonesDisponibles:
								recibirTodo(servidorMapa,&numeroPkm,sizeof(int)); //recibo del server el numero de mi pokemon
								log_info(logger,"Entrenador capturó correctamente pokemon %s", pokemon.nombre);
								estado = 4;
								break;
							}

							break;

						case 3://estado deadlock, abarca el caso perdedor y ganador
							if(recibirHeader(servidorMapa) == entrenadorGanador){ 			//caso en que sali ganador del deadlock
								log_info(logger, "Entrenador sale vencedor del Deadlock");
								estado = 2;
							}
							else
								if(recibirHeader(servidorMapa) == entrenadorPerdedor){		// caso en que sali perdedor del deadlock
									printf("Entrenador salio perdedor de la batalla y por eso morira, perdera todos sus pokemons atrapados en el mapa y"
												"se desconectara del mismo/n");
									log_info(logger, "Entrenador sale perdedor del Deadlock");
									murio = 1;
									muertes++;
									if(entrenador.vidas <= 0){	// si no le quedan vidas al entrenador, tiene la opcion de volver a empezar
										printf("Vidas insuficientes.\nCantidad de reintentos realizados: %d\n "
												"Desea volver a jugar? Ingrese 's' para si, 'n' para no",entrenador.reintentos);

										scanf("%c", &resultado);
										if(resultado == 's') //si elijo la opcion "si" se suma un reintento y vuelvo a empezar
											volverAEmpezar = 1;

										if (resultado == 'n')//si elijo que no, me desconecto y termino de jugar
											volverAEmpezar = 0;

										break;
									}

									entrenador.vidas --;
									list_clean(pokemonesAtrapados);
									j = 0;//empieza a leer los objs desde 0
									i--;//gracias a esto vuelve a conectarse al mismo mapa

									desconectarseDe(servidorMapa);
								}
							break;
							}
			}

			if(murio)//si se murio es xq no pudo capturar el pokemon, por eso no realizara los pasos q siguen
				break;

			char* numeroPokemon = obtenerNumero(numeroPkm);
			char* rutaPokemon = string_new();
			rutaPokemon =  armarRutaPokemon(nombreMapa,pokemon.nombre, numeroPokemon);

			t_config* metadataPokemon = config_create(rutaPokemon);
			pokemon.nivel = config_get_int_value(metadataPokemon, "nivel");		//leo el nivel del pokemon

			list_add(pokemonesAtrapados, &pokemon); //agrego el pkm a la lista de pokemones atrapados

			char* rutaDirBill= string_new();
			rutaDirBill = crearRutaDirBill(rutaMetadata);

			char* comando = string_new();
			comando = crearComando(rutaPokemon,rutaDirBill);
			system(comando);									//copio el archivo pkm en mi directorio Bill

			// AVISO QUE COPIE EL POKEMON
			enviarHeader(servidorMapa,entrenadorListo);

			free(pokenestProxima);

		}//aca cierra el for de atrapar pokemones

		if(volverAEmpezar == 0)
			break;
		else
			if(volverAEmpezar == 1){
				i = -1; //gracias a esto empieza desde cero su ruta de viaje
				entrenador.reintentos ++;
				list_clean(pokemonesAtrapados);
				removerMedallas(entrenador.nombre);
				removerPokemones(entrenador.nombre);
			}

		copiarMedalla(nombreMapa);
		free(elemento);
		enviarHeader(servidorMapa,finalizoMapa);
		desconectarseDe(servidorMapa);
	}

	if(volverAEmpezar!=0){
		printf("Felicidades!! Te has convertido en Maestro Pokemon.\nDatos de la aventura:\n"
				"Cantidad de veces involucrado en Deadlocks: %d\nCantidad de muertes: %d",
				cantidadDeadlocks, muertes); //todo falta calcular el tiempo total que tardo en recorrer la hoja de viaje y el tiempo
											//bloqueado
	}

	return EXIT_SUCCESS;
	}
