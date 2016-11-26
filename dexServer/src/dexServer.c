#include <stdio.h>
#include <string.h>
#include <stdlib.h>			 //fixme
#include <stdbool.h>         //
#include <time.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <src/sockets.h>
#include <src/protocolo.h>
#include <commons/log.h>
#include <pthread.h>
#include <sys/mman.h>


#define BLOCK_SIZE 64


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

typedef struct{
	int elSocket;
	char* elMapa;
}parametrosHilo;

//Prototipos

void leerEstructurasAdministrativas(FILE* archivo);
int pedirFecha(void);
void sacarNombre(char* path,char* nombre);
bool comprobarPathValido(char* path);
void recorrerDesdeIzquierda(char* path, char* nombre);
void guardarEstructuraEn(char* mapa);
void atenderConexion(void* arg);
int getAtr(char* path,char* mapa,int* tamanio);
void leerDirectorio(char* path, int socket);
void* leerArchivo(char* path,char* mapa, int* tamArchivo);
int crearDirectorio(char* path,char* mapa);
int crearArchivo(char* path,char* mapa);
int borrarArchivo(char* path,char* mapa);
int borrarDirectorioVacio(char* path,char* mapa);
int renombrar(char* pathOriginal, char* pathNuevo, char* mapa);
int escribirArchivo(char* path, char* fichero, int off, int tam, char* mapa, int socket);
int aperturaArchivo(char* path,int socket);
int cerradoArchivo(char* path,int socket);
int truncar(char* path,int tamanio,char* mapa,int socket);
void grabarNombreEn(int i, char* nombre);
void recuperarNombre(int i, char* nombre);


//Variables globales

t_log* logger;
t_estructuraAdministrativa estructuraAdministrativa;
int aperturado[2048] = {0};
pthread_mutex_t mutex;

int main(void) {

	logger = log_create("dexServer.log","DEXSERVER",0,log_level_from_string("INFO"));  //creo el archivo de log

	int PUERTO_POKEDEX_SERVIDOR = 9000;//atoi(getenv("PUERTO_POKEDEX_SERVIDOR"));
	char* IP_POKEDEX_SERVIDOR = "127.0.0.1";//getenv("IP_POKEDEX_SERVIDOR");
	FILE* archivo;
	pthread_attr_t attr;
	pthread_t thread;

	int i;
	int tamanioTotalFS;
	int socketNuevo;
	int listener;   //socket listener
	int fdmax;     //file descriptor maximo
	fd_set bolsaDeSockets;
	fd_set bolsaAuxiliar;
	parametrosHilo *parametro = malloc(sizeof(parametrosHilo));

	log_info(logger, "PUERTO_POKEDEX_SERVIDOR = %d",PUERTO_POKEDEX_SERVIDOR);
	log_info(logger, "IP_POKEDEX_SERVIDOR = %s",IP_POKEDEX_SERVIDOR);


	archivo = fopen("fileSystem.dat","rb+");
	fdmax = fileno(archivo);      //uso temporariamente el fdmax para almacenar el fd de mi FS

	leerEstructurasAdministrativas(archivo);   //me guardo las est.adm en una struct porque va a ser mas comodo leerlas desde ahi

	fseek(archivo,0,SEEK_END);
	tamanioTotalFS = ftell(archivo);        //uso i temporariamente para sacar la longitud en bytes del archivo
	parametro->elMapa = malloc(tamanioTotalFS); //le reservo i-bytes al archivo mapeado, es decir, 1M
	if (fdmax == -1){
		log_error(logger, "Error al obtener el 'file descriptor' del archivo");
		return 1;
	}
	parametro->elMapa = mmap(NULL, tamanioTotalFS, PROT_READ | PROT_WRITE, MAP_SHARED, fdmax, 0);
	if (parametro->elMapa == MAP_FAILED)
		log_error(logger, "Error al mapear el File System en memoria");

	log_info(logger, "Se mapeo correctamente el archivo en memoria");


	if (crearSocket(&listener)) {
		log_error(logger, "Se produjo un error creando el socket servidor");
		return 1;
	}
	if (escucharEn(listener, PUERTO_POKEDEX_SERVIDOR)) {
		printf("Error al conectar");
		log_error(logger, "Se produjo un error escuchando en el socket listener");
		return 1;
	}

	log_info(logger, "Se creo correctamente el socket servidor. Escuchando nuevas conexiones");


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
			log_error(logger,"Estructura del select tuvo un problema");
			return -1;
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

					int idRecibido = iniciarHandshake(socketNuevo, IDPOKEDEXSERVER);

					switch (idRecibido)
					{
						case IDERROR:
							log_info(logger, "Se desconecto el socket");
							close(socketNuevo);
							break;

						case IDPOKEDEXCLIENTE:
							FD_SET(socketNuevo,&bolsaDeSockets);

							log_info(logger, "Nuevo dexCliente conectado, socket= %d", socketNuevo);

							pthread_attr_init(&attr);
							pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

							parametro->elSocket = socketNuevo;		//le guardo el socket

							pthread_create(&thread,&attr,(void*)atenderConexion,(void*)parametro);

							pthread_attr_destroy(&attr);

							break;

						default:
							close(socketNuevo);
							log_error(logger, "Error en el handshake. Conexion inesperada");
							break;
					}
				} //-->if del listener
				else
				{
					//log_info(logger,"Turno del socket: %d",i);
					//atenderConexion(i, archivoMapeado);
					;
				}
			}
		} //for del select
	}//cierra el while





	fclose(archivo);
	free(estructuraAdministrativa.punteroBitmap->bitarray);
	bitarray_destroy(estructuraAdministrativa.punteroBitmap);
	free(estructuraAdministrativa.tablaAsignaciones);
	free(estructuraAdministrativa.tablaArchivos);
	free(parametro->elMapa);
	free(parametro);

	return 0;
}


