#include "funcionesE.h"

//prototipo de funciones
void senialRecibirVida();
void senialQuitarVida();
t_list* asignarHojaDeViajeYObjetivos(t_config*);
void moverEntrenador(t_metadataPokenest);
void moverEntrenadorcoordX(t_metadataPokenest);
void moverEntrenadorcoordY(t_metadataPokenest);
int cantidadDeMovimientosAPokenest(t_metadataPokenest);
char* crearRutaPkm(char*, char*);
char* crearRutaDirBill(char*);
char* crearComando(char* ,char* );

int main(int argc, char** argv){

	if(argc != 3){
		printf("Numero de parametros incorrectos\n");
		log_error(logger,"Numero de parametros incorrectos\n",mensaje);
		return 1;
	}


	char rutaMetadata[30];
	char nombreEntrenador[15];

	strcpy(nombreEntrenador,argv[1]);
	strcpy(rutaMetadata,argv[2]);
	strcat(strcat(rutaMetadata,"/Entrenadores/"), nombreEntrenador);

	t_config* metaDataEntrenador = config_create(rutaMetadata);

	//obtengo del archivo config todos los atributos de metadata entrenador
	entrenador.nombre  = config_get_string_value(metaDataEntrenador, "nombre");
	entrenador.hojaDeViaje = asignarHojaDeViajeYObjetivos(metaDataEntrenador);
	char* simbolo = config_get_string_value(metaDataEntrenador, "simbolo");
	entrenador.simbolo = simbolo[0];
	entrenador.vidas = config_get_int_value(metaDataEntrenador, "vidas");
	entrenador.reintentos = config_get_int_value(metaDataEntrenador, "reintentos");

	ubicacionEntrenador.coordenadasX = 0;
	ubicacionEntrenador.coordenadasY = 0;
	ubicacionEntrenador.ultimoMov = 'y';

	signal(SIGUSR1,senialRecibirVida);
	signal(SIGTERM,senialQuitarVida);


	//char* IP_MAPA_SERVIDOR = getenv("IP_MAPA_SERVIDOR");
	logger = log_create("Entrenador.log", "ENTRENADOR", 0, LOG_LEVEL_INFO);

	//creo el socket cliente
	if(crearSocket(&socketCliente)){
		printf("No se pudo crear el socket.");
		return 1;
	}

	int i;
	for(i=0;i <= list_size(entrenador.hojaDeViaje); i++){ //comienzo a leer los mapas de la hoja de viaje
		t_objetivosPorMapa elemento = list_get(entrenador.hojaDeViaje,i);
		//VA COMO PUNTERO O NO??
		char rutaMetadataMapa[30];
		char nombreMapa[15];
		strcpy(nombreMapa, elemento.mapa);
		strcat(strcat(rutaMetadataMapa,"mnt/pokedex/Mapas/"), nombreMapa);

		t_config* metaDataMapa = config_create(rutaMetadataMapa);

		IP_MAPA_SERVIDOR = config_get_string_value(metaDataMapa, "ip");
		PUERTO_MAPA_SERVIDOR = config_get_int_value(metaDataMapa, "puerto");

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

		//tengo q saber cual es el proximo pokemon y su correspondiente pokenest
		int j;
		for(j=0; j < list_size(elemento.objetivos);j++){
		char pokemon[0] = list_get(elemento.objetivos,j);
		int estado = 0;
		t_metadataPokenest pokenestProxima;
		char* nombrePknest = pokenestProxima.identificador;
		while (estado != 3){
		switch (estado){
						case 0:
							//solicitarUbicacionPokenest(pokemon);
						//	recibirTodo(servidorMapa, pokenestProxima, sizeof(t_metadataPokenest));
							estado = 1;
						break;

						case 1:
							moverEntrenador(pokenestProxima);
							if((ubicacionEntrenador.coordenadasX ==pokenestProxima.posicionX) && ubicacionEntrenador.coordenadasY == pokenestProxima.posicionY)
									estado = 2;
						break;

						case 2: solicitarAtraparPkm();
						//aca puede ser elegido como victima, si lo es, mostrara en pantalla
						//los motivos, borrara los archivos en su directorio de bill, se
						//desconectara del mapa y perdera todos los pkms
						//si le quedan vidas disponibles, se le descuenta, si no,
						//tiene la opcion de volver a empezar, aumentando el contador de
						//reintentos
						break;
						}
		}
		char* rutaPokemon = string_new();
		rutaPokemon = crearRutaPkm(nombreMapa, nombrePknest);

		char* rutaDirBill= string_new();
		rutaDirBill = crearRutaDirBill(rutaMetadata);

		char* comando = string_new();
		comando = crearComando(rutaPokemon,rutaDirBill);

		system(comando);
		//copio el metadata del pokemon en el directorio de bill
		}
		// ACA COPIAR MEDALLA EN DIRECTORIO DE BILL
				config_destroy(metaDataMapa);
			}
	//SE CONVIRTIO EN MAESTRO POKEMON, NOTIFICAR POR PANTALLA, INFORMAR TIEMPO TOTAL,
	//CUANTO TIEMPO PASO BLOQUEADO EN LAS POKENESTS, EN CUANTOS DEADBLOCKS ESTUVO INVOLUCRADO
	//Y CUANTAS VECES MURIO
	return EXIT_SUCCESS;
	}



