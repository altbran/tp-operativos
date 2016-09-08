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

typedef struct{
	char * tipo;
	int posicionX;
	int posicionY;
	int cantidad;
	char * identificador;
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
	uint32_t tamaño;

}t_privilegiosArchivo;

#endif /* LIBRERIAS_STRUCTS_H_ */