void atenderConexion(void* arg)
{
	int operacion;
	int resultado;
	int tamanio;
	int socket;

	parametrosHilo* parametro = (parametrosHilo*) arg;
	socket = parametro->elSocket;

	char* path;
	void* archivo = NULL;

	operacion = recibirHeader(socket);

	while(operacion != socketDesconectado)
	{
		log_info(logger,"socket: %d, operacion: %d",socket,operacion);

		path = malloc(50);

		switch(operacion)
		{
			case abrirArchivo:
				pthread_mutex_lock(&mutex);
				recibirTodo(socket,path,50);

				resultado = aperturaArchivo(path,socket);
				enviarHeader(socket,resultado);

				pthread_mutex_unlock(&mutex);
				break;

			case contenidoArchivo:
				pthread_mutex_lock(&mutex);
				recibirTodo(socket,path,50);
				archivo = leerArchivo(path,parametro->elMapa,&tamanio);

				enviarHeader(socket,tamanio);
				send(socket,archivo,tamanio,0);

				pthread_mutex_unlock(&mutex);

				log_info(logger,"Se envio el archivo %s",path);
				free(archivo);
				break;

			case privilegiosArchivo:
				pthread_mutex_lock(&mutex);
				recibirTodo(socket,path,50);

				resultado = getAtr(path,parametro->elMapa,&tamanio);
				log_info(logger,"Para el PATH: '%s' se esta mandando DIR: %d  TAMANIO: %d",path,resultado,tamanio);

				enviarHeader(socket,resultado);
				enviarHeader(socket,tamanio);

				pthread_mutex_unlock(&mutex);

				break;

			case contenidoDirectorio:
				pthread_mutex_lock(&mutex);
				recibirTodo(socket,path,50);
				log_info(logger,"path recibido readdir: %s",path);
				enviarHeader(socket,comprobarPathValido(path));
				leerDirectorio(path,socket);

				pthread_mutex_unlock(&mutex);
				break;

			case crearFichero:
				pthread_mutex_lock(&mutex);
				recibirTodo(socket,path,50);

				resultado = crearArchivo(path,parametro->elMapa);
				enviarHeader(socket,resultado);
				log_info(logger,"Resultado de la creacion de '%s': %d",path,resultado);
				pthread_mutex_unlock(&mutex);
				break;

			case escribirEnFichero:
				pthread_mutex_lock(&mutex);
				recibirTodo(socket,path,50);
				resultado = recibirHeader(socket);    //uso 'resultado', para recibir el offset
				tamanio = recibirHeader(socket);

				void* ficheroEnviado = malloc(tamanio);
				recv(socket,ficheroEnviado,tamanio,0);

				resultado = escribirArchivo(path,ficheroEnviado,resultado,tamanio,parametro->elMapa,socket);
				enviarHeader(socket,resultado);
				pthread_mutex_unlock(&mutex);

				log_info(logger,"Resultado de la escritura de '%s': %d",path,resultado);
				free(ficheroEnviado);
				break;

			case eliminarArchivo:
				pthread_mutex_lock(&mutex);
				recibirTodo(socket,path,50);

				resultado = borrarArchivo(path,parametro->elMapa);
				enviarHeader(socket,resultado);
				pthread_mutex_unlock(&mutex);
				log_info(logger,"Resultado del borrado de '%s': %d",path,resultado);
				break;

			case cerrarArchivo:
				pthread_mutex_lock(&mutex);
				recibirTodo(socket,path,50);

				resultado = cerradoArchivo(path,socket);
				enviarHeader(socket,resultado);
				pthread_mutex_unlock(&mutex);
				break;

			case crearCarpeta:
				pthread_mutex_lock(&mutex);
				recibirTodo(socket,path,50);

				resultado = crearDirectorio(path,parametro->elMapa);
				enviarHeader(socket,resultado);
				pthread_mutex_unlock(&mutex);
				log_info(logger,"Resultado del mkdir del path %s: %d",path,resultado);
				break;

			case renombrarCosas:
				pthread_mutex_lock(&mutex);
				recibirTodo(socket,path,50);
				archivo = malloc(50);
				recibirTodo(socket,archivo,50);   //uso archivo, como el 'nuevo path'

				resultado = renombrar(path,(char*)archivo,parametro->elMapa);
				enviarHeader(socket,resultado);
				pthread_mutex_unlock(&mutex);
				free(archivo);
				break;

			case removerDirectorio:
				pthread_mutex_lock(&mutex);
				recibirTodo(socket,path,50);

				resultado = borrarDirectorioVacio(path,parametro->elMapa);
				enviarHeader(socket,resultado);
				pthread_mutex_unlock(&mutex);
				break;

			case truncarArchivo:
				pthread_mutex_lock(&mutex);
				recibirTodo(socket,path,50);
				tamanio = recibirHeader(socket);

				resultado = truncar(path,tamanio,parametro->elMapa,socket);
				enviarHeader(socket,resultado);
				pthread_mutex_unlock(&mutex);
				break;

			default:
				log_info(logger,"Todavia no implementado. Codigo operacion: %d",operacion);
				break;
		}
		free(path);
		operacion = recibirHeader(socket);
	}
}

void leerEstructurasAdministrativas(FILE* archivo)
{
	t_header header;
	t_bitarray* punteroBitmap;
	char* data;
	osadaFile *tablaArchivos = malloc(sizeof(osadaFile) * 2048);
	int* tablaAsignaciones;
	int tamanioTablaAsignaciones = 0;


	fread(&header,BLOCK_SIZE,1,archivo);  //leo el cabezal, y lo guardo en la estructura

	data = malloc(header.tamanioBitmap * 64);
	fread(data,BLOCK_SIZE,header.tamanioBitmap,archivo);
	punteroBitmap = bitarray_create_with_mode(data,header.tamanioBitmap*64,MSB_FIRST);   //lee el espacio del bitmap y lo almacena en un bitarray

	fread(tablaArchivos,sizeof(osadaFile) * 2048,1,archivo);

	tamanioTablaAsignaciones = (header.tamanioFS - header.tamanioDatos -1 - header.tamanioBitmap - 1024) * BLOCK_SIZE;  //da 958 * 64 = 61312
	tablaAsignaciones = malloc(tamanioTablaAsignaciones);

	fread(tablaAsignaciones,tamanioTablaAsignaciones,1,archivo);


	estructuraAdministrativa.header = header;
	estructuraAdministrativa.punteroBitmap = punteroBitmap;
	estructuraAdministrativa.tablaArchivos = tablaArchivos;
	estructuraAdministrativa.tablaAsignaciones = tablaAsignaciones;
	rewind(archivo);
}

