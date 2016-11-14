/*
 * funcionesE.c
 *
 *  Created on: 29/10/2016
 *      Author: utnso
 */
#include "funcionesE.h"

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

	t_list* hojaDeViaje = list_create();
	char** arrayHojaDeViaje = config_get_array_value(metadata, "hojaDeViaje");

	int c=0;
	while(arrayHojaDeViaje[c]!= NULL)
		c++;
	log_info(logger,"Cantidad de mapas a recorrer para convertirse en Maestro Pokemon: %d", c);
	int i,j;
	t_objetivosPorMapa* objetivosPorMapa = malloc(sizeof(t_objetivosPorMapa));
	for(i=0; arrayHojaDeViaje[i]!= NULL; i++){ //recorro todos los mapas en la hoja de viaje
		t_objetivosPorMapa* objetivosPorMapa = malloc(sizeof(t_objetivosPorMapa)); //reservo memoria para objetivosxmapa
		objetivosPorMapa->mapa = arrayHojaDeViaje[i]; //le asigno el nombre del mapa q este leyendo
		objetivosPorMapa->objetivos = list_create(); //creo la lista con sus objetivos
		char* metadataObjetivo = string_new(); //aca se va aguardar lo q tendra q leer del archivo config
		string_append_with_format(&metadataObjetivo, "obj[%s]", objetivosPorMapa->mapa);
		char** objetivos = config_get_array_value(metadata, metadataObjetivo); //leo del config los objetivos del mapa
		for(j=0; objetivos[j]!= NULL; j++){
			if(j>0)if(string_starts_with(objetivos[j], objetivos[j-1])) return NULL; //logica para que no pida  dos veces seguidas el mismo pkm
			log_info(logger, "Para el mapa %d, %s, el objetivo numero %d es %s", i+1 , objetivosPorMapa ->mapa, j+1, objetivos[j]);//para tal mapa,cuales son sus objetivos
			list_add(objetivosPorMapa->objetivos, objetivos[j]);
			}			//agrego el objetivo leido al atributo de la struct objetivosPorMapa
			list_add(hojaDeViaje, objetivosPorMapa);//agrego el ObjetivosPorMapa a la lista hoja de viaje
	}
	return hojaDeViaje;
	}

int llegoAPokenest(t_metadataPokenest pokenest){
	return((ubicacionEntrenador.coordenadasX == pokenest.posicionX) && (ubicacionEntrenador.coordenadasY == pokenest.posicionY));
}

void cargarDatos(t_config* metaDataEntrenador){

	strncpy(entrenador.nombre, config_get_string_value(metaDataEntrenador, "nombre"), sizeof(entrenador.nombre)- 1);
	entrenador.nombre[17] = '\0';
	log_info(logger,"Nombre: %s", entrenador.nombre);
	char* simbolo = config_get_string_value(metaDataEntrenador, "simbolo");
	entrenador.simbolo = simbolo[0];
	log_info(logger,"Simbolo: %c", entrenador.simbolo);
	entrenador.vidas = config_get_int_value(metaDataEntrenador, "vidas");
	log_info(logger,"Vidas: %d", entrenador.vidas);
	entrenador.reintentos = config_get_int_value(metaDataEntrenador, "reintentos");
	log_info(logger,"Reintentos: %d", entrenador.reintentos);
	entrenador.hojaDeViaje = asignarHojaDeViajeYObjetivos(metaDataEntrenador);

}


char* armarRutaPokemon(char* nombreMapa, char* nombrePokenest, char* nro){
	char* path = string_new();
	string_append(&path, "/mnt/pokedex/Mapas/");
	string_append(&path,nombreMapa);
	string_append(&path,"/");
	string_append(&path,nombrePokenest);
	string_append(&path,"/");
	string_append(&path,nombrePokenest);
	string_append(&path,nro);
	string_append(&path,".dat");

	return path;
}


char* obtenerNumero(int numero){

	char* nro = string_new();
	if(numero < 10){
		string_append(&nro,"00");
		string_append(&nro,string_itoa(numero));
	}
	else
		if(numero < 100){
			string_append(&nro,"0");
			string_append(&nro,string_itoa(numero));
		}
		else
			string_append(&nro,string_itoa(numero));

	return nro;
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
for(;;){
	if (recibirHeader(socketOrigen) == enviarDatosPokenest){

	void *buffer = malloc(sizeof(int) + sizeof(int));
	recv(socketOrigen, buffer, sizeof(t_metadataPokenest), 0);

	int cursorMemoria = 0;
	memcpy(&pokenest.posicionX,buffer, sizeof(uint32_t));

	cursorMemoria += sizeof(uint32_t);
	memcpy(&pokenest.posicionY,buffer + cursorMemoria, sizeof(uint32_t));

	free(buffer);

	enviarHeader(socketOrigen, entrenadorListo);

	break;
	}
}
}
void solicitarUbicacionPokenest(int socketDestino,char pokemon){
	enviarHeader(socketDestino, datosPokenest);
	send(socketDestino,&pokemon,sizeof(char),0);
}

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
	string_append(&rutaDestino, "/medallas");

	char* comando = crearComando(rutaOrigen,rutaDestino);
	system(comando);
}

