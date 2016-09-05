#include <stdio.h>
#include <string.h>
#include <stdlib.h>			 //armar el bitmap
#include <stdbool.h>         //
#include <time.h>
#include <commons/string.h>
#include <commons/bitarray.h>


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

t_header leerHeader(FILE* archivo);
void setOsadaFile(FILE* archivo);
void grabarHeader(FILE* arch);
t_bitarray* crearBitmap(int cantBloquesBitmap);
void actualizarBitmap(FILE* archivo,t_bitarray* pBitMap);



int main(void) {

	//t_estructuraAdministrativa estructuraAdministrativa;
	t_header header;

	/*time_t tiempo = time(0);
	struct tm *tlocal = localtime(&tiempo);
	char fecha[5];
	strftime(fecha,5,"%d%m",tlocal);*/

	//con todo esto puedo sacar el dia y mes, necesario para el campo fecha de la tabla de archivos. Se va a usar mas tarde

	FILE* arch;
	arch = fopen("ArchivoPrueba.osada","rb+");
	header = leerHeader(arch);  //leo el cabezal, y lo guardo en la estructura

	fclose(arch);

	printf("%.7s\n",header.identificador);
	printf("%c\n",header.version);
	printf("tamanioFS: %d\n",header.tamanioFS);
	printf("tamanioBitmap: %d\n",header.tamanioBitmap);
	printf("inicioTablaAsignaciones: %d\n",header.inicioTablaAsignaciones);
	printf("tamanioDatos: %d\n",header.tamanioDatos);
	printf("relleno: %.40s\n",header.relleno);

	return 0;

}


void setOsadaFile(FILE* archivo)
{
	grabarHeader(archivo);
	//actualizarBitmap(FILE* archivo,t_bitarray* pBitMap);
}

t_bitarray* crearBitmap(int cantBloquesBitmap)       //crea y devuelve un puntero a un bitarray
{
	int i = 0;

	char* data = malloc(cantBloquesBitmap*64);   //me reservo el espacio para crear el bitarray
	for(i;i < (cantBloquesBitmap*64);i++)        //setea el bitarray en '0'
		data[i] = '0';
	t_bitarray* pBitMap;
	pBitMap = bitarray_create(data,cantBloquesBitmap*64);

	return pBitMap;
}

void actualizarBitmap(FILE* archivo,t_bitarray* pBitMap)
{

	char data[] = {0,0,0,0,0,0,0,0};
	pBitMap = bitarray_create(data,32);
}

t_header leerHeader(FILE* archivo)
{
	t_header aux;
	int i;

	for(i = 0;i < 7;i++)
		aux.identificador[i] = fgetc(archivo);
	aux.version = fgetc(archivo);
	fread(&aux.tamanioFS,4,1,archivo);   //en orden: la estructura donde guardo, el tamaño, la cantidad, el stream
	fread(&aux.tamanioBitmap,4,1,archivo);
	fread(&aux.inicioTablaAsignaciones,4,1,archivo);
	fread(&aux.tamanioDatos,4,1,archivo);
	fread(&aux.relleno,40,1,archivo);

	return aux;
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
