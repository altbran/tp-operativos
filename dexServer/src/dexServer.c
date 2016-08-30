#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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

typedef struct{
	char estado;
	char nombre[17];
	short int bloquePadre;
	int tamanioArchivo;
	char fecha[4];
	int bloqueInicial;
}TablaArchivos;

typedef struct{
	Header encabezado;
	//bitarray
	TablaArchivos TA[2048];
	//tabla asignaciones
}estructuraAdministrativa;

int main(void) {

	Header encab;

	/*time_t tiempo = time(0);
	struct tm *tlocal = localtime(&tiempo);
	char fecha[5];
	strftime(fecha,5,"%d%m",tlocal);*/

	//con todo esto puedo sacar el dia y mes, necesario para el campo fecha de la tabla de archivos. Se va a usar mas tarde


	printf("%d\n",sizeof(encab));
	printf("%d\n",sizeof(TablaArchivos));
	return 0;

}