void solicitarAtraparPkm(char pokemon, int servidorMapa){
	enviarHeader(servidorMapa, capturarPokemon);
	void *buffer = malloc(sizeof(char));
	memcpy(buffer, &pokemon,sizeof(char));
	send(servidorMapa, buffer, sizeof(char), 0);
	free(buffer);
}
void solicitarMovimiento(int socketDestino, t_metadataPokenest pokenest){
	void* buffer = malloc(sizeof(int)+sizeof(int)+sizeof(int));
	int cursorMemoria = 0;
	int header = posicionEntrenador;

	moverEntrenador(pokenest);
	memcpy(buffer, &header, sizeof(int));
	cursorMemoria += sizeof(int);
	memcpy(buffer+cursorMemoria, &ubicacionEntrenador.coordenadasX, sizeof(int));
	cursorMemoria += sizeof(int);
	memcpy(buffer+cursorMemoria, &ubicacionEntrenador.coordenadasY, sizeof(int));
	send(socketDestino,buffer,sizeof(int)+sizeof(int)+sizeof(int),0);
	//hasta aca envio el header con las coordenadas del entrenador

	free(buffer);
}


void enviarMisDatos(int socketDestino){

	int tamanio = sizeof(int)+sizeof(int)+sizeof(char[18])+sizeof(char);
	void* buffer = malloc(tamanio);
	int cursor = 0;

	memcpy(buffer,&entrenador.nombre ,sizeof(char[18]));
	cursor += sizeof(char[18]);
	memcpy(buffer+cursor,&entrenador.simbolo ,sizeof(char));
	cursor += sizeof(char);
	memcpy(buffer+cursor,&ubicacionEntrenador.coordenadasX,sizeof(int));
	cursor += sizeof(int);
	memcpy(buffer+cursor,&ubicacionEntrenador.coordenadasY,sizeof(int));

	send(socketDestino,buffer,tamanio,0);
	free(buffer);

}

void reestablecerDatos(){
	ubicacionEntrenador.coordenadasX = 0;
	ubicacionEntrenador.coordenadasY = 0;
	ubicacionEntrenador.ultimoMov = 'y';
}

void enviarCantidadDeMovsAPokenest(t_metadataPokenest pokenest, int serverMapa){

	int* movs =malloc(sizeof(int));
	*movs = cantidadDeMovimientosAPokenest(pokenest);
	send(serverMapa, movs, sizeof(int), 0);
	free(movs);

}


void enviarPokemon(int servidor, char pokemon){
	send(servidor, &pokemon, sizeof(char),0);
}

void recibirNombrePkm(int socketServer, char nombrePkm[18]){

	void* buffer = malloc(18);

	if(!recibirTodo(socketServer,buffer, 18))
		memcpy(&nombrePkm,buffer,18);

	free(buffer);

}

void enviarPokemonMasFuerte(t_list* pokemonesAtrapados,int servidorMapa){

	int i;
	t_metadataPokemon pokemonMasFuerte;
	for(i=0;i<list_size(pokemonesAtrapados);i++){
		t_metadataPokemon* variable = malloc(sizeof(t_metadataPokemon));
		variable =list_get(pokemonesAtrapados,i);
		if(i == 0)
			pokemonMasFuerte = *variable;
		else
			if(variable->nivel > pokemonMasFuerte.nivel)
				pokemonMasFuerte = *variable;
		free(variable);
	}
	enviarHeader(servidorMapa, mejorPokemon);
	void* buffer = malloc(sizeof(t_metadataPokemon));
	memcpy(buffer,&pokemonMasFuerte,sizeof(t_metadataPokemon));
	send(servidorMapa,buffer,sizeof(t_metadataPokemon),0);
	free(buffer);
	log_info(logger,"Entrenador envía a pelear a su pokemon más fuerte, el cual es: %s con un nivel de: %d",
			pokemonMasFuerte.nombre, pokemonMasFuerte.nivel);

}

void removerMedallas(char* entrenador){
	char* comando = string_new();
	string_append(&comando,"rm mnt/pokedex/Entrenadores");
	string_append(&comando,entrenador);
	string_append(&comando,"/medallas/*");
	system(comando);
}

void removerPokemones(char* entrenador){
	char* comando = string_new();
	string_append(&comando,"rm mnt/pokedex/Entrenadores");
	string_append(&comando,entrenador);
	string_append(&comando,"Directorio' 'de' 'Bill/*");
	system(comando);
}
