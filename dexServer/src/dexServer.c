#include <stdio.h>
#include <stdlib.h>			 //armar el bitmap
#include <stdbool.h>         //
#include <time.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <src/sockets.h>


#define BLOCK_SIZE 64
#define FILENAME_LENGTH 17

//Estructuras

typedef struct{
	unsigned char identificador[7];
	uint8_t version;
	uint32_t tamanioFS;
	uint32_t tamanioBitmap;
	uint32_t inicioTablaAsignaciones;
	uint32_t tamanioDatos;
	unsigned char relleno[40];
}t_header;

typedef enum __attribute__((packed)) {
    DELETED = '\0',
    REGULAR = '\1',
    DIRECTORY = '\2',
} osada_file_state;

typedef struct{
	osada_file_state estado;
	unsigned char nombre[17];
	uint16_t bloquePadre;
	uint32_t tamanioArchivo;
	uint32_t fecha;
	int bloqueInicial;
}osadaFile;

typedef struct{
	t_header header;
	t_bitarray* punteroBitmap;
	osadaFile* tablaArchivos;
	int* tablaAsignaciones;
}t_estructuraAdministrativa;


//Prototipos

void leerEstructurasAdministrativas(FILE* archivo,t_estructuraAdministrativa* estructuraAdministrativa);
void actualizarBitmap(FILE* archivo,t_bitarray* pBitMap);



int main(void) {

	t_estructuraAdministrativa estructuraAdministrativa;
	FILE* archivo;
	int i;

	/*time_t tiempo = time(0);
	struct tm *tlocal = localtime(&tiempo);
	char fecha[5];
	strftime(fecha,5,"%d%m",tlocal);*/

	//con esto puedo sacar el dia y mes, necesario para el campo fecha de la tabla de archivos. Se va a usar mas tarde


	archivo = fopen("fileSystem.dat","rb+");

	leerEstructurasAdministrativas(archivo,&estructuraAdministrativa);

	fclose(archivo);

	printf("%.7s\n",estructuraAdministrativa.header.identificador);
	printf("%d\n",estructuraAdministrativa.header.version);
	printf("tamanioFS: %d\n",estructuraAdministrativa.header.tamanioFS);
	printf("tamanioBitmap: %d\n",estructuraAdministrativa.header.tamanioBitmap);
	printf("inicioTablaAsignaciones: %d\n",estructuraAdministrativa.header.inicioTablaAsignaciones);
	printf("tamanioDatos: %d\n",estructuraAdministrativa.header.tamanioDatos);
	printf("relleno: %.40s\n",estructuraAdministrativa.header.relleno);


	for(i = 0;i<60;i++)
		printf("Valor bitarray[%d]: %d\n",i,bitarray_test_bit(estructuraAdministrativa.punteroBitmap,i));
	printf("Valor bitarray[%d]: %d\n",2047,bitarray_test_bit(estructuraAdministrativa.punteroBitmap,2047));


	for(i = 0;i<60;i++)
		printf("Estado osadaFile: %d\n",estructuraAdministrativa.tablaArchivos[i].estado);
	for(i = 0;i<60;i++)
		printf("Valor tablaAsignaciones: %d\n",estructuraAdministrativa.tablaAsignaciones[i]);



	free(estructuraAdministrativa.punteroBitmap->bitarray);
	bitarray_destroy(estructuraAdministrativa.punteroBitmap);
	free(estructuraAdministrativa.tablaAsignaciones);
	free(estructuraAdministrativa.tablaArchivos);

	return 0;
}


void leerEstructurasAdministrativas(FILE* archivo,t_estructuraAdministrativa* estructuraAdministrativa)
{
	t_header header;
	t_bitarray* punteroBitmap;
	char* data;
	osadaFile *tablaArchivos = malloc(sizeof(osadaFile) * 2048);
	int* tablaAsignaciones;
	int tamanioTablaAsignaciones;


	fread(&header,BLOCK_SIZE,1,archivo);  //leo el cabezal, y lo guardo en la estructura

	data = malloc(header.tamanioBitmap * 64);
	fread(data,BLOCK_SIZE,header.tamanioBitmap,archivo);
	punteroBitmap = bitarray_create(data,header.tamanioBitmap*64);    //lee el espacio del bitmap y lo almacena en un bitarray

	fread(tablaArchivos,sizeof(osadaFile) * 2048,1,archivo);

	tamanioTablaAsignaciones = (header.tamanioFS - 1 - header.tamanioBitmap - 1024) * 4 / BLOCK_SIZE;  //en bloques
	tablaAsignaciones = malloc(sizeof(tamanioTablaAsignaciones) * 64 * 4 );  //bloques * BLOCK_SIZE * 4(int)

	fread(tablaAsignaciones,tamanioTablaAsignaciones,1,archivo);


	estructuraAdministrativa->header = header;
	estructuraAdministrativa->punteroBitmap = punteroBitmap;
	estructuraAdministrativa->tablaArchivos = tablaArchivos;
	estructuraAdministrativa->tablaAsignaciones = tablaAsignaciones;
}

void actualizarBitmap(FILE* archivo,t_bitarray* pBitMap)
{
	bool a;
	a = bitarray_test_bit(pBitMap, 0);
	printf("%d",a);

}
