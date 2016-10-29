#ifndef LIBRERIAS_STRUCTS_H_
#define LIBRERIAS_STRUCTS_H_
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
//#include <parser/metadata_program.h>
#include <stdint.h>

typedef struct {
	char nombre[18];
	uint32_t tiempoChequeoDeadlock;
	uint32_t batalla;
	char * algoritmo;
	uint32_t quantum;
	uint32_t retardo;
	char * ip;
	uint32_t puerto;
}t_metadataMapa;

typedef struct {
	char * identificador;
	int posicionX;
	int posicionY;
	int socket;
}t_datosEntrenador;

typedef struct {
	int socket;
	char * identificadorPokemon;
}t_entrenadorBloqueado;

typedef struct {
	char * identificador;
	int cantidad;
}t_recursosPokenest;

typedef struct{
	char * tipo;
	int posicionX;
	int posicionY;
	int cantidad;
	char identificador;
}t_metadataPokenest;

typedef struct{
	int nivel;
}t_metadataPokemon;

typedef struct{
	char* nombre;
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
