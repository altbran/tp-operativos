#include <stdio.h>
#include <string.h>
#include <stdlib.h>			 //recargar el FS con dos nuevos dir, y volver a probar fuse
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

void leerEstructurasAdministrativas(FILE* archivo);
int pedirFecha(void);
void sacarNombre(char* path,char* nombre);
bool comprobarPathValido(char* path);
void recorrerDesdeIzquierda(char* path, char* nombre);
void guardarEstructuraEn(char* mapa);
void atenderConexion(int socket, char* mapa);
int getAtr(char* path,char* mapa,int* tamanio);
void leerDirectorio(char* path, int socket);
void* leerArchivo(char* path,char* mapa, int* tamArchivo);
int crearDirectorio(char* path,char* mapa);
void crearArchivo(char* path,char* mapa);
int borrarArchivo(char* path,char* mapa);
void borrarDirectorioVacio(char* path,char* mapa);
void renombrar(char* pathOriginal, char* pathNuevo, char* mapa);
int escribirArchivo(char* path, char* fichero, int tam, char* mapa,int socket);
int aperturaArchivo(char* path,char* mapa,int socket);
int cerradoArchivo(char* path,int socket);


//Variables globales

t_log* logger;
t_estructuraAdministrativa estructuraAdministrativa;
int aperturado[2048] = {0};

int main(void) {

	logger = log_create("dexServer.log","DEXSERVER",0,log_level_from_string("INFO"));  //creo el archivo de log

	int PUERTO_POKEDEX_SERVIDOR = 9000; //getenv("PUERTO_POKEDEX_SERVIDOR");
	char* IP_POKEDEX_SERVIDOR = getenv("IP_POKEDEX_SERVIDOR");
	FILE* archivo;

	int i;
	int socketNuevo;
//	int paquete;
	int listener;   //socket listener
	int fdmax;     //file descriptor maximo
	fd_set bolsaDeSockets;
	fd_set bolsaAuxiliar;
	char* archivoMapeado;

	archivoMapeado = getenv("PUERTO_POKEDEX_SERVIDOR");        //la uso temporalmente, para evitar aumentar el heap
	PUERTO_POKEDEX_SERVIDOR = atoi(archivoMapeado);

	log_info(logger, "PUERTO_POKEDEX_SERVIDOR = %d",PUERTO_POKEDEX_SERVIDOR);
	log_info(logger, "IP_POKEDEX_SERVIDOR = %s",IP_POKEDEX_SERVIDOR);


	archivo = fopen("fileSystem.dat","rb+");
	fdmax = fileno(archivo);      //uso temporariamente el fdmax para almacenar el fd de mi FS

	leerEstructurasAdministrativas(archivo);   //me guardo las est.adm en una struct porque va a ser mas comodo leerlas desde ahi

	fseek(archivo,0,SEEK_END);
	i = ftell(archivo);        //uso i temporariamente para sacar la longitud en bytes del archivo
	archivoMapeado = malloc(i); //le reservo i-bytes al archivo mapeado, es decir, 1M
	if (fdmax == -1)
		log_error(logger, "Error al obtener el 'file descriptor' del archivo");
	archivoMapeado = mmap(NULL, i, PROT_READ | PROT_WRITE, MAP_SHARED, fdmax, 0);
	if (archivoMapeado == MAP_FAILED)
		log_error(logger, "Error al mapear el File System en memoria");

	log_info(logger, "Se mapeo correctamente el archivo en memoria");

	//ademas, mapeo el FS


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

	/*crearDirectorio("/Entrenadores",archivoMapeado);
	crearDirectorio("/Mapas",archivoMapeado);
	crearDirectorio("/Mapas/Paleta",archivoMapeado);
	crearDirectorio("/Mapas/Paleta/Pokenests",archivoMapeado);*/

	//crearArchivo("/entrenador/juan/pikachu.dat",archivoMapeado);
	//crearArchivo("/pokedex/ruperto.dat",archivoMapeado);

	/*char* pruebaMapaArchivo;
	FILE* el = fopen("pikachu.dat","rb+");
	fdmax = fileno(el);
	fseek(el,0,SEEK_END);
	i = ftell(el);
	pruebaMapaArchivo = malloc(i);
	pruebaMapaArchivo = mmap(NULL, i, PROT_READ | PROT_WRITE, MAP_SHARED, fdmax, 0);
	escribirArchivo("/entrenador/juan/pikachu.dat",pruebaMapaArchivo,i,archivoMapeado);*/

	//char* el = leerArchivo("/entrenador/juan/pikachu.dat",archivoMapeado);


	//renombrar("/entrenador/juan/algo.dat","/entrenador/juan/esOtroAlgo.dat",archivoMapeado);

	//borrarArchivo("/entrenador/juan/pikachu.dat",archivoMapeado);
	//borrarDirectorioVacio("/entrenador/juan",archivoMapeado);

	//comprobarPathValidoLectura("/pokedex");
	//comprobarPathValidoLectura("/entrenador/pikachu.dat");
	//comprobarPathValidoLectura("/pokedex/pikachu.dat");


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
							FD_SET(socketNuevo, &bolsaDeSockets);

							log_info(logger, "Nuevo dexCliente conectado, socket= %d", socketNuevo);
							break;

						default:
							close(socketNuevo);
							log_error(logger, "Error en el handshake. Conexion inesperada");
							break;
					}
				} //-->if del listener
				else
				{  //aca mas adelante, voy a tener que crear hilos

					//TODO hilos
					atenderConexion(i,archivoMapeado);
				}
			}
		} //for del select
	}//cierra el while





	fclose(archivo);
	free(estructuraAdministrativa.punteroBitmap->bitarray);
	bitarray_destroy(estructuraAdministrativa.punteroBitmap);
	free(estructuraAdministrativa.tablaAsignaciones);
	free(estructuraAdministrativa.tablaArchivos);
	free(archivoMapeado);

	log_destroy(logger);
	return 0;
}


