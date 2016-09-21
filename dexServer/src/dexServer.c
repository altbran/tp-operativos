#include <stdio.h>
#include <string.h>
#include <stdlib.h>			 //
#include <stdbool.h>         //
#include <time.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <src/sockets.h>
#include <pthread.h>
#include <commons/log.h>
#include <sys/mman.h>


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
void obtenerNombreArchivoDePath(char* path, char* nombre);
void* leerArchivo(char* pathSolicitado,t_estructuraAdministrativa est,char* mapa, t_log* logger);

int main(void) {

	int PUERTO_DEX_SERVIDOR = 5000; //getenv("PUERTO_DEX_SERVIDOR");
	//int IP_DEX_SERVIDOR = getenv("IP_DEX_SERVIDOR");
	FILE* archivo;

	t_estructuraAdministrativa estructuraAdministrativa;
	int i;
	int socketNuevo;
	int paquete;
	int listener;   //socket listener
	int fdmax;     //file descriptor maximo
	fd_set bolsaDeSockets;
	fd_set bolsaAuxiliar;
	char* archivoMapeado;

	t_log* logger = log_create("dexServer.log","DEXSERVER",0,log_level_from_string("INFO"));

	/*time_t tiempo = time(0);
	struct tm *tlocal = localtime(&tiempo);
	char fecha[5];
	strftime(fecha,5,"%d%m",tlocal);*/

	//con esto puedo sacar el dia y mes, necesario para el campo fecha de la tabla de archivos. Se va a usar mas tarde

	archivo = fopen("fileSystem.dat","rb+");
	fdmax = fileno(archivo);      //uso temporariamente el fdmax para almacenar el fd de mi FS

	leerEstructurasAdministrativas(archivo,&estructuraAdministrativa);

	fseek(archivo,0,SEEK_END);
	i = ftell(archivo);        //uso i temporariamente para sacar la longitud en bytes del archivo
	if (fdmax == -1)
		log_error(logger, "Error al obtener el 'file descriptor' del archivo");
	archivoMapeado = mmap(NULL, i, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE, fdmax, 0);
	if (archivoMapeado == MAP_FAILED)
		log_error(logger, "Error al mapear el File System en memoria");

	log_info(logger, "Se mapeo correctamente el archivo en memoria");

	//me guardo las est.adm en una struct porque va a ser mas comodo leerlas desde ahi
	//ademas, mapeo el FS


	if (crearSocket(&listener)) {
		log_error(logger, "Se produjo un error creando el socket servidor");
		return 1;
	}
	if (escucharEn(listener, PUERTO_DEX_SERVIDOR)) {
		printf("Error al conectar");
		log_error(logger, "Se produjo un error escuchando en el socket listener");
		return 1;
	}

	log_info(logger, "Se creó correctamente el socket servidor");
	log_info(logger, "Escuchando nuevas conexiones");



	struct sockaddr_in direccionCliente;

	socketNuevo = listener;     //listening socket descriptor

	FD_ZERO(&bolsaDeSockets);    //limpia los sets
	FD_ZERO(&bolsaAuxiliar);

	FD_SET(socketNuevo, &bolsaDeSockets);       //mete el socket en la bolsa

	fdmax = listener;

	while (1)
	{
		bolsaAuxiliar = bolsaDeSockets;
		if (select(fdmax + 1, &bolsaAuxiliar, NULL, NULL, NULL) == -1)
		{
			perror("select");
			return 1;
		}

		for (i = 0; i <= fdmax; i++)
		{
			if (FD_ISSET(i, &bolsaAuxiliar)) // we got one!!
			{
				if (i == listener)
				{
					socketNuevo = aceptarConexion(listener, &direccionCliente);

					if (socketNuevo > fdmax)
						fdmax = socketNuevo;

					int idRecibido = iniciarHandshake(socketNuevo, IPOKEDEXSERVER); //todo cambiar

					switch (idRecibido)
					{
						case IDERROR:
							log_info(logger, "Se desconecto el socket");
							close(socketNuevo);
							break;

						case IDPOKEDEXCLIENTE:  //todo cambiar esto
							FD_SET(socketNuevo, &bolsaDeSockets);

							log_info(logger, "Nuevo dexCliente conectado, socket %d", socketNuevo);
							break;

						default:
							close(socketNuevo);
							log_error(logger, "Error en el handshake. Conexion inesperada");
							break;
					}
				}
				else
				{  //aca mas adelante, voy a tener que crear hilos

					recibirTodo(i,&paquete,4);
					printf("%d\n",paquete);

					//int header = recibirHeader(i);

					/*switch(header)
					{
						case IDENTRENADOR:
							recibirTodo(i,&paquete,4);
							printf("%d\n",paquete);
					}*/
				}
			}
		} //for del select
	}//cierra el while





	fclose(archivo);
	free(estructuraAdministrativa.punteroBitmap->bitarray);
	bitarray_destroy(estructuraAdministrativa.punteroBitmap);
	free(estructuraAdministrativa.tablaAsignaciones);
	free(estructuraAdministrativa.tablaArchivos);

	log_destroy(logger);
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

void obtenerNombreArchivoDePath(char* path, char* nombre)
{
	int i = 0;
	int marcador;

	while(path[i] != '\0')
	{
		if(path[i] == '/')
			marcador = i;  //me dejo guardada la ubicacion de la ultima '/'
	}

	i = marcador;
	marcador = 0;
	while(path[i] != '\0')
	{
		nombre[marcador] = path[i];
		marcador++;
		i++;
	}
	nombre[marcador] = '\0';
}

void* leerArchivo(char* pathSolicitado,t_estructuraAdministrativa est,char* mapa, t_log* logger)
{
	char* directorio = malloc(18); //son 17 bytes maximos del nombre, mas el caracter '\0'
	int tamanioTotalEnBytes;
	int bloqueSiguienteEnTabla;
	int posicionDelMapa;
	int i = 0;
	int j;
	int contadorGlobal = 0;
	void* archivoLeido;

	obtenerNombreArchivoDePath(pathSolicitado,directorio);  //obtiene el nombre del path. Esto es, despues del ultimo '/'

	i = 0;
	while(strcmp(est.tablaArchivos[i].nombre,directorio) && (i < 2048))  //busco en la tabla de archivos su ubicacion
		i++;

	if(i == 2048)
		log_error(logger,"El archivo no existe");
	else
	{
		if(est.tablaArchivos[i].estado == '\2')              //si es un directorio, no debe ser leido
			log_error(logger,"No se puede leer un directorio");
		else
			log_info(logger,"Localizado el archivo dentro de la tabla");
	}

	tamanioTotalEnBytes = est.tablaArchivos[i].tamanioArchivo;
	bloqueSiguienteEnTabla = est.tablaArchivos[i].bloqueInicial;
	posicionDelMapa = (est.header.tamanioFS - est.header.tamanioDatos + bloqueSiguienteEnTabla) * 64;  //inicializo el cabezal en bytes
	free(directorio);      //reinicializo el string para poder guardarle el archivo adentro

	directorio = malloc(tamanioTotalEnBytes);  //ya se el tamaño que tiene, por eso le guardo el espacio en memoria

	while(contadorGlobal < tamanioTotalEnBytes)   //aca voy haciendo la lectura de a bloques
	{
		if((tamanioTotalEnBytes - contadorGlobal) >= 64 )  //si hay mas de un bloque, tengo que leer al menos un bloque entero
		{
			for(j = 0;j < 64;j++)
			{
				directorio[contadorGlobal] = mapa[posicionDelMapa];
				contadorGlobal++;
			}
		}
		else
		{
			for(j = 0;j < tamanioTotalEnBytes % BLOCK_SIZE;j++)			//me quedo con los bytes restantes
			{
				directorio[contadorGlobal] = mapa[posicionDelMapa];
				contadorGlobal++;
			}
		}

		bloqueSiguienteEnTabla = est.tablaAsignaciones[bloqueSiguienteEnTabla]; //recorro el array de asignaciones
		if(bloqueSiguienteEnTabla != 0xFFFFFFFF)
			posicionDelMapa = (est.header.tamanioFS - est.header.tamanioDatos + bloqueSiguienteEnTabla) * 64; //vuelvo a posicionar
	}

	archivoLeido = directorio;  //use el char* para obtener los datos, y se lo pase a un void* para que no tenga formato
	return archivoLeido;
}
