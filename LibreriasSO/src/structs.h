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
}t_metadataPokenest;

typedef struct{
	int nivel;
}t_metadataPokemon;

typedef struct{
	char* nombre;
	char simbolo;
	char hojaDeViaje[6]; //PuebloPaleta = A, CiudadVerde = B, CiudadPaleta = C
	char* objA;
	char* objB;
	char* objC;
	unsigned vidas;
	unsigned reintentos;

}t_metadataEntrenador;



#endif /* LIBRERIAS_STRUCTS_H_ */