void senialRecibirVida(){
	entrenador.vidas = entrenador.vidas + 1;
}

void senialQuitarVida(){
	entrenador.vidas = entrenador.vidas - 1;
}

void moverEntrenador(t_metadataPokenest pokenest){

	if(ubicacionEntrenador.coordenadasX == pokenest.posicionX){
	moverEntrenadorcoordY(pokenest);;
	}
	else {
		if(ubicacionEntrenador.coordenadasY == pokenest.posicionY){
			moverEntrenadorcoordX(pokenest);
		}
		else{
			if(ubicacionEntrenador.ultimoMov == 'x'){
				moverEntrenadorcoordY(pokenest);
				ubicacionEntrenador.ultimoMov = 'y';
			}
			else
				if(ubicacionEntrenador.ultimoMov == 'y'){
					moverEntrenadorcoordX(pokenest);
					ubicacionEntrenador.ultimoMov = 'x';
				}
		}
	}
}

void moverEntrenadorcoordX(t_metadataPokenest pokenest){
if (pokenest.posicionX > ubicacionEntrenador.coordenadasX)
	ubicacionEntrenador.coordenadasX++;
else
	ubicacionEntrenador.coordenadasX--;
}

void moverEntrenadorcoordY(t_metadataPokenest pokenest){
	if (pokenest.posicionY > ubicacionEntrenador.coordenadasY)
		ubicacionEntrenador.coordenadasY++;
	else
		ubicacionEntrenador.coordenadasY--;
}

int cantidadDeMovimientosAPokenest(t_metadataPokenest pokenest){
	int counter = 0;
	while((ubicacionEntrenador.coordenadasX != pokenest.posicionX) || (ubicacionEntrenador.coordenadasY != pokenest.posicionY )){
		moverEntrenador(pokenest);
		counter++;
	}
	return counter;
}

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
			if(j>0)if(string_starts_with(objetivos[j], objetivos[j-1])) return NULL; //logica para que no pida  dos veces seguidas el mismo pkm
			//log_info(log, "Para el mapa %d, %s, objetivos: %d -> %s", i, objetivosPorMapa ->mapa, j, objetivos[j]);//para tal mapa,cuales son sus objetivos
			list_add(objetivosPorMapa->objetivos, objetivos[j]);
			}			//agrego el objetivo leido al atributo de la struct objetivosPorMapa
			list_add(hojaDeViaje, objetivosPorMapa);//agrego el ObjetivosPorMapa a la lista hoja de viaje
	}
	//log_destroy(log);
	return hojaDeViaje;
	}
char* crearRutaPkm(char* nombreMapa, char* nombrePknest){
	char* rutaPokemon = string_new();
	string_append(&rutaPokemon,"/mnt/pokedex/Mapas/");
	string_append(&rutaPokemon,nombreMapa);
	string_append(&rutaPokemon,"/");
	string_append(&rutaPokemon,nombrePknest);
	string_append(&rutaPokemon,"/");
	string_append(&rutaPokemon,nombrePknest);
	string_append(&rutaPokemon,"001.dat");
	return rutaPokemon;
}

char* crearRutaDirBill(char* ruta){
	char* rutaDirBill = string_new();
	string_append(&rutaDirBill,ruta);
	string_append(&rutaDirBill,"/Directorio' 'de' 'Bill/");
	return rutaDirBill;
}

char* crearComando(char* rutaOrigen,char* rutaDestino){
char* comando = string_new();
string_append(&comando,"cp ");
string_append(&comando, rutaOrigen);
string_append(&comando, " ");
string_append(&comando, rutaDestino);
return comando;
}
