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
#include <time.h>

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

typedef struct{
	int nivel;
	char nombre[18];
	char* mapa;
	int numero;
}t_pokemon;


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
char* tiempoDeInicio;
char* tiempoFinal;
char* tiempoBloqueo;
char* nombreMapa;
int j;
int estado;
char* puntero;
char pkm;
t_metadataPokenest* pokenestProxima;
char* comando;
char* numeroPokemon;
char* rutaPokemon;
t_config* metadataMapa;
int murio;
char* rutaMetadataMapa;
t_list* pokemonesAtrapados;
t_objetivosPorMapa* elemento;
t_config* metadataPokemon;
char* comando;
char* rutaDirBill;
char resultado;
time_t solicitoAtraparPkm;
time_t atrapePkm;
double diferencia;
char* rutaMontaje;


//FUNCIONES
void senialRecibirVida();
void senialQuitarVida();
t_list* asignarHojaDeViajeYObjetivos(t_config*);
void moverEntrenador(t_metadataPokenest);
void moverEntrenadorcoordX(t_metadataPokenest);
void moverEntrenadorcoordY(t_metadataPokenest);
int llegoAPokenest(t_metadataPokenest);
void cargarDatos();
int cantidadDeMovimientosAPokenest(t_metadataPokenest* pokenest);
char* armarRutaPokemon(char* nombreMapa, char* nombrePokenest, char* nro);
char* crearRutaDirBill(char*);
char* copiarArchivo(char* rutaOrigen,char* rutaDestino);
void solicitarUbicacionPokenest(int socketDestino,char pokemonRecibido);
void recibirYAsignarCoordPokenest(int socketOrigen, t_metadataPokenest* pokenest, char nombrePkm[18]);
void solicitarAtraparPkm(char, int);
void solicitarMovimiento(int, t_metadataPokenest);
void desconectarseDe(int socketServer);
char* generarPathDelPokemon(char* nombreMapa, char* nombrePokenest);
char* obtenerNumero(int numero);
void enviarMisDatos(int socketDestino);
void reestablecerDatos();
void enviarCantidadDeMovsAPokenest(t_metadataPokenest* pokenest, int serverMapa);
char* obtenerNombre(char identificador);
void enviarPokemon(int servidor, char pokemon);
void removerMedallas(char* entrenador);
void copiarMedalla(char* nombreMapa);
void enviarPokemonMasFuerte(t_list* pokemonesAtrapados,int servidorMapa);
char* diferenciaDeTiempo(char* tiempoDeInicio, char* tiempoFinal);
void sumarTiempos(char** tiempo, char* tiempoASumar);
void eliminarArchivosPokemones(t_list* lista);
bool filtrarMapa(t_pokemon* pokemon);
bool distintoMapa(t_pokemon* pokemon);
char* sumaT(char* t1, char* t2);
char* rutaMedallas;

#endif /* FUNCIONESE_H_ */
