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
	t_bitarray* punteroBitmap;
	osadaFile* tablaArchivos;
	int* tablaAsignaciones;
}t_estructuraAdministrativa;


//Prototipos

void setOsadaFile(FILE* archivo);
void grabarHeader(FILE* arch);
void grabarTablaOsadaFile(FILE* archivo);
void grabarTablaAsignaciones(FILE* archivo, t_header header);
void leerEstructurasAdministrativas(FILE* archivo,t_estructuraAdministrativa* estructuraAdministrativa);
void actualizarBitmap(FILE* archivo,t_bitarray* pBitMap);



int main(void) {

	//setOsadaFile(arch);     --->>> si necesito formatear otra cosa, solamente le paso el archivo

	t_estructuraAdministrativa estructuraAdministrativa;
	FILE* archivo;
	int i;

	/*time_t tiempo = time(0);
	struct tm *tlocal = localtime(&tiempo);
	char fecha[5];
	strftime(fecha,5,"%d%m",tlocal);*/

	//con esto puedo sacar el dia y mes, necesario para el campo fecha de la tabla de archivos. Se va a usar mas tarde


	archivo = fopen("ArchivoPrueba.osada","rb+");

	leerEstructurasAdministrativas(archivo,&estructuraAdministrativa);

	fclose(archivo);

	printf("%.7s\n",estructuraAdministrativa.header.identificador);
	printf("%c\n",estructuraAdministrativa.header.version);
	printf("tamanioFS: %d\n",estructuraAdministrativa.header.tamanioFS);
	printf("tamanioBitmap: %d\n",estructuraAdministrativa.header.tamanioBitmap);
	printf("inicioTablaAsignaciones: %d\n",estructuraAdministrativa.header.inicioTablaAsignaciones);
	printf("tamanioDatos: %d\n",estructuraAdministrativa.header.tamanioDatos);
	printf("relleno: %.40s\n",estructuraAdministrativa.header.relleno);


	for(i = 0;i<60;i++)
		printf("Valor bitarray[%d]: %c\n",i,estructuraAdministrativa.punteroBitmap->bitarray[i]);
	printf("Valor bitarray[%d]: %c\n",2047,estructuraAdministrativa.punteroBitmap->bitarray[2047]);


	for(i = 0;i<60;i++)
		printf("Estado osadaFile: %c\n",estructuraAdministrativa.tablaArchivos[i].estado);
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

	fwrite(dataBitmap,BLOCK_SIZE,header.tamanioBitmap,archivo);   //grabo el bitmap

	grabarTablaOsadaFile(archivo);   //grabo el array de osadaFiles

	grabarTablaAsignaciones(archivo,header);  //guardo los bytes correspondientes a la tabla de asignaciones

	rewind(archivo);

	free(dataBitmap);
}

void grabarTablaOsadaFile(FILE* archivo)
{
	osadaFile of;
	int i;

	of.estado = '0';
	strcpy(of.nombre,"Nombrearchivo");
	of.bloquePadre = 0;
	of.tamanioArchivo = 500;
	of.fecha[0] = '0';
	of.fecha[1] = '9';
	of.fecha[2] = '0';
	of.fecha[3] = '9';
	of.bloqueInicial = 0;

	for(i = 0;i<2048;i++)
		fwrite(&of,BLOCK_SIZE/2,1,archivo);
}

void grabarTablaAsignaciones(FILE* archivo, t_header header)
{
	int tamanioTabla;
	int arrayTablaAsignaciones[16] = {0};  //esto equivale a 1 bloque de 64B
	int auxiliarResto;
	int i;

	tamanioTabla = (header.tamanioFS - 1 - header.tamanioBitmap - 1024) * 4 / BLOCK_SIZE;  //el tamaño de la tablaAsignaciones
	auxiliarResto = ((header.tamanioFS - 1 - header.tamanioBitmap - 1024) * 4) % BLOCK_SIZE;

	if(auxiliarResto)
		tamanioTabla++;

	for(i = 0; i < tamanioTabla; i++)         //va a grabar tanto bloques del array, como tamaño tenga la tabla
		fwrite(arrayTablaAsignaciones,BLOCK_SIZE,1,archivo);

	//TODO - NOTA: el resto que calcule arriba, lo multiplico por 4, y me da la cantidad de int que ignoro del ultimo bloque

}

void grabarHeader(FILE* arch)     //crea el header del FS
{
	t_header header;
	int tamanioTotalArchivoEnBytes;
	float auxiliarResto;
	int tamanioTabla;

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
	header.tamanioBitmap = (header.tamanioFS / 8) / BLOCK_SIZE;
	auxiliarResto = (header.tamanioFS / BLOCK_SIZE) % 8;
	if(auxiliarResto)
		header.tamanioBitmap++;
		//si el resultado entero es menor que el resultado racional, entonces sumo 1 (no existen 5 bloques y medio)

	header.inicioTablaAsignaciones = 1 + header.tamanioBitmap + 1024;

	tamanioTabla = (header.tamanioFS - 1 - header.tamanioBitmap - 1024) * 4 / BLOCK_SIZE;  //el tamaño de la tablaAsignaciones
	auxiliarResto = ((header.tamanioFS - 1 - header.tamanioBitmap - 1024) * 4) % BLOCK_SIZE;

	if(auxiliarResto)      //si hay resto, necesito otro bloque mas
		tamanioTabla++;

	header.tamanioDatos = header.tamanioFS - 1 - header.tamanioBitmap - 1024 - tamanioTabla;

	fwrite(&header,BLOCK_SIZE,1,arch);
}