int getAtr(char* path,char* mapa,int* tamanio)
{
	char* nombreArchivo;
	char* copiaPath = malloc(50);
	char* nombreRecuperado;
	int i = 0;
	int offset = 0xFFFF;

	strcpy(copiaPath,path);

	if(comprobarPathValido(copiaPath))
	{
		if(!strcmp(copiaPath,"/"))
		{
			*tamanio = 0;
			free(copiaPath);
			return 1;
		}
		else
		{
			while(strlen(copiaPath))  //esto saca el offset del archivo, en la tabla
			{
				nombreArchivo = malloc(18);
				recorrerDesdeIzquierda(copiaPath,nombreArchivo);
				i = 0;
				while(i < 2048)
				{
					nombreRecuperado = malloc(18);
					recuperarNombre(i,nombreRecuperado);

					if(!strcmp(nombreRecuperado,nombreArchivo))
						if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
							break;
					free(nombreRecuperado);
					i++;
				}


				if(i == 2048) //si llego al final es porque no encontró nada
				{
					log_error(logger,"GETATTR --- No se ha encontrado la entrada especificada. Path: '%s'",nombreArchivo);
					*tamanio = 0;
					free(nombreArchivo);
					free(copiaPath);
					return -1;
				}
				else
					offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

				free(nombreArchivo);
			}

			*tamanio = estructuraAdministrativa.tablaArchivos[offset].tamanioArchivo;

			log_info(logger,"Lo encontrado. Nombre: %s   Estado: %u",nombreRecuperado,estructuraAdministrativa.tablaArchivos[offset].estado);

			free(nombreRecuperado);
			free(copiaPath);
			if(estructuraAdministrativa.tablaArchivos[offset].estado == '\1')
				return 0;
			else
				return 1;
		}
	}
	*tamanio = 0;
	free(copiaPath);
	return -1;
}

