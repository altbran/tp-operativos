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

void leerEstructurasAdministrativas(FILE* archivo);
bool esDirectorio(char* path);
int pedirFecha(void);
void sacarNombre(char* path,char* nombre);
bool comprobarPathValido(char* path);
void* leerArchivo(char* pathSolicitado,t_estructuraAdministrativa est,char* mapa);
void crearDirectorio(char* path,char* mapa);

//Variables globales

t_log* logger;
t_estructuraAdministrativa estructuraAdministrativa;

int main(void) {

	logger = log_create("dexServer.log","DEXSERVER",0,log_level_from_string("INFO"));  //creo el archivo de log

	int PUERTO_POKEDEX_SERVIDOR; //getenv("PUERTO_POKEDEX_SERVIDOR");
	//int IP_POKEDEX_SERVIDOR = *getenv("IP_POKEDEX_SERVIDOR");
	FILE* archivo;

	int i;
	int socketNuevo;
//	int paquete;
	int listener;   //socket listener
	int fdmax;     //file descriptor maximo
	fd_set bolsaDeSockets;
	fd_set bolsaAuxiliar;
	void* archivoMapeado;

	archivoMapeado = getenv("PUERTO_POKEDEX_SERVIDOR");        //la uso temporalmente, para evitar aumentar el heap
	PUERTO_POKEDEX_SERVIDOR = atoi(archivoMapeado);

	log_info(logger, "PUERTO_POKEDEX_SERVIDOR = %d",PUERTO_POKEDEX_SERVIDOR);


	archivo = fopen("fileSystem.dat","rb+");
	fdmax = fileno(archivo);      //uso temporariamente el fdmax para almacenar el fd de mi FS

	leerEstructurasAdministrativas(archivo);   //me guardo las est.adm en una struct porque va a ser mas comodo leerlas desde ahi

	fseek(archivo,0,SEEK_END);
	i = ftell(archivo);        //uso i temporariamente para sacar la longitud en bytes del archivo
	log_warning(logger,"El tamaño del archivo a mapear: %d",i);
	archivoMapeado = malloc(i); //le reservo i-bytes al archivo mapeado, es decir, 1M
	if (fdmax == -1)
		log_error(logger, "Error al obtener el 'file descriptor' del archivo");
	archivoMapeado = mmap(NULL, i, PROT_READ | PROT_WRITE, MAP_SHARED, fdmax, 0);
	if (archivoMapeado == MAP_FAILED)
		log_error(logger, "Error al mapear el File System en memoria");

	log_info(logger, "Se mapeo correctamente el archivo en memoria");
	log_warning(logger,"El tamaño del archivo mapeado es: %d",strlen(archivoMapeado));

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

	log_info(logger, "Se creó correctamente el socket servidor");
	log_info(logger, "Escuchando nuevas conexiones");


	crearDirectorio("/entrenador",archivoMapeado);


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

					int idRecibido = iniciarHandshake(socketNuevo, 4/*IDPOKEDEXSERVER*/);

					switch (idRecibido)
					{
						case IDERROR:
							log_info(logger, "Se desconecto el socket");
							close(socketNuevo);
							break;

						case IDPOKEDEXCLIENTE:
							FD_SET(socketNuevo, &bolsaDeSockets);

							log_info(logger, "Nuevo dexCliente conectado, socket: %d", socketNuevo);
							break;

						default:
							close(socketNuevo);
							log_error(logger, "Error en el handshake. Conexion inesperada");
							break;
					}
				} //-->if del listener
				else
				{  //aca mas adelante, voy a tener que crear hilos

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


bool esDirectorio(char* path)
{
	int i = strlen(path);
	if(path[i-1] == '/')
		return true;
	else
		return false;
}

void* leerArchivo(char* path,t_estructuraAdministrativa est,char* mapa)
{
	char* nombreArchivo = malloc(18); //son 17 bytes maximos del nombre, mas el caracter '\0'
	char* pathSolicitado = malloc(60);
	int tamanioTotalEnBytes;
	int bloqueSiguienteEnTabla;
	int posicionDelMapa;
	int i = 0;
	int j;
	int contadorGlobal = 0;
	void* archivoLeido;

	strcpy(pathSolicitado,path);

	if(comprobarPathValido(pathSolicitado))
	{
		sacarNombre(pathSolicitado,nombreArchivo);  //obtiene el nombre del path. Esto es, despues del ultimo '/'

		i = 0;
		while(strcmp((char*)est.tablaArchivos[i].nombre,nombreArchivo) && (i < 2048))  //busco en la tabla de archivos su ubicacion
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

		if(i < 2048 && est.tablaArchivos[i].estado == '\1')  //si definitivamente es un archivo
		{
			tamanioTotalEnBytes = est.tablaArchivos[i].tamanioArchivo;
			bloqueSiguienteEnTabla = est.tablaArchivos[i].bloqueInicial;
			posicionDelMapa = (est.header.tamanioFS - est.header.tamanioDatos + bloqueSiguienteEnTabla) * 64;  //inicializo el cabezal en bytes

			free(nombreArchivo);      //reinicializo el string para poder guardarle el archivo adentro
			nombreArchivo = malloc(tamanioTotalEnBytes);  //ya se el tamaño que tiene, por eso le guardo el espacio en memoria

			while(contadorGlobal < tamanioTotalEnBytes)   //aca voy haciendo la lectura de a bloques
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

				bloqueSiguienteEnTabla = est.tablaAsignaciones[bloqueSiguienteEnTabla]; //recorro el array de asignaciones
				if(bloqueSiguienteEnTabla != 0xFFFFFFFF)
					posicionDelMapa = (est.header.tamanioFS - est.header.tamanioDatos + bloqueSiguienteEnTabla) * 64; //vuelvo a posicionar
			}

			memcpy(archivoLeido,nombreArchivo,strlen(nombreArchivo));  //use el char* para obtener los datos, y se lo pase a un void* para que no tenga formato
			free(nombreArchivo);
			free(pathSolicitado);
			return archivoLeido;
		}
		else
		{
			free(nombreArchivo);
			free(pathSolicitado);
			return NULL;
		}
	}
	else
	{
		free(nombreArchivo);
		free(pathSolicitado);
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

bool comprobarPathValido(char* path)  // ejemplo:  "/pokedex/ash/objetivos/algo.dat"
{
	char* copiaSeguraPath = malloc(50);
	char* nombre = malloc(18);
	char nombrePadre[17];
	unsigned int i;
	uint16_t offsetBloquePadre;

	strcpy(copiaSeguraPath,path);

	//tengo que recorrer el arbol de directorios para saber si el path indicado es valido o no

	sacarNombre(copiaSeguraPath,nombre);  //aca tengo algo asi -->  /pokedex/ash/objetivos ,  algo.dat

	free(nombre); //limpio el nombre
	nombre = malloc(18);

	if(!strlen(copiaSeguraPath))
	{                                                       //si no queda nada, es porque está en el raiz
		log_info(logger,"La ruta '%s' es valida (directorio raiz)",path);
		free(nombre);
		free(copiaSeguraPath);
		return true;
	}
	else
	{
		sacarNombre(copiaSeguraPath,nombre);  //aca tengo el nombre del directorio, ahora lo busco en la tabla
		while(strlen(copiaSeguraPath))  //mientras queden rutas donde buscar...
		{
			for(i=0; i < 2047 && strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombre);i++)
				;
			if(i == 2048)
			{
				log_error(logger,"La ruta especificada no es válida. Nombre: '%s' inválido",nombre);
				return false;  //si llega a recorrer el array completo, significa que no lo encontro
			}
			else
			{
				offsetBloquePadre = estructuraAdministrativa.tablaArchivos[i].bloquePadre;
				strcpy(nombrePadre,(char*)estructuraAdministrativa.tablaArchivos[offsetBloquePadre].nombre);

				free(nombre);
				nombre = malloc(18);
				sacarNombre(copiaSeguraPath,nombre);

				if(strcmp(nombre,nombrePadre))
				{
					log_error(logger,"La ruta especificada no es válida. Nombre: '%s' inválido",nombre);
					free(nombre);
					free(copiaSeguraPath);
					return false;  //si llega a recorrer el array completo, significa que no lo encontro
				}
			}
		}
		log_info(logger,"La ruta '%s' es válida",path);
		free(nombre);
		free(copiaSeguraPath);
		return true;
	}
}

void guardarEstructuraEn(char* mapa)
{
	int peso,  //este es el tamaño de los campos, medidos en BYTES
		marcador = 0,   //fija el offset al inicio, sirve para saber DESDE donde escribir
		offset = 0;		//sirve para saber HASTA donde escribir
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
		//mapa[offset] = mander[offset];
		offset++;
	} //todo fijarse que se guarda cualquier cosa
	marcador = offset;
	free(mander);

					//ahora guardo la tabla de archivos
	peso = sizeof(osadaFile) * 2048;
	mander = malloc(peso);
	memcpy(mander,estructuraAdministrativa.tablaArchivos,peso);
	while(offset < (peso + marcador))
	{
		mapa[offset] = mander[offset];
		offset++;
	}
	marcador = offset;
	free(mander);

					//ahora guardo la tabla de asignaciones
	peso = estructuraAdministrativa.header.tamanioFS - estructuraAdministrativa.header.tamanioDatos;
	peso = peso - 1 - estructuraAdministrativa.header.tamanioBitmap - (sizeof(osadaFile) * 2048 / 64);  //total estructura - header - bitmap - tabla de archivos
	peso = peso * 64;  //lo paso a bytes
	mander = malloc(peso);
	memcpy(mander,estructuraAdministrativa.tablaAsignaciones,peso);
	while(offset < (peso + marcador))
	{
		mapa[offset] = mander[offset];
		offset++;
	}
	marcador = offset;
	free(mander);
}

