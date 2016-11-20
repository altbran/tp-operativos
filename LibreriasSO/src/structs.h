#ifndef LIBRERIAS_STRUCTS_H_
#define LIBRERIAS_STRUCTS_H_
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
//#include <parser/metadata_program.h>
#include <stdint.h>

typedef struct {
	uint32_t tiempoChequeoDeadlock;
	uint32_t batalla;
	char algoritmo;
	uint32_t quantum;
	uint32_t retardo;
	char ip[15];
	uint32_t puerto;
}t_metadataMapa;

typedef struct {
	int socketEntrenador;
	int numeroPokemon;
	char identificadorPokemon;
}t_duenioPokemon;

typedef struct {
	char identificador;
	int posicionX;
	int posicionY;
	int socket;
	char nombre[18];
	int distanciaAPokenest;
}t_datosEntrenador;

typedef struct {
	int socket;
	char identificadorPokemon;
}t_entrenadorBloqueado;

typedef struct {
	char identificador;
	int cantidad;
}t_recursosPokenest;

typedef struct{
	char tipo[10];
	int posicionX;
	int posicionY;
	int cantidad;
	char identificador;
	char nombre[18];
	t_queue * colaPokenest;
	sem_t * semaforoPokenest;
	sem_t * disponiblesPokenest;
}t_metadataPokenest;

typedef struct{
	int nivel;
	char nombre[18];
}t_metadataPokemon;

typedef struct{
	char nombre[18];
	char simbolo;
	t_list* hojaDeViaje;
	int vidas;
	int reintentos;
}t_metadataEntrenador;

typedef struct {
	int esDir;/*1 si es directorio, 0 si es archivo regular(final)*/
	uint32_t tamanio;
}t_privilegiosArchivo;

typedef struct {
	const char *pathAntiguo;
	const char *pathNuevo;
}t_cambioDeDirectorios;

#endif /* LIBRERIAS_STRUCTS_H_ */