void atenderConexion(int socket, char* mapa)
{
	int operacion;
	int resultado;
	int tamanio;
	char* path = malloc(50);
	void* archivo = NULL;

	operacion = recibirHeader(socket);

	while(operacion != socketDesconectado)
	{
		log_info(logger,"socket: %d, operacion: %d",socket,operacion);

		switch(operacion)  //magia potagia TODO
		{
			case abrirArchivo:
				recibirTodo(socket,path,50);
				resultado = aperturaArchivo(path,mapa,socket);
				enviarHeader(socket,resultado);
				break;

			case contenidoArchivo:
				recibirTodo(socket,path,50);
				archivo = leerArchivo(path,mapa,&tamanio);
				enviarHeader(socket,tamanio);
				send(socket,archivo,tamanio,0);
				log_info(logger,"Se envio el archivo %s",path);
				free(archivo);
				break;

			case privilegiosArchivo:
				recibirTodo(socket,path,50);

				resultado = getAtr(path,mapa,&tamanio);
				log_info(logger,"Para el PATH: '%s' se esta mandando DIR: %d  TAMANIO: %d",path,resultado,tamanio);

				enviarHeader(socket,resultado);
				enviarHeader(socket,tamanio);
				break;

			case contenidoDirectorio:
				recibirTodo(socket,path,50);
				enviarHeader(socket,comprobarPathValido(path));
				leerDirectorio(path,socket);
				break;

			case escribirEnFichero:
				recibirTodo(socket,path,50);
				tamanio = recibirHeader(socket);

				void* ficheroEnviado = malloc(tamanio);
				recv(socket,ficheroEnviado,tamanio,0);

				resultado = escribirArchivo(path,ficheroEnviado,tamanio,mapa,socket);
				enviarHeader(socket,resultado);
				log_info(logger,"Resultado de la escritura de '%s': %d",resultado);
				free(ficheroEnviado);
				break;

			case eliminarArchivo:
				recibirTodo(socket,path,50);
				resultado = borrarArchivo(path,mapa);
				enviarHeader(socket,resultado);
				log_info(logger,"Resultado del borrado de '%s': %d",path,resultado);
				break;

			case cerrarArchivo:
				recibirTodo(socket,path,50);
				resultado = cerradoArchivo(path,socket);
				enviarHeader(socket,resultado);
				break;

			case crearCarpeta:
				recibirTodo(socket,path,50);
				resultado = crearDirectorio(path,mapa);
				enviarHeader(socket,resultado);
				log_info(logger,"Resultado del mkdir del path %s: %d",path,resultado);
				break;

			default:
				log_info(logger,"Todavia no implementado. Codigo operacion: %d",operacion);
				break;
		}

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
	int i = 0;
	int offset = 0xFFFF;

	strcpy(copiaPath,path);

	if(comprobarPathValido(copiaPath))
	{
		if(!strcmp(copiaPath,"/"))
		{
			*tamanio = 0;
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
					if(!strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombreArchivo))
						if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
							break;
					i++;
				}


				if(i == 2048) //si llego al final es porque no encontró nada
				{
					log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
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

			log_warning(logger,"Lo encontrado. Nombre: %s   Estado: %u",estructuraAdministrativa.tablaArchivos[offset].nombre,estructuraAdministrativa.tablaArchivos[offset].estado);

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

void leerDirectorio(char* path, int socket)
{
	char* nombreArchivo;
	char* copiaPath = malloc(50);
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
				if(!strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
						break;
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				break;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(nombreArchivo);
		}
			//aca tenemos el offset del directorio a contar

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
				strcpy(nombreArchivo,(char*)estructuraAdministrativa.tablaArchivos[i].nombre);
				send(socket,nombreArchivo,18,0);

				log_info(logger,"Nombre enviado: %s",nombreArchivo);
				free(nombreArchivo);
			}
			i++;
		}
	log_info(logger,"PATH: '%s' leido correctamente",path);
	free(copiaPath);
	}
}