int truncar(char* path,int tamanio,char* mapa,int socket)
{
	int i = 0;
	int offset = 0xFFFF;
	char* copiaPath = malloc(50);
	char* nombreArchivo;
	char* archivo;
	char* archivoTruncado;
	char* nombreRecuperado;

	strcpy(copiaPath,path);

	if(comprobarPathValido(copiaPath))  //si el path es correcto
	{
		while(strlen(copiaPath))  //esto saca el offset del archivo, en la tabla
		{
			nombreArchivo = malloc(18);
			recorrerDesdeIzquierda(copiaPath,nombreArchivo);
			i = 0;
			while(i < 2048)
			{
				nombreRecuperado = malloc(18);
				recuperarNombre(i,nombreRecuperado);

				if(!strcmp(nombreRecuperado,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
					{
						free(nombreRecuperado);
						break;
					}
				free(nombreRecuperado);
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				free(copiaPath);
				return -1;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(nombreArchivo);
		}					//aca tengo el offset del bloque padre, en offset

		if(estructuraAdministrativa.tablaArchivos[offset].estado != '\1')
		{
			log_error(logger,"No se puede truncar, no es un archivo");
			free(copiaPath);
			return -1;
		}

		archivo = malloc(estructuraAdministrativa.tablaArchivos[offset].tamanioArchivo);
			//me conviene levantar el archivo, y guardar solo lo que necesito
		archivo = leerArchivo(path,mapa,&i);
		cerradoArchivo(path,socket);
		borrarArchivo(path,mapa);

		archivoTruncado = malloc(tamanio);

		i = 0;
		while(i < tamanio)			//copio los 'tamanio' bytes del viejo archivo al nuevo
		{
			archivoTruncado[i] = archivo[i];
			i++;
		}

		crearArchivo(path,mapa);
		aperturaArchivo(path,1);
		escribirArchivo(path,archivoTruncado,0,tamanio,mapa,1);
		cerradoArchivo(path,1);

		i = estructuraAdministrativa.tablaArchivos[offset].tamanioArchivo;
		estructuraAdministrativa.tablaArchivos[offset].tamanioArchivo = tamanio; //actualizo el tamaño
		estructuraAdministrativa.tablaArchivos[offset].fecha = pedirFecha();	//tiempo de ultima modificacion

		guardarEstructuraEn(mapa);
		free(archivo);
		free(archivoTruncado);
		free(copiaPath);

		log_info(logger,"Archivo: '%s' truncado de %d bytes, a %d bytes",path,i,tamanio);
		return 0;
	}
	free(copiaPath);
	return -1;
}

void leerDirectorio(char* path, int socket)
{
	char* nombreArchivo;
	char* copiaPath = malloc(50);
	char* nombreRecuperado;
	int i = 0;
	int contador = 0;
	int offset = 0xFFFF;

	strcpy(copiaPath,path);

	if(comprobarPathValido(copiaPath))
	{
		if(!strcmp(copiaPath,"/"))
			copiaPath[0] = '\0';

		while(strlen(copiaPath))
		{
			nombreArchivo = malloc(18);
			recorrerDesdeIzquierda(copiaPath,nombreArchivo);
			i = 0;
			while(i < 2048)
			{
				nombreRecuperado = malloc(18);
				recuperarNombre(i,nombreRecuperado);

				if(!strcmp(nombreRecuperado,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
					{
						free(nombreRecuperado);
						break;
					}
				free(nombreRecuperado);
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				enviarHeader(socket,0);
				free(nombreArchivo);
				break;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(nombreArchivo);
		}
			//aca tenemos el offset del directorio a listar

		if(i != 2048)
		{
			i = 0;
			while(i < 2048)		//primera ronda, cuenta los archivos que tiene que mandar
			{
				if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset && estructuraAdministrativa.tablaArchivos[i].estado != '\0')
					contador++;
				i++;
			}
			enviarHeader(socket,contador);

			i = 0;
			while(i < 2048)
			{
				if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset && estructuraAdministrativa.tablaArchivos[i].estado != '\0')
				{
					nombreArchivo = malloc(18);
					recuperarNombre(i,nombreArchivo);

					send(socket,nombreArchivo,18,0);

					log_info(logger,"Nombre enviado: %s",nombreArchivo);
					free(nombreArchivo);
				}
				i++;
			}
		log_info(logger,"PATH: '%s' leido correctamente",path);
		}
	free(copiaPath);
	}
}

void* leerArchivo(char* path,char* mapa, int* tamArchivo)
{
	char* nombreArchivo;
	char* copiaPath = malloc(50);
	char* nombreRecuperado;
	int tamanioTotalEnBytes;
	int bloqueSiguienteEnTabla;
	int posicionDelMapa;
	int i = 0;
	int offset = 0xFFFF;
	int j;
	int contadorGlobal = 0;
	void* archivoLeido;

	strcpy(copiaPath,path);

	if(comprobarPathValido(copiaPath))
	{
		while(strlen(copiaPath))  //esto saca el offset del archivo, en la tabla
		{
			nombreArchivo = malloc(18);
			recorrerDesdeIzquierda(copiaPath,nombreArchivo);
			i = 0;
			while(i < 2048)
			{
				nombreRecuperado = malloc(18);
				recuperarNombre(i,nombreRecuperado);

				if(!strcmp(nombreRecuperado,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
					{
						free(nombreRecuperado);
						break;
					}
				free(nombreRecuperado);
				i++;
			}


			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				free(copiaPath);
				return NULL;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(nombreArchivo);
		}

		i = offset;

		if(i < 2048 && estructuraAdministrativa.tablaArchivos[i].estado == '\1')  //si definitivamente es un archivo
		{
			tamanioTotalEnBytes = estructuraAdministrativa.tablaArchivos[i].tamanioArchivo;
			bloqueSiguienteEnTabla = estructuraAdministrativa.tablaArchivos[i].bloqueInicial;
			posicionDelMapa = (estructuraAdministrativa.header.tamanioFS - estructuraAdministrativa.header.tamanioDatos + bloqueSiguienteEnTabla) * 64;  //inicializo el cabezal en bytes

			nombreArchivo = malloc(tamanioTotalEnBytes);  //ya se el tamaño que tiene, por eso le guardo el espacio en memoria

			while(bloqueSiguienteEnTabla != 0xFFFFFFFF)   //aca voy haciendo la lectura de a bloques
			{
				if((tamanioTotalEnBytes - contadorGlobal) >= 64 )  //si hay mas de un bloque, tengo que leer al menos un bloque entero
				{
					for(j = 0;j < 64;j++)
					{
						nombreArchivo[contadorGlobal] = mapa[posicionDelMapa];
						posicionDelMapa++;
						contadorGlobal++;
					}
				}
				else
				{
					for(j = 0;j < tamanioTotalEnBytes % BLOCK_SIZE;j++)			//me quedo con los bytes restantes
					{
						nombreArchivo[contadorGlobal] = mapa[posicionDelMapa];
						posicionDelMapa++;
						contadorGlobal++;
					}
				}

				bloqueSiguienteEnTabla = estructuraAdministrativa.tablaAsignaciones[bloqueSiguienteEnTabla]; //recorro el array de asignaciones
				posicionDelMapa = (estructuraAdministrativa.header.tamanioFS - estructuraAdministrativa.header.tamanioDatos + bloqueSiguienteEnTabla) * 64; //vuelvo a posicionar
					//aca no hay drama si el bloque es 0xFFFFFFF, porque sale del bucle
			}

			archivoLeido = malloc(tamanioTotalEnBytes);
			memcpy(archivoLeido,nombreArchivo,tamanioTotalEnBytes);  //use el char* para obtener los datos, y se lo pase a un void* para que no tenga formato
			*tamArchivo = tamanioTotalEnBytes;

			free(nombreArchivo);
			free(copiaPath);
			return archivoLeido;
		}
		else
		{
			free(copiaPath);
			return NULL;
		}
	}
	else
	{
		free(copiaPath);
		return NULL;
	}
}

int pedirFecha(void)
{
	return (int)time(NULL);
}

void sacarNombre(char* path,char* nombre)  //CUIDADO: devuelve el path sin el nombre
{
	int i = 0;
	int marcador;
	int j;

	while(path[i] != '\0')
	{
		if(path[i] == '/' && path[i+1] != '\0')  //si es algo como '/pokedex/' se ignora
			marcador = i;  //me dejo guardada la ubicacion de la ultima '/'
		i++;
	}

	i = marcador + 1;
	j = marcador;  // me guardo la posicion de la ultima letra del path antes de la '/'
	marcador = 0;
	while(path[i] != '\0')
	{
		nombre[marcador] = path[i];
		marcador++;
		i++;
	}

	path[j] = '\0'; //y asi, el path perdio su ultimo nombre mi querido padawan

	nombre[marcador] = '\0';
}

void recorrerDesdeIzquierda(char* path, char* nombre)
{
	int i = 0;
	int j = 0;
	int largo;

	if(path[i] == '/')   //si llega algo asi:  '/entrenador/pikachu'    queda algo asi:   'entrenador/pikachu'
		i++;

	while(path[i] != '/' && path[i] != '\0')
	{
		nombre[j] = path[i];
		i++;
		j++;
	}

	nombre[j] = '\0';
	largo = i;

	for(i = 0;path[i + largo] != '\0';i++)
		path[i] = path[i + largo];
	path[i] = '\0';
}

void guardarEstructuraEn(char* mapa)
{
	int peso,  //este es el tamaño de los campos, medidos en BYTES
		marcador = 0,   //fija el offset al inicio, sirve para saber DESDE donde escribir
		offset = 0,		//sirve para saber HASTA donde escribir
		contadorEnCero = 0;
	char* mander;
						//primero guardo el header
	peso = sizeof(t_header);
	mander = malloc(peso);
	memcpy(mander,&estructuraAdministrativa.header,peso);
	while(offset < (peso + marcador))
	{
		mapa[offset] = mander[offset];
		offset++;
	}
	marcador = offset;
	free(mander);

					//ahora guardo el bitarray
	peso = estructuraAdministrativa.header.tamanioBitmap * 64;
	mander = malloc(peso);
	memcpy(mander,estructuraAdministrativa.punteroBitmap->bitarray,peso);
	while(offset < (peso + marcador))
	{
		mapa[offset] = mander[contadorEnCero];
		offset++;
		contadorEnCero++;
	}
	marcador = offset;
	free(mander);
	contadorEnCero = 0;

					//ahora guardo la tabla de archivos
	peso = sizeof(osadaFile) * 2048;
	mander = malloc(peso);
	memcpy(mander,estructuraAdministrativa.tablaArchivos,peso);
	while(offset < (peso + marcador))
	{
		mapa[offset] = mander[contadorEnCero];
		offset++;
		contadorEnCero++;
	}
	marcador = offset;
	free(mander);
	contadorEnCero = 0;

					//ahora guardo la tabla de asignaciones
	peso = estructuraAdministrativa.header.tamanioFS - estructuraAdministrativa.header.tamanioDatos;
	peso = peso - 1 - estructuraAdministrativa.header.tamanioBitmap - (sizeof(osadaFile) * 2048 / 64);  //total estructura - header - bitmap - tabla de archivos
	peso = peso * 64;  //lo paso a bytes
	mander = malloc(peso);
	memcpy(mander,estructuraAdministrativa.tablaAsignaciones,peso);
	while(offset < (peso + marcador))
	{
		mapa[offset] = mander[contadorEnCero];
		offset++;
		contadorEnCero++;
	}
	marcador = offset;
	free(mander);
	log_info(logger,"Estructuras administrativas guardadas correctamente");
}

int crearDirectorio(char* path,char* mapa)
{
	int i = 0;
	int offset = 0xFFFF;  //con esta recorro el bitmap
	char* nombreArchivo = malloc(18);
	char* nombreDirectorio = malloc(18);
	char* copiaPath = malloc(50);
	char* nombreRecuperado;

	strcpy(copiaPath,path);
	sacarNombre(copiaPath,nombreDirectorio);   //ahora tengo el nombre que le quieren dar al directorio

	if(strlen(nombreDirectorio) > 17)
	{
		log_error(logger,"El nombre que le quieren dar al directorio es demasiado largo. Nombre: %s",nombreDirectorio);
		free(copiaPath);
		free(nombreDirectorio);
		return -2;
	}

	if(comprobarPathValido(copiaPath))  //si el path es correcto
	{
		while(strlen(copiaPath))  //esto saca el offset del archivo, en la tabla
		{
			nombreArchivo = malloc(18);
			recorrerDesdeIzquierda(copiaPath,nombreArchivo);
			i = 0;
			while(i < 2048)
			{
				nombreRecuperado = malloc(18);
				recuperarNombre(i,nombreRecuperado);

				if(!strcmp(nombreRecuperado,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
					{
						free(nombreRecuperado);
						break;
					}
				free(nombreRecuperado);
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				free(nombreDirectorio);
				free(copiaPath);
				return -1;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(nombreArchivo);
		}					//aca tengo el offset del bloque padre, en offset

		i = 0;
		while((i < 2048) && estructuraAdministrativa.tablaArchivos[i].estado != '\0')
			i++;

		if(i == 2048)   //si llego a 2048, es porque no hay lugar en el array de archivos
		{
			log_error(logger,"Tabla de archivos completamente ocupada");
			free(copiaPath);
			free(nombreDirectorio);
			return -3;
		}
		else
		{
			estructuraAdministrativa.tablaArchivos[i].estado = '\2'; //es un directorio
			estructuraAdministrativa.tablaArchivos[i].bloquePadre = offset;
			grabarNombreEn(i,nombreDirectorio);

					//ahora le asigno un bloque libre
			int bitmapOffset = 0;
			while(bitarray_test_bit(estructuraAdministrativa.punteroBitmap,bitmapOffset))  //hasta que no encuentre un '0'...
				bitmapOffset++;
			bitarray_set_bit(estructuraAdministrativa.punteroBitmap,bitmapOffset); //y actualizo el bitmap
				//este es el offset global del bitmap, pero en realidad yo quiero el offset en la tablaAsignaciones

			bitmapOffset = bitmapOffset - (estructuraAdministrativa.header.tamanioFS - estructuraAdministrativa.header.tamanioDatos);
				//aca tengo el offset de la tabla, me dice que bloque de datos es el que tengo en esta posicion
			estructuraAdministrativa.tablaAsignaciones[bitmapOffset] = 0xFFFFFFFF;
				//voy a la tabla en ese offset, y como es unico, le pongo 0xFFFFFF

			estructuraAdministrativa.tablaArchivos[i].bloqueInicial = bitmapOffset;
					//por ultimo, guardo el offset de la tablaAsig en donde esta su primer bloque

			estructuraAdministrativa.tablaArchivos[i].tamanioArchivo = 0; //el directorio vacio no pesa nada
			estructuraAdministrativa.tablaArchivos[i].fecha = pedirFecha();

			free(copiaPath);
			free(nombreDirectorio);

			guardarEstructuraEn(mapa);
			log_info(logger,"Directorio creado. Nombre: '%s'",path);
			return 0;
		}
	}
	free(copiaPath);
	free(nombreDirectorio);
	return -1;
}

int crearArchivo(char* path,char* mapa)
{
	int i = 0;
	int offset = 0xFFFF;  //con esta recorro el bitmap
	int verificador = 0;
	char* nombreEfectivo = malloc(18);
	char* copiaPath = malloc(50);
	char* nombreArchivo;
	char* nombreRecuperado;

	strcpy(copiaPath,path);
	sacarNombre(copiaPath,nombreEfectivo);   //ahora tengo el nombre que le quieren dar al archivo

	if(strlen(nombreEfectivo) > 17)
	{
		log_error(logger,"El nombre que le quieren dar al archivo es demasiado largo. Nombre: %s",nombreEfectivo);
		free(copiaPath);
		free(nombreEfectivo);
		return -2;
	}

	if(comprobarPathValido(copiaPath))  //si el path es correcto
	{
		while(strlen(copiaPath))  //esto saca el offset del archivo, en la tabla
		{
			nombreArchivo = malloc(18);
			recorrerDesdeIzquierda(copiaPath,nombreArchivo);
			i = 0;
			while(i < 2048)
			{
				nombreRecuperado = malloc(18);
				recuperarNombre(i,nombreRecuperado);

				if(!strcmp(nombreRecuperado,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
					{
						free(nombreRecuperado);
						break;
					}
				free(nombreRecuperado);
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				free(copiaPath);
				free(nombreEfectivo);
				return -1;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(nombreArchivo);
		}					//aca tengo el offset del bloque padre, en offset

		i = 0;
		while((i < 2048) && estructuraAdministrativa.tablaArchivos[i].estado != '\0')
			i++;

		if(i == 2048)   //si llego a 2048, es porque no hay lugar en el array de archivos
		{
			log_error(logger,"Tabla de archivos completamente ocupada");
			free(copiaPath);
			free(nombreEfectivo);
			return -3;
		}
		else
		{
			estructuraAdministrativa.tablaArchivos[i].estado = '\1'; //es un archivo
			grabarNombreEn(i,nombreEfectivo);
			estructuraAdministrativa.tablaArchivos[i].bloquePadre = offset;  //el offset, es el del bloque padre encontrado mas arriba

					//ahora le asigno un bloque libre
			offset = 0;
			while(offset < estructuraAdministrativa.header.tamanioFS)
			{
				if(!bitarray_test_bit(estructuraAdministrativa.punteroBitmap,offset))
					verificador++;
				offset++;
			}
				//aca verifico el espacio disponible

			if(!verificador)
			{
				log_error(logger,"No hay espacio disponible");
				free(copiaPath);
				free(nombreEfectivo);
				return -3;
			}

			offset = 0;
			while(bitarray_test_bit(estructuraAdministrativa.punteroBitmap,offset))  //hasta que no encuentre un '0'...
				offset++;
			bitarray_set_bit(estructuraAdministrativa.punteroBitmap,offset); //y actualizo el bitmap
				//este es el offset global del bitmap, pero en realidad yo quiero el offset en la tablaAsignaciones

			offset = offset - (estructuraAdministrativa.header.tamanioFS - estructuraAdministrativa.header.tamanioDatos);
				//aca tengo el offset de la tabla, me dice que bloque de datos es el que tengo en esta posicion
			estructuraAdministrativa.tablaAsignaciones[offset] = 0xFFFFFFFF;
				//voy a la tabla en ese offset, y como es un unico bloque, le pongo 0xFFFFFF

			estructuraAdministrativa.tablaArchivos[i].bloqueInicial = offset;
					//por ultimo, guardo el offset de la tablaAsig en donde esta su primer bloque

			estructuraAdministrativa.tablaArchivos[i].tamanioArchivo = 64;

			estructuraAdministrativa.tablaArchivos[i].fecha = pedirFecha();

			log_info(logger,"Archivo creado. Nombre '%s'",nombreEfectivo);
			free(nombreEfectivo);
			free(copiaPath);
			guardarEstructuraEn(mapa);

			return 1;
		}
	}
	free(copiaPath);
	free(nombreEfectivo);
	return -1;
}

int borrarArchivo(char* path,char* mapa)
{
	int i = 0;
	int offset = 0xFFFF;
	int bloqueInicioDatos;
	char* nombreArchivo = malloc(18);
	char* copiaPath = malloc(50);
	char* nombreRecuperado;

	strcpy(copiaPath,path);
	if(comprobarPathValido(path))  //si el path es correcto
	{
		while(strlen(copiaPath))  //esto saca el offset del archivo, en la tabla
		{
			nombreArchivo = malloc(18);
			recorrerDesdeIzquierda(copiaPath,nombreArchivo);
			i = 0;
			while(i < 2048)
			{
				nombreRecuperado = malloc(18);
				recuperarNombre(i,nombreRecuperado);

				if(!strcmp(nombreRecuperado,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
					{
						free(nombreRecuperado);
						break;
					}
				free(nombreRecuperado);
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				free(nombreArchivo);
				free(copiaPath);
				return -1;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(nombreArchivo);
		}
				//aca tengo unequivocamente al archivo solicitado
		if(aperturado[offset] != 0)
		{
			log_error(logger,"Archivo en uso por %d, no se puede eliminar. Path: %s",aperturado[offset],path);
			free(copiaPath);
			return -1;
		}

		i = offset;
		int viejoOffset;
		estructuraAdministrativa.tablaArchivos[i].estado = '\0'; //lo borra
		strcpy((char*)estructuraAdministrativa.tablaArchivos[i].nombre,"");
		estructuraAdministrativa.tablaArchivos[i].bloquePadre = 0xFFFF;
		estructuraAdministrativa.tablaArchivos[i].fecha = 0;

				//ahora le tengo que borrar los bloques ocupados
		bloqueInicioDatos = estructuraAdministrativa.header.tamanioFS - estructuraAdministrativa.header.tamanioDatos;
		offset = estructuraAdministrativa.tablaArchivos[i].bloqueInicial;
		viejoOffset = offset;
		while(offset != 0xFFFFFFFF)
		{
			bitarray_clean_bit(estructuraAdministrativa.punteroBitmap,(offset + bloqueInicioDatos)); //limpio el bit
			offset = estructuraAdministrativa.tablaAsignaciones[offset]; //paso al siguiente bloque
			estructuraAdministrativa.tablaAsignaciones[viejoOffset] = 0; //reinicio la tabla de asignaciones
			viejoOffset = offset;
		}
			//una vez que limpie el bitmap, termino de acondicionar la estructura de ese archivo eliminado
		estructuraAdministrativa.tablaArchivos[i].bloqueInicial = 0;
		estructuraAdministrativa.tablaArchivos[i].tamanioArchivo = 0;

		guardarEstructuraEn(mapa);
		log_info(logger,"Archivo eliminado. Nombre '%s'",path);
		free(copiaPath);
		return 0;
	}
	free(copiaPath);
	return -1;
}

int borrarDirectorioVacio(char* path,char* mapa)
{
	int i = 0;
	int offset = 0xFFFF;
	int bloqueInicioDatos;
	char* nombreArchivo = malloc(18);
	char* copiaPath = malloc(50);
	char* nombreRecuperado;

	strcpy(copiaPath,path);
	if(comprobarPathValido(path))  //si el path es correcto
	{
		while(strlen(copiaPath))  //esto saca el offset del archivo, en la tabla
		{
			nombreArchivo = malloc(18);
			recorrerDesdeIzquierda(copiaPath,nombreArchivo);
			i = 0;
			while(i < 2048)
			{
				nombreRecuperado = malloc(18);
				recuperarNombre(i,nombreRecuperado);

				if(!strcmp(nombreRecuperado,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
					{
						free(nombreRecuperado);
						break;
					}
				free(nombreRecuperado);
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				free(copiaPath);
				return -1;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(nombreArchivo);
		}
				//aca tengo unequivocamente al archivo solicitado

		if(estructuraAdministrativa.tablaArchivos[offset].estado != '\2')	//si no es un directorio, avisa
		{
			log_error(logger,"El path no corresponde a un directorio. Path: %s",path);
			free(copiaPath);
			return -1;
		}

		i = 0;	//tengo que recorrer la tabla y ver si esta vacio..
		while(i < 2048)
		{
			if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset && estructuraAdministrativa.tablaArchivos[i].estado != '\0')
			{
				log_error(logger,"Se esta queriendo eliminar un directorio que no esta vacio. Path: %s",path);
				free(copiaPath);
				return -2;
			}
			i++;
		}

		i = offset;		//copie codigo, por eso hago esto

		estructuraAdministrativa.tablaArchivos[i].estado = '\0'; //lo borra
		strcpy((char*)estructuraAdministrativa.tablaArchivos[i].nombre,"");
		estructuraAdministrativa.tablaArchivos[i].bloquePadre = 0xFFFF;
		estructuraAdministrativa.tablaArchivos[i].fecha = 0;

				//ahora le tengo que borrar el bloque ocupado (es un directorio, por ende tiene uno solo
		bloqueInicioDatos = estructuraAdministrativa.header.tamanioFS - estructuraAdministrativa.header.tamanioDatos;
		offset = estructuraAdministrativa.tablaArchivos[i].bloqueInicial;
		bitarray_clean_bit(estructuraAdministrativa.punteroBitmap,(offset + bloqueInicioDatos)); //limpio el bit

			//una vez que limpie el bitmap, termino de acondicionar la estructura de ese archivo eliminado
		estructuraAdministrativa.tablaArchivos[i].bloqueInicial = 0;
		estructuraAdministrativa.tablaArchivos[i].tamanioArchivo = 0;

		free(copiaPath);
		guardarEstructuraEn(mapa);
		log_info(logger,"Directorio eliminado. Nombre '%s'",path);

		return 0;
	}
	free(copiaPath);
	return -1;
}

bool comprobarPathValido(char* path)
{
	int offset = 0xFFFF;
	int i;
	char* copiaPath = malloc(50);
	char* viejoNombre;
	char* nombreRecuperado;

	strcpy(copiaPath,path);

	if(!strcmp(copiaPath,"/"))
	{
		free(copiaPath);
		return true;
	}
	else
	{
		while(strlen(copiaPath))  //mientras siga teniendo cosas para recorrer...
		{
			viejoNombre = malloc(18);
			recorrerDesdeIzquierda(copiaPath,viejoNombre);
			i = 0;
			while(i < 2048)
			{
				nombreRecuperado = malloc(18);
				recuperarNombre(i,nombreRecuperado);

				if(!strcmp(nombreRecuperado,viejoNombre))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
					{
						free(nombreRecuperado);
						break;
					}
				free(nombreRecuperado);
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"COMP PATH VALID --- No se ha encontrado la ruta especificada. Path: '%s'",path);
				free(copiaPath);
				free(viejoNombre);
				return false;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(viejoNombre);
		}
	}
	free(copiaPath);
	return true;
}

int renombrar(char* pathOriginal, char* pathNuevo, char* mapa)
{
	int offset;
	int i;
	char* copiaPathOriginal = malloc(50);
	char* copiaPathNuevo = malloc(50);
	char* viejoNombre = malloc(17);
	char* nuevoNombre = malloc(17);
	char* nombreRecuperado;

	strcpy(copiaPathOriginal,pathOriginal);
	strcpy(copiaPathNuevo,pathNuevo);

	sacarNombre(copiaPathNuevo,nuevoNombre);   //aca tengo el nombre nuevo que le quieren dar al archivo
	sacarNombre(copiaPathOriginal,viejoNombre);

	if(strcmp(copiaPathOriginal,copiaPathNuevo))
	{
		log_error(logger,"Se está intentando cambiar dos nombres a la vez. Original: '%s'. Nuevo: '%s'",pathOriginal,pathNuevo);
		free(copiaPathOriginal);
		free(copiaPathNuevo);
		free(nuevoNombre);
		free(viejoNombre);
		return -1;
	}
	if(strlen(nuevoNombre) > 17)
	{
		log_error(logger,"Nombre '%s' demasiado largo",nuevoNombre);
		free(copiaPathOriginal);
		free(copiaPathNuevo);
		free(nuevoNombre);
		free(viejoNombre);
		return -2;
	}
	else
	{
			//tras verificar que estan bien los paths, debo volver a inicializarlos
		free(copiaPathOriginal);
		copiaPathOriginal = malloc(50);
		strcpy(copiaPathOriginal,pathOriginal);

			//ahora tengo el path original entero de nuevo, debo recorrerlo
		offset = 0xFFFF;
		while(strlen(copiaPathOriginal))  //mientras siga teniendo cosas para recorrer...
		{
			viejoNombre = malloc(18);
			recorrerDesdeIzquierda(copiaPathOriginal,viejoNombre);
			i = 0;
			while(i < 2048)
			{
				nombreRecuperado = malloc(18);
				recuperarNombre(i,nombreRecuperado);

				if(!strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,viejoNombre))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
					{
						free(nombreRecuperado);
						break;
					}
				free(nombreRecuperado);
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",pathOriginal);
				free(copiaPathOriginal);
				free(copiaPathNuevo);
				free(nuevoNombre);
				free(viejoNombre);
				return -1;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(viejoNombre);
		}
				//en este punto tengo el punto del archivo en la tabla, listo para modificar
		if(i != 2048)
		{
			estructuraAdministrativa.tablaArchivos[i].fecha = pedirFecha();
			grabarNombreEn(i,nuevoNombre);

				//una vez cambiadas las cosas necesarias, no queda mas que guardar
			guardarEstructuraEn(mapa);
			log_info(logger,"Archivo renombrado. Original: '%s'. Nuevo: '%s'",pathOriginal,pathNuevo);

			free(copiaPathOriginal);
			free(copiaPathNuevo);
			free(nuevoNombre);
			return 0;
		}
		return 0;
	}
}

int escribirArchivo(char* path, char* fichero, int off, int tam, char* mapa, int socket)
{
	int i;
	int j;
	int otroContador;
	int offset = 0xFFFF;
	int posicionMapa;
	int bloqueSiguienteEnTabla;
	int bloqueInicioDatos;
	int verificadorEspacioDisponible = 0;
	char* copiaPath = malloc(50);
	char* viejoNombre;
	char* nombreRecuperado;

	strcpy(copiaPath,path);

	if(comprobarPathValido(path))
	{
		while(strlen(copiaPath))  //esto saca el offset del archivo, en la tabla
		{
			viejoNombre = malloc(17);
			recorrerDesdeIzquierda(copiaPath,viejoNombre);
			i = 0;
			while(i < 2048)
			{
				nombreRecuperado = malloc(18);
				recuperarNombre(i,nombreRecuperado);

				if(!strcmp(nombreRecuperado,viejoNombre))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
					{
						free(nombreRecuperado);
						break;
					}
				free(nombreRecuperado);
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				free(copiaPath);
				free(viejoNombre);
				return -1;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(viejoNombre);
		}
							//aca tengo el offset de la tabla, o '2048' si es error
		if(estructuraAdministrativa.tablaArchivos[offset].estado == '\2')        //si es un directorio, no debe ser escrito
		{
			log_error(logger,"No se puede escribir un directorio");
			free(copiaPath);
			return -1;
		}
		else
		{
			bloqueInicioDatos = estructuraAdministrativa.header.tamanioFS - estructuraAdministrativa.header.tamanioDatos;
			bloqueSiguienteEnTabla = estructuraAdministrativa.tablaArchivos[offset].bloqueInicial;

			i = 0;
			otroContador = 0;
			while (i < off)
			{
				if(otroContador == 64)
				{
					bloqueSiguienteEnTabla = estructuraAdministrativa.tablaAsignaciones[bloqueSiguienteEnTabla];
					otroContador = 0;
				}
				i++;
				otroContador++;
			}
						//hasta aca tengo, el bloque donde va a escribir, y el offset inicial, guardado en 'otroContador'
			i = 0;

			posicionMapa = (bloqueInicioDatos + bloqueSiguienteEnTabla) * BLOCK_SIZE;  //me paro en el bloque...
			posicionMapa += otroContador; 	//..y en el offset

			j = 0;
			while(j < estructuraAdministrativa.header.tamanioFS)
			{
				if(!bitarray_test_bit(estructuraAdministrativa.punteroBitmap,j))
					verificadorEspacioDisponible++;
				j++;
			}
				//aca verifico el espacio disponible

			if(tam > verificadorEspacioDisponible * BLOCK_SIZE)
			{
				log_error(logger,"No hay espacio disponible");
				return -2;
			}

			while(i < tam)	//ahora tengo que escribir el tamaño que me indican
			{
				if(otroContador == 64)		//este contador me va a indicar los cambios de bloque
				{
					if(estructuraAdministrativa.tablaAsignaciones[bloqueSiguienteEnTabla] == 0xFFFFFFFF)
					{
						j = 0;
						while(bitarray_test_bit(estructuraAdministrativa.punteroBitmap,j) && (j < estructuraAdministrativa.header.tamanioFS))
							j++;
						   //donde para j, es el offset del bloque libre
						bitarray_set_bit(estructuraAdministrativa.punteroBitmap,j);
						   //aca ya reserve el bloque
						estructuraAdministrativa.tablaAsignaciones[bloqueSiguienteEnTabla] = j - bloqueInicioDatos;
							//finalmente, le aseigno el nuevo bloque libre de datos

						bloqueSiguienteEnTabla = estructuraAdministrativa.tablaAsignaciones[bloqueSiguienteEnTabla];
								//con esto obtengo el bloque siguiente para guardar datos
						estructuraAdministrativa.tablaAsignaciones[bloqueSiguienteEnTabla] = 0xFFFFFFFF;
								//y finalmente, con esta mierda, le doy un cierre al puto archivo
					}
					else
						bloqueSiguienteEnTabla = estructuraAdministrativa.tablaAsignaciones[bloqueSiguienteEnTabla];

					posicionMapa = (bloqueInicioDatos + bloqueSiguienteEnTabla) * BLOCK_SIZE;
					otroContador = 0;
				}

				mapa[posicionMapa] = fichero[i];
				i++;
				posicionMapa++;
				otroContador++;
			}
			estructuraAdministrativa.tablaArchivos[offset].tamanioArchivo = off + tam;
			estructuraAdministrativa.tablaArchivos[offset].fecha = pedirFecha();

			guardarEstructuraEn(mapa);
			log_info(logger,"Archivo correctamente guardado. Path: %s",path);
			free(copiaPath);
			return 0;
		}
	}
	free(copiaPath);
	return -1;
}

int aperturaArchivo(char* path,int socket)
{
	char* nombreArchivo;
	char* copiaPath = malloc(50);
	char* nombreRecuperado;
	int i = 0;
	int offset = 0xFFFF;

	strcpy(copiaPath,path);				//aca puede ser, o un archivo existente, o uno nuevo

	if(comprobarPathValido(copiaPath))  //si existe....
	{
		while(strlen(copiaPath))
		{
			nombreArchivo = malloc(18);
			recorrerDesdeIzquierda(copiaPath,nombreArchivo);
			i = 0;
			while(i < 2048)
			{
				nombreRecuperado = malloc(18);
				recuperarNombre(i,nombreRecuperado);

				if(!strcmp(nombreRecuperado,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
					{
						free(nombreRecuperado);
						break;
					}
				free(nombreRecuperado);
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				free(copiaPath);
				free(nombreArchivo);
				return -1;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(nombreArchivo);
		}
			//aca tenemos el offset del directorio a contar

		if(aperturado[offset] != 0)   //si el archivo ya esta abierto, sea por quien sea, devuelve ERROR
		{
			free(copiaPath);
			return -1;
		}
		else
			aperturado[offset] = socket;

	nombreArchivo = malloc(18);
	recuperarNombre(offset,nombreArchivo);
	log_info(logger,"Archivo '%s' abierto por el dexCliente N. %d",nombreArchivo,socket);
	free(nombreArchivo);
	free(copiaPath);
	return 0;
	}

	free(copiaPath);
	return -1;
}

int cerradoArchivo(char* path,int socket)
{
	char* nombreArchivo;
	char* copiaPath = malloc(50);
	char* nombreRecuperado;
	int i = 0;
	int offset = 0xFFFF;

	strcpy(copiaPath,path);

	if(comprobarPathValido(copiaPath))
	{
		while(strlen(copiaPath))
		{
			nombreArchivo = malloc(18);
			recorrerDesdeIzquierda(copiaPath,nombreArchivo);
			i = 0;
			while(i < 2048)
			{
				nombreRecuperado = malloc(18);
				recuperarNombre(i,nombreRecuperado);

				if(!strcmp(nombreRecuperado,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
					{
						free(nombreRecuperado);
						break;
					}
				free(nombreRecuperado);
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				free(copiaPath);
				free(nombreArchivo);
				return -1;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(nombreArchivo);
		}
			//aca tenemos el offset del directorio a contar

		if(aperturado[offset] == socket)
			aperturado[offset] = 0;

	log_info(logger,"Archivo '%s' cerrado por el dexCliente N. %d",estructuraAdministrativa.tablaArchivos[offset].nombre,socket);
	free(copiaPath);
	}
	else
		free(copiaPath);
	return 0;
}

void grabarNombreEn(int i, char* nombre)
{
	int contador;

	for(contador = 0; contador < strlen(nombre);contador++)
	{
		estructuraAdministrativa.tablaArchivos[i].nombre[contador] = nombre[contador];
	}

	if(contador < 17)
		estructuraAdministrativa.tablaArchivos[i].nombre[contador] = '\0';
}

void recuperarNombre(int i,char* nombre)
{
	int contador;

	for(contador = 0; contador < 17;contador++)
	{
		nombre[contador] = estructuraAdministrativa.tablaArchivos[i].nombre[contador];
	}
	nombre[contador] = '\0';
}
