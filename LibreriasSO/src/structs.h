#ifndef LIBRERIAS_STRUCTS_H_
#define LIBRERIAS_STRUCTS_H_
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
//#include <parser/metadata_program.h>
#include <stdint.h>

typedef struct {
	char nombre[18];
	uint32_t tiempoChequeoDeadlock;
	char batalla;
	char algoritmo[10];
	uint8_t quantum;
	uint32_t retardo;
	char ip[16];
	uint32_t puerto;
}t_metadataMapa;

typedef struct{
	char tipo[12];
	int posicionX;
	int posicionY;
	char identificador;
	int cantidad;
}t_metadataPokenest;

typedef struct{
	int nivel;
}t_metadataPokemon;

typedef struct{
	char* nombre;
	char simbolo;
	t_list hojaDeViaje;
	int vidas;
	int reintentos;
}t_metadataEntrenador;

typedef struct{
	char* mapa;
	t_list objetivos;
}t_objetivosPorMapa;


#endif /* LIBRERIAS_STRUCTS_H_ */
