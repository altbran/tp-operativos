#include "funcionesE.h"
void senialRecibirVida();
void senialQuitarVida();
t_list* asignarHojaDeViajeYObjetivos(t_config*);
void moverEntrenador(t_metadataPokenest);
void moverEntrenadorcoordX(t_metadataPokenest);
void moverEntrenadorcoordY(t_metadataPokenest);
int cantidadDeMovimientosAPokenest(t_metadataPokenest);
bool llegoAPokenest(t_metadataPokenest);
void cargarDatos();
int cantidadDeMovimientosAPokenest(t_metadataPokenest);
char* crearRutaPkm(char*, char*,char*);
char* crearRutaDirBill(char*);
char* crearComando(char* ,char* );
void solicitarUbicacionPokenest(int,char);
void recibirYAsignarCoordPokenest(int,t_metadataPokenest);
void solicitarAtraparPkm(char, int);


int main(int argc, char** argv){

	if(argc != 3){
		printf("Numero de parametros incorrectos\n");
		log_error(logger,"Numero de parametros incorrectos\n",mensaje);
		return 1;
	}

	char* rutaMetadata = malloc(50);
	strcpy(rutaMetadata, argv[2]);
	char* nombreEntrenador = string_new();
	string_append(&rutaMetadata, "/Entrenadores/");
	string_append(&rutaMetadata, argv[1]);
	string_append(&rutaMetadata, nombreEntrenador);
	string_append(&rutaMetadata, "/MetadataEntrenador.txt");

	printf("ruta: %s",rutaMetadata);

	metaDataEntrenador = config_create(rutaMetadata);


	logger = log_create("Entrenador.log", "ENTRENADOR", 0, LOG_LEVEL_INFO);

	//entrenador.nombre = malloc(50);
	entrenador.nombre  = config_get_string_value(metaDataEntrenador, "nombre");//problema
	printf("nombre: %s", entrenador.nombre );
	log_info(logger,"Entrenador leera sus atributos de la ruta %s\n", rutaMetadata);

	//cargarDatos();


	signal(SIGUSR1,senialRecibirVida);
	signal(SIGTERM,senialQuitarVida);


	logger = log_create("Entrenador.log", "ENTRENADOR", 0, LOG_LEVEL_INFO);

	//creo el socket cliente
	if(crearSocket(&socketCliente)){
		log_error(logger, "No se pudo crear socket cliente");
		return 1;
	}
	log_info(logger, "Socket cliente creado");

	int i;
	for(i=0;i <= list_size(entrenador.hojaDeViaje); i++){ //comienzo a leer los mapas de la hoja de viaje
		t_objetivosPorMapa* elemento = malloc(sizeof(t_objetivosPorMapa));
		elemento = list_get(entrenador.hojaDeViaje,i);


		char* nombreMapa = elemento->mapa;
		char* rutaMetadataMapa = string_new();
		string_append(&rutaMetadataMapa,"mnt/pokedex/Mapas/");
		string_append(&rutaMetadataMapa,nombreMapa);

		t_config* metaDataMapa = config_create(rutaMetadataMapa);

		IP_MAPA_SERVIDOR = config_get_string_value(metaDataMapa, "ip");
		PUERTO_MAPA_SERVIDOR = config_get_int_value(metaDataMapa, "puerto");

		if(conectarA(servidorMapa, IP_MAPA_SERVIDOR, PUERTO_MAPA_SERVIDOR)){
				log_error(logger, "Fallo al conectarse al servidor.");
				return 1;
			}
		log_info(logger, "Se ha iniciado conexion con el servidor");

		if(responderHandshake(servidorMapa, IDENTRENADOR, IDMAPA)){
				log_error(logger, "No se pudo responder handshake");
				return 1;
			}
			log_info(logger, "Conexion establecida", mensaje);


		int j;
		for(j=0; j < list_size(elemento->objetivos);j++){

			char pokemon;
			pokemon = list_get(elemento->objetivos,j);
			int estado = 0;
			char numeroPokemon[3];
			t_metadataPokenest* pokenestProxima = malloc(sizeof(t_metadataPokenest));
			pokenestProxima->identificador = pokemon;
			void *buffer = malloc(3);
			while (estado != 3){
				switch (estado){
						case 0:
							solicitarUbicacionPokenest(servidorMapa, pokemon);
							recibirYAsignarCoordPokenest(servidorMapa, *pokenestProxima);
							estado = 1;
						break;

						case 1:
							if(llegoAPokenest(*pokenestProxima))
							estado = 2;
							else{
							moverEntrenador(*pokenestProxima);
							//avisarQueMeMuevo(servidorMapa);
							}

						break;

						case 2:
						solicitarAtraparPkm(pokemon,servidorMapa);
						if(!recibirTodo(servidorMapa, buffer, 3)){
							memcpy(&numeroPokemon,buffer,3);
						//aca puede ser elegido como victima, si lo es, mostrara en pantalla
						//los motivos, borrara los archivos en su directorio de bill, se
						//desconectara del mapa y perdera todos los pkms
						//si le quedan vidas disponibles, se le descuenta, si no,
						//tiene la opcion de volver a empezar, aumentando el contador de
						//reintentos
						// atrape pokemon,
							estado = 3;
						}
						free(buffer);
						break;
						}
				/*char* rutaPokemon = string_new();
				rutaPokemon = crearRutaPkm(nombreMapa, nombrePknest, numeroPokemon);

				char* rutaDirBill= string_new();
				rutaDirBill = crearRutaDirBill(rutaMetadata);

				char* comando = string_new();
				comando = crearComando(rutaPokemon,rutaDirBill);
				system(comando);*/
				}

		free(pokenestProxima);
		}
		//void solicitarMedallaMapa(){}
		//void cargarYCopiarMedalla(){}
				//config_destroy(metaDataMapa);
		//free(elemento); //elemento de hoja de viaje, osea un t_objetivosXMapa
		}
	//SE CONVIRTIO EN MAESTRO POKEMON, NOTIFICAR POR PANTALLA, INFORMAR TIEMPO TOTAL,
	//CUANTO TIEMPO PASO BLOQUEADO EN LAS POKENESTS, EN CUANTOS DEADBLOCKS ESTUVO INVOLUCRADO
	//Y CUANTAS VECES MURIO
	//free(rutaMetadata);
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
	counter = abs(pokenest.posicionX - ubicacionEntrenador.coordenadasX)
			+ abs(pokenest.posicionY - ubicacionEntrenador.coordenadasY);

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

bool llegoAPokenest(t_metadataPokenest pokenest){
	return((ubicacionEntrenador.coordenadasX == pokenest.posicionX) && (ubicacionEntrenador.coordenadasY == pokenest.posicionY));
}

void cargarDatos(){
	entrenador.nombre  = config_get_string_value(metaDataEntrenador, "nombre");
	log_info(logger,"El nombre del entrenador es: %s\n", entrenador.nombre);
	//entrenador.hojaDeViaje = asignarHojaDeViajeYObjetivos(metaDataEntrenador);
	char* simbolo = config_get_string_value(metaDataEntrenador, "simbolo");
	entrenador.simbolo = simbolo[0];
	log_info(logger,"El simbolo del entrenador es: %c", entrenador.simbolo);
	entrenador.vidas = config_get_int_value(metaDataEntrenador, "vidas");
	log_info(logger,"Las vidas del entrenador son: %d", entrenador.vidas);
	entrenador.reintentos = config_get_int_value(metaDataEntrenador, "reintentos");
	log_info(logger,"Los reintentos del entrenador son: %d", entrenador.reintentos);

	ubicacionEntrenador.coordenadasX = 0;
	ubicacionEntrenador.coordenadasY = 0;
	ubicacionEntrenador.ultimoMov = 'y';
}


char* crearRutaPkm(char* nombreMapa, char* nombrePknest, char* numeroPkm){
	char* rutaPokemon = string_new();
	string_append(&rutaPokemon,"/mnt/pokedex/Mapas/");
	string_append(&rutaPokemon,nombreMapa);
	string_append(&rutaPokemon,"/");
	string_append(&rutaPokemon,nombrePknest);
	string_append(&rutaPokemon,"/");
	string_append(&rutaPokemon,nombrePknest);
	string_append(&rutaPokemon, numeroPkm);
	string_append(&rutaPokemon,".dat");
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


void desconectarseDe(int socketServer){
	close(socketServer);
}
void recibirYAsignarCoordPokenest(int socketOrigen, t_metadataPokenest pokenest){

	void *buffer = malloc(sizeof(int) + sizeof(int));
	recv(socketOrigen, buffer, sizeof(t_metadataPokenest), 0);


	int cursorMemoria = 0;
	memcpy(&pokenest.posicionX,buffer, sizeof(uint32_t));

	cursorMemoria += sizeof(uint32_t);
	memcpy(&pokenest.posicionY,buffer + cursorMemoria, sizeof(uint32_t));

	free(buffer);
}
void solicitarUbicacionPokenest(int socketDestino,char pokemon){
	void *buffer=malloc(sizeof(char));
	memcpy(buffer,&pokemon,sizeof(char));
	send(socketDestino,buffer,sizeof(char),0);
	free(buffer);
}
/*void solicitarMedallaMapa(char* nombreMapa, int socketDestino){
	void *buffer = malloc(strlen(nombreMapa));		//PROBAR SI ANDA ESTO
	memcpy(buffer,nombreMapa,strlen(nombreMapa));
	send(socketDestino, buffer, strlen(nombreMapa), 0);
	free(buffer);
 } ES NECESARIO ESTO??*/

void copiarMedalla(char* nombreMapa){
	char* rutaOrigen = string_new();
	string_append(&rutaOrigen,"mnt/pokedex/Mapas/");
	string_append(&rutaOrigen, nombreMapa);
	string_append(&rutaOrigen, "/");
	string_append(&rutaOrigen, "medalla-");
	string_append(&rutaOrigen, nombreMapa);
	string_append(&rutaOrigen, ".jpg");

	char* rutaDestino = string_new();
	string_append(&rutaDestino,"mnt/pokedex/Entrenadores/");
	string_append(&rutaDestino, entrenador.nombre);
	string_append(&rutaDestino, "medallas");

	char* comando = crearComando(rutaOrigen,rutaDestino);
	system(comando);
}

void enviarCantidadDeMovsAPokenest(int socketDestino,t_metadataPokenest pokenest){
	int movs = cantidadDeMovimientosAPokenest(pokenest);
	void *buffer = malloc(sizeof(int));
	memcpy(buffer,&movs,sizeof(int));
	send(socketDestino,buffer,sizeof(int),0);
	free(buffer);
}

void solicitarAtraparPkm(char pokemon, int servidorMapa){
	void *buffer = malloc(sizeof(char));
	memcpy(buffer, &pokemon,sizeof(char));
	send(servidorMapa, buffer, sizeof(char), 0);
	free(buffer);
}
