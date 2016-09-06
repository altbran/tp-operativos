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
}t_mapa;





#endif /* LIBRERIAS_STRUCTS_H_ */