void crearDirectorio(char* path,char* mapa)
{
	int i = 0;
	int offsetTablaAsignaciones = 0;
	int offset = 0;  //con esta recorro el bitmap
	char* nombreEfectivo = malloc(18);
	char* pathAuxiliar = malloc(50);


	strcpy(pathAuxiliar,path);
	if(comprobarPathValido(path))  //si el path es correcto
	{
		sacarNombre(pathAuxiliar,nombreEfectivo);   //ahora tengo el nombre que le quieren dar al directorio

		while((i < 2048) && estructuraAdministrativa.tablaArchivos[i].estado != '\0')
			i++;

		if(i == 2048)   //si llego a 2048, es porque no hay lugar en el array de archivos
			log_error(logger,"Tabla de archivos completamente ocupada");
		else
		{
			log_info(logger,"i = %d",i);
			estructuraAdministrativa.tablaArchivos[i].estado = '\2'; //es un directorio
			strcpy((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombreEfectivo);

					//ahora le asigno un bloque libre
			while(bitarray_test_bit(estructuraAdministrativa.punteroBitmap,offset))  //hasta que no encuentre un '0'...
				offset++;
			bitarray_set_bit(estructuraAdministrativa.punteroBitmap,offset); //y actualizo el bitmap
				//este es el offset global del bitmap, pero en realidad yo quiero el offset en la tablaAsignaciones

			offset = offset - (estructuraAdministrativa.header.tamanioFS - estructuraAdministrativa.header.tamanioDatos);
				//aca tengo el offset de la tabla, me dice que bloque de datos es el que tengo en esta posicion
			while(estructuraAdministrativa.tablaAsignaciones[offsetTablaAsignaciones] != 0) //va a parar cuando encuentre un lugar libre
				offsetTablaAsignaciones++;
				//aca se va a guardar, en la tabla de asignaciones el offset del bloque
			estructuraAdministrativa.tablaAsignaciones[offsetTablaAsignaciones] = offset;

			estructuraAdministrativa.tablaArchivos[i].bloqueInicial = offsetTablaAsignaciones;
					//por ultimo, guardo el offset de la tablaAsig en donde esta su primer bloque

			estructuraAdministrativa.tablaArchivos[i].tamanioArchivo = 0; //el directorio vacio no pesa nada
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

			log_info(logger,"estado: %u",estructuraAdministrativa.tablaArchivos[i].estado);
			log_info(logger,"nombre: %.17s",estructuraAdministrativa.tablaArchivos[i].nombre);
			log_info(logger,"bloquePadre: %d",estructuraAdministrativa.tablaArchivos[i].bloquePadre);
			log_info(logger,"tamaño: %d",estructuraAdministrativa.tablaArchivos[i].tamanioArchivo);
			log_info(logger,"fecha: %d",estructuraAdministrativa.tablaArchivos[i].fecha);
			log_info(logger,"bloqueInicial: %d",estructuraAdministrativa.tablaArchivos[i].bloqueInicial);

			free(nombreEfectivo);
			free(pathAuxiliar);

			guardarEstructuraEn(mapa);
		}
	}
}

void crearArchivo(char* pathYNombre,char* mapa)
{
	;
}
