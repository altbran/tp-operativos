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
#include <assert.h>
#include "deadlock.h"
#include "planificadores.h"
#include <semaphore.h>
#include <errno.h>

//variables
t_log* logger;
char *texto;
char * nombreMapa;
int servidorMapa;
fd_set bolsaDeSockets;
fd_set bolsaAuxiliar;
sem_t contadorEntrenadoresListos;
sem_t contadorEntrenadoresBloqueados;
pthread_mutex_t mutex;
pthread_mutex_t mutexDeadlock;
t_metadataMapa * configuracion;
char * ruta;
t_list * Pokenests;
t_list * Entrenadores;
t_queue * listos;
t_list * recursosTotales;
t_list * listaRecursosDisponibles;
pthread_t planificador;
pthread_t deadlock;
t_list * pokemones;


//funciones
void receptorSIG(int sig);
void cargarMetadata();
char* concat(int count, ...);
void cargarRecursos();
int contadorDePokemon(char * directorio);
t_metadataPokenest* devolverPokenest(char * identificador);
int devolverIndicePokenest(char identificador);
int enviarCoordPokenest(int socketDestino, t_metadataPokenest * pokenest);
int recibirEntrenador(int socketOrigen,t_datosEntrenador * entrenador);
t_datosEntrenador * devolverEntrenador(int socketOrigen);
int devolverIndiceEntrenador(int socket);
int movimientoValido(int socket,int posX, int posY);
int pokemonDisponible(int indicePokenest, char identificador,int * numeroPokemon, int * indice);
char** str_split(char* a_str, const char a_delim);
void restarRecursoDisponible(int indicePokenest);
void iniciarPlanificador();
void reasignarPokemonesDeEntrenadorADisponibles(int socketEntrenador);
void desconectadoOFinalizado(int socketEntrenador);


#endif /* FUNCIONES_H_ */