void* leerArchivo(char* path,char* mapa, int* tamArchivo)
{
	char* nombreArchivo;
	char* copiaPath = malloc(50);
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
				if(!strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
						break;
				i++;
			}


			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				break;
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
						contadorGlobal++;
					}
				}
				else
				{
					for(j = 0;j < tamanioTotalEnBytes % BLOCK_SIZE;j++)			//me quedo con los bytes restantes
					{
						nombreArchivo[contadorGlobal] = mapa[posicionDelMapa];
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

	strcpy(copiaPath,path);
	sacarNombre(copiaPath,nombreDirectorio);   //ahora tengo el nombre que le quieren dar al directorio

	log_info(logger,"path: %s, nombre: %s",copiaPath,nombreDirectorio);

	if(comprobarPathValido(copiaPath))  //si el path es correcto
	{
		while(strlen(copiaPath))  //esto saca el offset del archivo, en la tabla
		{
			nombreArchivo = malloc(18);
			recorrerDesdeIzquierda(copiaPath,nombreArchivo);
			i = 0;
			while(i < 2048)
			{
				if(!strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
						break;
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
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
			return -1;
		}
		else
		{
			estructuraAdministrativa.tablaArchivos[i].estado = '\2'; //es un directorio
			estructuraAdministrativa.tablaArchivos[i].bloquePadre = offset;
			log_info(logger,"asdasdasdasdasdasdasdasdasdasdasdsadasdadasddddddddddddd %s",nombreDirectorio);
			strcpy((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombreDirectorio);

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

			guardarEstructuraEn(mapa);
			log_info(logger,"Directorio creado. Nombre: '%s'",path);
			return 0;
		}
	}
	return -1;
}

void crearArchivo(char* path,char* mapa)
{
	int i = 0;
	int offset = 0;  //con esta recorro el bitmap
	char* nombreEfectivo = malloc(18);
	char* pathAuxiliar = malloc(50);

	strcpy(pathAuxiliar,path);
	sacarNombre(pathAuxiliar,nombreEfectivo);   //ahora tengo el nombre que le quieren dar al archivo

	if(comprobarPathValido(pathAuxiliar))  //si el path es correcto
	{
		while((i < 2048) && estructuraAdministrativa.tablaArchivos[i].estado != '\0')
			i++;

		if(i == 2048)   //si llego a 2048, es porque no hay lugar en el array de archivos
			log_error(logger,"Tabla de archivos completamente ocupada");
		else
		{
			estructuraAdministrativa.tablaArchivos[i].estado = '\1'; //es un archivo
			strcpy((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombreEfectivo);

					//ahora le asigno un bloque libre
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

			estructuraAdministrativa.tablaArchivos[i].tamanioArchivo = 0; //el archivo vacio no pesa nada
			if(strlen(pathAuxiliar))
			{
				sacarNombre(pathAuxiliar,nombreEfectivo); //reutilizo la variable 'nombreEfectivo', total no se vuelve a usar
				offset = 0;
				while(strcmp((char*)estructuraAdministrativa.tablaArchivos[offset].nombre,nombreEfectivo)) //ahora debo saber el offset del bloque padre
					offset++;
				estructuraAdministrativa.tablaArchivos[i].bloquePadre = offset; //una vez que encontre el offset del bloque padre, guardo
			}
			else
				estructuraAdministrativa.tablaArchivos[i].bloquePadre = 0xFFFF; //si el path está vacio, entonces esta en el raiz

			estructuraAdministrativa.tablaArchivos[i].fecha = pedirFecha();

			free(nombreEfectivo);
			free(pathAuxiliar);

			guardarEstructuraEn(mapa);
			log_info(logger,"Archivo creado. Nombre '%s'",path);
		}
	}
}

int borrarArchivo(char* path,char* mapa)
{
	int i = 0;
	int offset = 0xFFFF;
	int bloqueInicioDatos;
	char* nombreArchivo = malloc(18);
	char* copiaPath = malloc(50);

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
				if(!strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
						break;
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				return -1;;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(nombreArchivo);
		}
				//aca tengo unequivocamente al archivo solicitado
		if(aperturado[offset] != 0)
		{
			log_error(logger,"Archivo en uso, no se puede eliminar. Path: %s",path);
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

void borrarDirectorioVacio(char* path,char* mapa)
{
	int i = 0;
	int offset = 0;
	int bloqueInicioDatos;
	char* nombreEfectivo = malloc(18);
	char* pathAuxiliar = malloc(50);

	strcpy(pathAuxiliar,path);
	if(comprobarPathValido(path))  //si el path es correcto
	{
		sacarNombre(pathAuxiliar,nombreEfectivo);   //ahora tengo el nombre del directorio a borrar

		while((i < 2048) && strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombreEfectivo))
			i++;

		if(i == 2048)   //si llego a 2048, es porque no lo encontro
			log_error(logger,"El directorio '%s' a eliminar no existe",path);
		else
		{
					//ahora tengo que saber si está vacio, es decir, que no esté de bloque padre de nadie
			while((offset < 2048) && estructuraAdministrativa.tablaArchivos[offset].bloquePadre != i)
				offset++;

			if(offset != 2048)   //si no llego a 2048, es porque tiene archivos adentro, osea, no lo puedo borrar
				log_error(logger,"El directorio '%s' tiene archivos adentro. No puede ser eliminado.",path);
			else
			{
						//una vez comprobado que estén las cosas bien, paso a eliminarlo
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

				free(nombreEfectivo);
				free(pathAuxiliar);

				guardarEstructuraEn(mapa);
				log_info(logger,"Directorio eliminado. Nombre '%s'",path);
			}
		}
	}
}

bool comprobarPathValido(char* path)
{
	int offset = 0xFFFF;
	int i;
	char* copiaPath = malloc(50);
	char* otraCopia = malloc(50);
	char* viejoNombre;
	char* otroNombre = malloc(18);

	strcpy(copiaPath,path);
	strcpy(otraCopia,path);

	if(!strcmp(copiaPath,"/"))
	{
		free(copiaPath);
		free(otroNombre);
		free(otraCopia);
		return true;
	}
	else
	{
		while(strlen(copiaPath))  //mientras siga teniendo cosas para recorrer...
		{
			viejoNombre = malloc(17);
			recorrerDesdeIzquierda(copiaPath,viejoNombre);
			i = 0;
			while(i < 2048)
			{
				if(!strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,viejoNombre))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
						break;
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
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

void renombrar(char* pathOriginal, char* pathNuevo, char* mapa)
{
	int offset;
	int i;
	char* copiaPathOriginal = malloc(50);
	char* copiaPathNuevo = malloc(50);
	char* viejoNombre = malloc(17);
	char* nuevoNombre = malloc(17);

	strcpy(copiaPathOriginal,pathOriginal);
	strcpy(copiaPathNuevo,pathNuevo);

	sacarNombre(copiaPathNuevo,nuevoNombre);   //aca tengo el nombre nuevo que le quieren dar al archivo
	sacarNombre(copiaPathOriginal,viejoNombre);

	if(strcmp(copiaPathOriginal,copiaPathNuevo))
	{
		log_error(logger,"Se está intentando cambiar dos nombres a la vez. Original: '%s'. Nuevo: '%s'",pathOriginal,pathNuevo);
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
				if(!strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,viejoNombre))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
						break;
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",pathOriginal);
				break;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(viejoNombre);
		}
				//en este punto tengo el punto del archivo en la tabla, listo para modificar
		if(i != 2048)
		{
			estructuraAdministrativa.tablaArchivos[i].fecha = pedirFecha();
			strcpy((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nuevoNombre);

				//una vez cambiadas las cosas necesarias, no queda mas que guardar
			guardarEstructuraEn(mapa);
			log_info(logger,"Archivo renombrado. Original: '%s'. Nuevo: '%s'",pathOriginal,pathNuevo);

			free(copiaPathOriginal);
			free(copiaPathNuevo);
			free(nuevoNombre);
		}
	}
}

int escribirArchivo(char* path, char* fichero, int tam, char* mapa, int socket)
{
	int i;
	int j;
	int otroContador = 0;
	int offset = 0xFFFF;
	int contadorGlobal = 0;
	int bloqueSiguienteEnTabla;
	int bloqueInicioDatos;
	char* copiaPath = malloc(50);
	char* viejoNombre;
	char* bloqueAGuardar;

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
				if(!strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,viejoNombre))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
						break;
				i++;
			}


			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				return -1;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(viejoNombre);
		}
							//aca tengo el offset de la tabla, o '2048' si es error
		if(offset == 2048)
		{
			log_error(logger,"El archivo no existe. Path: '%s'",path);
			return -1;
		}
		else
		{
			if(aperturado[offset] != socket)   //si no tiene este archivo abierto, o si está abierto y no es de el, no puede escribir
			{
				log_error(logger,"No tiene permiso para escribir este archivo. Path: '%s'",path);
				return -1;
			}
			else
			{
				if(estructuraAdministrativa.tablaArchivos[offset].estado == '\2')        //si es un directorio, no debe ser escrito
				{
					log_error(logger,"No se puede escribir un directorio");
					return -1;
				}
				else
				{
					i = tam / BLOCK_SIZE;
					if(tam % BLOCK_SIZE)
						i++;               //aca tengo la cantidad de bloques que tengo que guardar

					bloqueInicioDatos = estructuraAdministrativa.header.tamanioFS - estructuraAdministrativa.header.tamanioDatos;

					bloqueSiguienteEnTabla = estructuraAdministrativa.tablaArchivos[offset].bloqueInicial;

					while(otroContador < i)
					{
						bloqueAGuardar = malloc(BLOCK_SIZE);

						for(j = 0;(j < BLOCK_SIZE) && (contadorGlobal < tam); j++)   //grabo un bloque, para meterlo al mapeado
						{
							bloqueAGuardar[j] = fichero[contadorGlobal];
							contadorGlobal++;
						}

						for(j = 0;j < BLOCK_SIZE; j++)  //guardo en el mapa, el bloque de datos
						{
							mapa[(bloqueInicioDatos + bloqueSiguienteEnTabla)*BLOCK_SIZE + j] = bloqueAGuardar[j];
						}

						if(estructuraAdministrativa.tablaAsignaciones[bloqueSiguienteEnTabla] == 0xFFFFFFFF && (i - otroContador > 1))
						{
							j = 0;
							while(bitarray_test_bit(estructuraAdministrativa.punteroBitmap,j) && (j < estructuraAdministrativa.header.tamanioBitmap*BLOCK_SIZE*8))
								j++;
							   //donde para j, es el offset del bloque libre
							bitarray_set_bit(estructuraAdministrativa.punteroBitmap,j);
							   //aca ya reserve el bloque
							estructuraAdministrativa.tablaAsignaciones[bloqueSiguienteEnTabla] = j - bloqueInicioDatos;
								//finalmente, le aseigno el nuevo bloque libre de datos
						}
						bloqueSiguienteEnTabla = estructuraAdministrativa.tablaAsignaciones[bloqueSiguienteEnTabla];
								//con esto obtengo el bloque siguiente para guardar datos
						estructuraAdministrativa.tablaAsignaciones[bloqueSiguienteEnTabla] = 0xFFFFFFFF;
								//y finalmente, con esta mierda, le doy un cierre al puto archivo

						free(bloqueAGuardar);
						otroContador++;
					}

					estructuraAdministrativa.tablaArchivos[offset].tamanioArchivo = tam;
					estructuraAdministrativa.tablaArchivos[offset].fecha = pedirFecha();

					guardarEstructuraEn(mapa);
					log_info(logger,"Archivo correctamente guardado.");
					free(copiaPath);
					return 0;
				}
			}
		}
	}
	return -1;
}

int aperturaArchivo(char* path,char* mapa, int socket)
{
	char* nombreArchivo;
	char* copiaPath = malloc(50);
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
				if(!strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
						break;
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				return -1;
			}
			else
				offset = i;  //pero si no, el offset pasa a ser el contador que recorre la tabla

			free(nombreArchivo);
		}
			//aca tenemos el offset del directorio a contar

		if(aperturado[offset] != 0)   //si el archivo ya esta abierto, sea por quien sea, devuelve ERROR
			return -1;
		else
			aperturado[offset] = socket;

	log_info(logger,"Archivo '%s' abierto por el dexCliente N. %d",estructuraAdministrativa.tablaArchivos[offset].nombre,socket);
	free(copiaPath);
	return 0;
	}
	return -1;
}

int cerradoArchivo(char* path,int socket)
{
	char* nombreArchivo;
	char* copiaPath = malloc(50);
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
				if(!strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombreArchivo))
					if(estructuraAdministrativa.tablaArchivos[i].bloquePadre == offset)
						break;
				i++;
			}

			if(i == 2048) //si llego al final es porque no encontró nada
			{
				log_error(logger,"No se ha encontrado la ruta especificada. Path: '%s'",path);
				return 0;
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
	return 0;
}
