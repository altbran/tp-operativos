#ifndef FUNCIONES_H_
#define FUNCIONES_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include "commons/log.h"
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <curses.h>
#include <nivel.h>
#include <src/sockets.h>
#include <src/structs.h>
#include <src/protocolo.h>
#include "dibujador.h"
#include <signal.h>
#include <pthread.h>

//variables
t_log* logger;
char *texto;
int clientePokeDex;
int servidorMapa;
fd_set bolsaDeSockets;
fd_set bolsaAuxiliar;
pthread_mutex_t mutex;
t_metadataMapa configuracion;
char * ruta;
t_list * Pokenests;
t_list * Entrenadores;
t_queue * listos;
t_queue * bloqueados;
t_list * recursosDisponibles; //todo


//funciones
void receptorSIG();
void cargarMetadata();
char* concat(int count, ...);
void cargarRecursos();
int contadorDePokemon(char * directorio);
t_metadataPokenest devolverPokenest(char identificador);
void enviarCoordPokenest(int socketDestino, t_metadataPokenest pokenest);
t_datosEntrenador recibirEntrenador(int socketOrigen);
t_datosEntrenador devolverEntrenador(int socketOrigen,int * posicionEnLista);
int movimientoValido(int socket,int posX, int posY);
int pokemonDisponible(char * identificador);

#endif /* FUNCIONES_H_ */
