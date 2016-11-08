/*
 * funcionesE.h
 *
 *  Created on: 14/9/2016
 *      Author: utnso
 */

#ifndef FUNCIONESE_H_
#define FUNCIONESE_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "commons/log.h"
//#include <curses.h>
//#include <nivel.h>
#include <src/sockets.h>
#include <src/structs.h>
#include <src/protocolo.h>
#include <signal.h>
#include <pthread.h>
#include "commons/config.h"
#include "commons/string.h"
#include "commons/temporal.h"
#include <string.h>
#include "commons/collections/list.h"
#include "commons/temporal.h"

//STRUCTS
typedef struct{
	char* mapa;
	t_list* objetivos;
}t_objetivosPorMapa;

typedef struct{
	int coordenadasX;
	int coordenadasY;
	char ultimoMov;
}t_ubicacion;


//VARIABLES
uint32_t PUERTO_MAPA_SERVIDOR;
char* IP_MAPA_SERVIDOR;
char* mensaje;
char* nombre;
t_log* logger;
t_log* loger;
int servidorMapa;
t_metadataEntrenador entrenador;
t_ubicacion ubicacionEntrenador;
int cantidadDeadlocks;
int volverAEmpezar;
int muertes;

//FUNCIONES
void senialRecibirVida();
void senialQuitarVida();
t_list* asignarHojaDeViajeYObjetivos(t_config*);
void moverEntrenador(t_metadataPokenest);
void moverEntrenadorcoordX(t_metadataPokenest);
void moverEntrenadorcoordY(t_metadataPokenest);
int cantidadDeMovimientosAPokenest(t_metadataPokenest);
int llegoAPokenest(t_metadataPokenest);
void cargarDatos();
int cantidadDeMovimientosAPokenest(t_metadataPokenest);
char* armarRutaPokemon(char* nombreMapa, char* nombrePokenest, char* nro);
char* crearRutaDirBill(char*);
char* crearComando(char* ,char* );
void solicitarUbicacionPokenest(int,char);
void recibirYAsignarCoordPokenest(int,t_metadataPokenest);
void solicitarAtraparPkm(char, int);
void solicitarMovimiento(int, t_metadataPokenest);
void desconectarseDe(int socketServer);
char* generarPathDelPokemon(char* nombreMapa, char* nombrePokenest);
char* obtenerNumero(int numero);
void enviarMisDatos(int socketDestino);
void reestablecerDatos();
void enviarCantidadDeMovsAPokenest(t_metadataPokenest pokenest, int serverMapa);
char* obtenerNombre(char identificador);
void enviarPokemon(int servidor, char pokemon);
void recibirNombrePkm(int socketServer, char nombrePkm[18]);
void removerMedallas(char* entrenador);
void removerPokemones(char* entrenador);
void copiarMedalla(char* nombreMapa);
void enviarPokemonMasFuerte(t_list* pokemonesAtrapados,int servidorMapa);
void hastaQueNoReciba(int header, int socketOrigen);

#endif /* FUNCIONESE_H_ */
