#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/bitarray.h>

#define BLOCK_SIZE 64

typedef struct{
	char identificador[7];
	char version;
	int tamanioFS;
	int tamanioBitmap;
	int inicioTablaAsignaciones;
	int tamanioDatos;
	char relleno[40];
}Header;

int main(void) {

	Header encab;

	printf("%d\n",sizeof(encab));
	return 0;

}
