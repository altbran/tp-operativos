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

int cantidadDeMovimientosAPokenest(t_metadataPokenest* pokenest){
	int counter = 0;
	counter = abs(pokenest->posicionX - ubicacionEntrenador.coordenadasX) + abs(pokenest->posicionY - ubicacionEntrenador.coordenadasY);
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

	//recorro todos los mapas en la hoja de viaje
	for(i=0; arrayHojaDeViaje[i]!= NULL; i++){

		t_objetivosPorMapa* objetivosPorMapa = malloc(sizeof(t_objetivosPorMapa));
		objetivosPorMapa->mapa = arrayHojaDeViaje[i];
		objetivosPorMapa->objetivos = list_create();

		char* metadataObjetivo = string_new();
		string_append_with_format(&metadataObjetivo, "obj[%s]", objetivosPorMapa->mapa);
		char** objetivos = config_get_array_value(metadata, metadataObjetivo);

		for(j=0; objetivos[j]!= NULL; j++){
			// dos veces seguidas el mismo pkm
			if(j>0)
				if(string_starts_with(objetivos[j], objetivos[j-1]))
					return NULL;

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

char* copiarArchivo(char* rutaOrigen,char* rutaDestino){
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
void recibirYAsignarCoordPokenest(int socketOrigen, t_metadataPokenest* pokenest, char nombrePkm[18]){

	if (recibirHeader(socketOrigen) == enviarDatosPokenest){

		void *buffer = malloc(sizeof(int) + sizeof(int)+sizeof(char[18]));

		if(!recibirTodo(socketOrigen,buffer,(sizeof(int) + sizeof(int)+sizeof(char[18])))){

		int cursorMemoria = 0;
		memcpy(&pokenest->posicionX,buffer, sizeof(int));

		cursorMemoria += sizeof(int);
		memcpy(&pokenest->posicionY,buffer + cursorMemoria, sizeof(int));

		cursorMemoria += sizeof(int);

		memcpy(nombrePkm,buffer + cursorMemoria, sizeof(char[18]));

		free(buffer);
		}

	}

}
void solicitarUbicacionPokenest(int socketDestino,char pokemonRecibido){
	enviarHeader(socketDestino, datosPokenest);
	send(socketDestino,&pokemonRecibido,sizeof(char),0);
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

	char* comando = copiarArchivo(rutaOrigen,rutaDestino);
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
	void* buffer = malloc(sizeof(int)+sizeof(int));
	int cursorMemoria = 0;
	enviarHeader(socketDestino, posicionEntrenador);

	moverEntrenador(pokenest);

	memcpy(buffer, &ubicacionEntrenador.coordenadasX, sizeof(int));
	cursorMemoria += sizeof(int);
	memcpy(buffer+cursorMemoria, &ubicacionEntrenador.coordenadasY, sizeof(int));
	cursorMemoria += sizeof(int);
	log_info(logger, "ubicacion  enviada: (%d,%d)", ubicacionEntrenador.coordenadasX, ubicacionEntrenador.coordenadasY);
	send(socketDestino,buffer,cursorMemoria,0);
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
	log_warning(logger,"CAdena pt: %s",buffer);
	free(buffer);

}

void reestablecerDatos(){
	ubicacionEntrenador.coordenadasX = 0;
	ubicacionEntrenador.coordenadasY = 0;
	ubicacionEntrenador.ultimoMov = 'y';
}

void enviarCantidadDeMovsAPokenest(t_metadataPokenest* pokenest, int serverMapa){

	int* movs =malloc(sizeof(int));
	*movs = cantidadDeMovimientosAPokenest(pokenest);
	send(serverMapa, movs, sizeof(int), 0);
	log_info(logger,"cantidad de movs a la pokenest %d", *movs);
	free(movs);

}


void enviarPokemon(int servidor, char pokemon){
	send(servidor, &pokemon, sizeof(char),0);
}

void recibirNombrePkm(int socketServer, char nombrePkm[18]){

	void* buffer = malloc(18);
	nombrePkm[18] = '\0';

	if(!recibirTodo(socketServer,buffer, 18))
		memcpy(&nombrePkm,buffer,18);

	free(buffer);

}

void enviarPokemonMasFuerte(t_list* pokemonesAtrapados,int servidorMapa){

	int i;
	t_pokemon pokemonMasFuerte;
	for(i=0;i<list_size(pokemonesAtrapados);i++){
		t_pokemon* variable = malloc(sizeof(t_pokemon));
		variable =list_get(pokemonesAtrapados,i);
		if(i == 0)
			pokemonMasFuerte = *variable;
		else
			if(variable->nivel > pokemonMasFuerte.nivel)
				pokemonMasFuerte = *variable;
		free(variable);
	}
	t_metadataPokemon* pkm = malloc(sizeof(t_metadataPokemon));
	pkm->nivel = pokemonMasFuerte.nivel;
	strcpy(pkm->nombre,pokemonMasFuerte.nombre);

	enviarHeader(servidorMapa, mejorPokemon);
	void* buffer = malloc(sizeof(t_metadataPokemon));
	memcpy(buffer,pkm,sizeof(t_metadataPokemon));
	send(servidorMapa,buffer,sizeof(t_metadataPokemon),0);
	free(buffer);
	free(pkm);

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

char* diferenciaDeTiempo(char* tiempoDeInicio, char* tiempoFinal){

	//paso a entero los diferentes tiempos

	int milisegundosI = atoi(string_substring(tiempoDeInicio, 9, 3));
	int segundosI = atoi(string_substring(tiempoDeInicio,6,2));
	int minutosI = atoi(string_substring(tiempoDeInicio,3,2));
	int horaI = atoi(string_substring(tiempoDeInicio,0,2));

	int milisegundosF = atoi(string_substring(tiempoFinal, 9, 3));
	int segundosF = atoi(string_substring(tiempoFinal,6,2));
	int minutosF = atoi(string_substring(tiempoFinal,3,2));
	int horaF = atoi(string_substring(tiempoFinal,0,2));

	//declaro los valores que adquirira el nuevo tiempo
	int milisegundosD;
	int segundosD;
	int minutosD;
	int horaD;

	//declaro las diferentes partes del nuevo tiempo
	char* mSegundosDif= string_new();
	char* segundosDif= string_new();
	char* minutosDif= string_new();
	char* horaDif= string_new();
	char* tiempoDeDiferencia = string_new();


	//calcular diferencia de tiempo

	milisegundosD = milisegundosF - milisegundosI;

	if(milisegundosD < 0){
		milisegundosD = (1000 + milisegundosF) - milisegundosI;
		segundosF--;
	}

	segundosD = segundosF - segundosI;

	if(segundosD < 0){
		segundosD = (60 + segundosF) - segundosI;
		minutosF--;
	}

	minutosD = minutosF - minutosI;

	if(minutosD < 0){
		minutosD = (60+minutosF) - minutosI;
		horaF--;
	}

	horaD = horaF - horaI;

	if(milisegundosD < 10){
		string_append(&mSegundosDif,"00");
		string_append(&mSegundosDif,string_itoa(milisegundosD));
	}
	else
		if(milisegundosD < 100){
		string_append(&mSegundosDif,"0");
		string_append(&mSegundosDif, string_itoa(milisegundosD));
		}
		else
			mSegundosDif = string_itoa(milisegundosD);

	if(segundosD < 10){
		string_append(&segundosDif,"0");
		string_append(&segundosDif, string_itoa(segundosD));
	}
	else
		segundosDif = string_itoa(segundosD);

	if(minutosD < 10){
			string_append(&minutosDif,"0");
			string_append(&minutosDif,string_itoa(minutosD));
		}
		else
			if(minutosD == 0)
			minutosDif = "00";
			else
				minutosDif = string_itoa(minutosD);

	if(horaD < 10){
		string_append(&horaDif, "0");
		string_append(&horaDif, string_itoa(horaD))	;
	}
	else
		horaDif = string_itoa(horaD);


	//concateno las diferentes partes del tiempo
	string_append(&tiempoDeDiferencia,horaDif);
	string_append(&tiempoDeDiferencia,":");
	string_append(&tiempoDeDiferencia,minutosDif);
	string_append(&tiempoDeDiferencia,":");
	string_append(&tiempoDeDiferencia,segundosDif);
	string_append(&tiempoDeDiferencia,":");
	string_append(&tiempoDeDiferencia,mSegundosDif);

	return tiempoDeDiferencia;

}

void sumarTiempos(char** tiempo, char* tiempoASumar){



	int milisegundos = atoi(string_substring(*tiempo,9, 3));
	int segundos = atoi(string_substring(*tiempo,6, 2));
	int minutos = atoi(string_substring(*tiempo,3, 2));
	int hora = atoi(string_substring(*tiempo,0,2));


	int horaAS = atoi(string_substring(tiempoASumar,0,2));
	int minutosAS = atoi(string_substring(tiempoASumar,3, 2));
	int segundosAS = atoi(string_substring(tiempoASumar, 6, 2));
	int milisegundosAS = atoi(string_substring(tiempoASumar, 9, 3));


	char* mSegundosDif= string_new();
	char* segundosDif= string_new();
	char* minutosDif= string_new();
	char* horaDif= string_new();


	*tiempo = string_new();



	milisegundos += milisegundosAS;

	if(milisegundos >= 1000){
		milisegundos = milisegundos - 1000;
		segundos++;
	}


	segundos += segundosAS;
	if(milisegundos >= 60){
			milisegundos = milisegundos - 60;
			minutos++;
	}

	minutos += minutosAS;
	if(minutos > 60){
		minutos = minutos - 60;
		hora++;
	}

	hora+= horaAS;
	if(hora >= 24){
		hora = hora - 24;
	}


	if(milisegundos < 10){
			string_append(&mSegundosDif,"00");
			string_append(&mSegundosDif,string_itoa(milisegundos));
		}
		else
			if(milisegundos < 100){
			string_append(&mSegundosDif,"0");
			string_append(&mSegundosDif, string_itoa(milisegundos));
			}
			else
				mSegundosDif = string_itoa(milisegundos);

		if(segundos < 10){
			string_append(&segundosDif,"0");
			string_append(&segundosDif, string_itoa(segundos));
		}
		else
			segundosDif = string_itoa(segundos);

		if(minutos < 10){
				string_append(&minutosDif,"0");
				string_append(&minutosDif,string_itoa(minutos));
			}
			else
				if(minutos == 0)
				minutosDif = "00";
				else
					minutosDif = string_itoa(minutos);

		if(hora < 10){
			string_append(&horaDif, "0");
			string_append(&horaDif, string_itoa(hora))	;
		}
		else
			horaDif = string_itoa(hora);


	string_append(&(*tiempo),horaDif);
	string_append(&(*tiempo),":");
	string_append(&(*tiempo),minutosDif);
	string_append(&(*tiempo),":");
	string_append(&(*tiempo),segundosDif);
	string_append(&(*tiempo),":");
	string_append(&(*tiempo),mSegundosDif);

}

void eliminarArchivosPokemones(t_list* lista, char* ruta){

	int i;
	for(i=0;i<list_size(lista);i++){
		t_pokemon* pers = malloc(sizeof(t_pokemon));
		pers = list_get(lista,i);
		char* rutta = string_new();
		char* nro = obtenerNumero(pers->numero);

		string_append(&rutta,ruta);
		string_append(&rutta, "/");
		string_append(&rutta,pers->nombre);
		string_append(&rutta,"/");
		string_append(&rutta,pers->nombre);
		string_append(&rutta,nro);
		string_append(&rutta,".jpg");

		char* comando = string_new();
		string_append(&comando, "rm ");
		string_append(&comando, rutta);

		system(comando);
		free(pers);
	}
}

bool filtrarMapa(t_pokemon* pokemon){

	return string_equals_ignore_case(pokemon->mapa, nombreMapa);

}

bool distintoMapa(t_pokemon* pokemon){

	return !string_equals_ignore_case(pokemon->mapa, nombreMapa);
}
