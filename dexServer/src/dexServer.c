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


//Prototipos

void setOsadaFile(FILE* archivo);
void grabarHeader(FILE* arch);
void actualizarBitmap(FILE* archivo,t_bitarray* pBitMap);



int main(void) {

	//t_estructuraAdministrativa estructuraAdministrativa;
	t_header header;
	t_bitarray* punteroBitmap;
	FILE* arch;
	char* data;

	int i;

	/*time_t tiempo = time(0);
	struct tm *tlocal = localtime(&tiempo);
	char fecha[5];
	strftime(fecha,5,"%d%m",tlocal);*/

	//con todo esto puedo sacar el dia y mes, necesario para el campo fecha de la tabla de archivos. Se va a usar mas tarde


	arch = fopen("ArchivoPrueba.osada","rb+");
	//setOsadaFile(arch);

	fread(&header,BLOCK_SIZE,1,arch);  //leo el cabezal, y lo guardo en la estructura
	data = malloc(header.tamanioBitmap * 64);
	fread(data,BLOCK_SIZE,header.tamanioBitmap,arch);
	punteroBitmap = bitarray_create(data,header.tamanioBitmap*64);    //lee el espacio del bitmap y lo almacena en un bitarray

	fclose(arch);

	printf("%.7s\n",header.identificador);
	printf("%c\n",header.version);
	printf("tamanioFS: %d\n",header.tamanioFS);
	printf("tamanioBitmap: %d\n",header.tamanioBitmap);
	printf("inicioTablaAsignaciones: %d\n",header.inicioTablaAsignaciones);
	printf("tamanioDatos: %d\n",header.tamanioDatos);
	printf("relleno: %.40s\n",header.relleno);

	for(i = 0;i<60;i++)
		printf("Valor bitarray[%d]: %c\n",i,punteroBitmap->bitarray[i]);
	printf("Valor bitarray[%d]: %c\n",2047,punteroBitmap->bitarray[2047]);




	bitarray_destroy(punteroBitmap);

	return 0;

}


void setOsadaFile(FILE* archivo)   //dado cualquier archivo, lo setea a formato OSADA
{
	t_header header;
	int i;
	char* dataBitmap;  //me reservo el espacio donde va a ir el bitArray

	grabarHeader(archivo);
	rewind(archivo);
	fread(&header,BLOCK_SIZE,1,archivo);   //almaceno los datos del header, van a ser utiles para las demás estructuras

	dataBitmap = malloc(header.tamanioBitmap * 64);  //el tamanioBitmap tiene de unidad [bloque], por ende, lo paso a bytes
	for(i = 0;i < (header.tamanioBitmap * 64);i++)
		dataBitmap[i] = '0';                //seteo el bitmap en '0', resulta mas practico

	fwrite(dataBitmap,BLOCK_SIZE,header.tamanioBitmap,archivo);
	rewind(archivo);

	free(dataBitmap);
}

void actualizarBitmap(FILE* archivo,t_bitarray* pBitMap)
{
	bool a;
	a = bitarray_test_bit(pBitMap, 0);
	printf("%d",a);

}

void grabarHeader(FILE* arch)     //crea el header del FS
{
	t_header header;
	int tamanioTotalArchivoEnBytes;
	int auxiliarBitmap = 0;

	fseek(arch,0,SEEK_END);
	tamanioTotalArchivoEnBytes = ftell(arch);
	fseek(arch,0,SEEK_SET);

	header.identificador[0] = 'O';
	header.identificador[1] = 's';
	header.identificador[2] = 'a';
	header.identificador[3] = 'd';
	header.identificador[4] = 'a';
	header.identificador[5] = 'F';
	header.identificador[6] = 'S';
	header.version = '1';
	header.tamanioFS = tamanioTotalArchivoEnBytes / BLOCK_SIZE;
		//aca saco cuentas del tamaño del bitmap
	auxiliarBitmap = (header.tamanioFS / 8) / BLOCK_SIZE;
	if(auxiliarBitmap < (header.tamanioFS / 8 / BLOCK_SIZE))
		auxiliarBitmap++;
		//si el resultado entero es menor que el resultado racional, entonces sumo 1 (no existen 5 bloques y medio)
	header.tamanioBitmap = auxiliarBitmap;
	header.inicioTablaAsignaciones = 1 + header.tamanioBitmap + 1024;
	header.tamanioDatos = header.tamanioFS - 1 - header.tamanioBitmap - 1024 - header.inicioTablaAsignaciones;

	fwrite(&header,BLOCK_SIZE,1,arch);
}
