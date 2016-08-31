#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
}t_header;

typedef struct{
	char estado;
	char nombre[17];
	short int bloquePadre;
	int tamanioArchivo;
	char fecha[4];
	int bloqueInicial;
}osadaFile;

typedef struct{
	t_header header;
	t_bitarray bitmap;
	osadaFile tablaArchivos[2048];
	//tabla asignaciones
}t_estructuraAdministrativa;

int main(void) {

	//t_estructuraAdministrativa estructuraAdministrativa;
	t_header header;
	int i = 0;

	/*time_t tiempo = time(0);
	struct tm *tlocal = localtime(&tiempo);
	char fecha[5];
	strftime(fecha,5,"%d%m",tlocal);*/

	//con todo esto puedo sacar el dia y mes, necesario para el campo fecha de la tabla de archivos. Se va a usar mas tarde

	FILE* arch;
	arch = fopen("ArchivoPrueba.osada","rb+");
	fread(&header,64,1,arch);  					//en orden: la estructura donde guardo, el tamaño, la cantidad, el stream
	fseek(arch,0,SEEK_END);
	i = ftell(arch);
	fclose(arch);


	printf("%d bytes\n",i);
	return 0;

}
