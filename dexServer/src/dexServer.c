#include <stdio.h>
#include <string.h>
#include <stdlib.h>			 //
#include <stdbool.h>         //
#include <time.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <src/sockets.h>
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
bool comprobarPathValidoLectura(char* path);
void recorrerDesdeIzquierda(char* path, char* nombre);
void guardarEstructuraEn(char* mapa);
void* leerArchivo(char* pathSolicitado,t_estructuraAdministrativa est,char* mapa);
void crearDirectorio(char* path,char* mapa);
void crearArchivo(char* path,char* mapa);
void borrarArchivo(char* path,char* mapa);
void borrarDirectorioVacio(char* path,char* mapa);
void renombrar(char* pathOriginal, char* pathNuevo, char* mapa);

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
	char* archivoMapeado;

	archivoMapeado = getenv("PUERTO_POKEDEX_SERVIDOR");        //la uso temporalmente, para evitar aumentar el heap
	PUERTO_POKEDEX_SERVIDOR = atoi(archivoMapeado);

	log_info(logger, "PUERTO_POKEDEX_SERVIDOR = %d",PUERTO_POKEDEX_SERVIDOR);


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

	log_info(logger, "Se creó correctamente el socket servidor. Escuchando nuevas conexiones");

	//crearDirectorio("/entrenador",archivoMapeado);
	//crearDirectorio("/pokedex",archivoMapeado);
	//crearDirectorio("/entrenador/juan",archivoMapeado);

	//crearArchivo("/entrenador/juan/algo.dat",archivoMapeado);
	//crearArchivo("/pokedex/ruperto.dat",archivoMapeado);

	//renombrar("/entrenador/juan/algo.dat","/entrenador/juan/esquivelForro.dat",archivoMapeado);

	//borrarArchivo("/entrenador/juan/algo.dat",archivoMapeado);
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
	free(archivoMapeado);

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
	void* archivoLeido = "";

	strcpy(pathSolicitado,path);

	if(comprobarPathValidoLectura(pathSolicitado))
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

bool comprobarPathValidoLectura(char* path)   // ejemplo:  "/pokedex/ash/objetivos/algo.dat"
{
	char* copiaSeguraPath;
	char* pathCopiado = malloc(50);
	char* nombre = malloc(50);
	char nombreCopia[18];
	char nombrePadre[18];
	unsigned int i;
	uint16_t offsetBloquePadre = 0;

	if(!strlen(path))
	{
		log_info(logger,"La ruta es válida, directorio raiz",path);
		return true;
	}
	else
	{
		strcpy(pathCopiado,path);
		do
		{
			free(nombre);
			nombre = malloc(50);
			sacarNombre(pathCopiado,nombre);  //aca tengo algo asi -->  /pokedex/ash/objetivos ,  algo.dat

			i = 0;
			while((i < 2048) && (offsetBloquePadre != 0xFFFF))
			{
				if(!strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombre)) //si encuentra un arc/dir que se llama asi
				{
					offsetBloquePadre = estructuraAdministrativa.tablaArchivos[i].bloquePadre;

					if(offsetBloquePadre != 0xFFFF)
					{
						if(strlen(pathCopiado))   //verifico solo para que no me de seg.fault
						{
							copiaSeguraPath = malloc(50);
							strcpy(copiaSeguraPath,pathCopiado);
							strcpy(nombrePadre,(char*)estructuraAdministrativa.tablaArchivos[offsetBloquePadre].nombre);
							sacarNombre(copiaSeguraPath,nombreCopia);  //aca saco una copia del nombre padre provisto por path

							if(!strcmp(nombrePadre,nombreCopia)) //si efectivamente es el dir padre, rompo el bucle
							{
								free(copiaSeguraPath);
								break;
							}
							free(copiaSeguraPath);
						}
					}
				}
				i++;
			}

			if(i == 2048)  //si llega a recorrer el array completo, significa que no encontró algun dir. padre
			{
				log_error(logger,"La ruta especificada no existe. Ruta: %s",path);
				return false;
			}
		}while(offsetBloquePadre != 0xFFFF);

		log_info(logger,"La ruta '%s' es válida",path);
		free(pathCopiado);
		free(nombre);
		return true;
	}
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

void crearDirectorio(char* path,char* mapa)
{
	int i = 0;
	int offset = 0;  //con esta recorro el bitmap
	char* nombreEfectivo = malloc(18);
	char* pathAuxiliar = malloc(50);


	strcpy(pathAuxiliar,path);
	sacarNombre(pathAuxiliar,nombreEfectivo);   //ahora tengo el nombre que le quieren dar al directorio

	if(comprobarPathValidoLectura(pathAuxiliar))  //si el path es correcto
	{
		while((i < 2048) && estructuraAdministrativa.tablaArchivos[i].estado != '\0')
			i++;

		if(i == 2048)   //si llego a 2048, es porque no hay lugar en el array de archivos
			log_error(logger,"Tabla de archivos completamente ocupada");
		else
		{
			estructuraAdministrativa.tablaArchivos[i].estado = '\2'; //es un directorio
			strcpy((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombreEfectivo);

					//ahora le asigno un bloque libre
			while(bitarray_test_bit(estructuraAdministrativa.punteroBitmap,offset))  //hasta que no encuentre un '0'...
				offset++;
			bitarray_set_bit(estructuraAdministrativa.punteroBitmap,offset); //y actualizo el bitmap
				//este es el offset global del bitmap, pero en realidad yo quiero el offset en la tablaAsignaciones

			offset = offset - (estructuraAdministrativa.header.tamanioFS - estructuraAdministrativa.header.tamanioDatos);
				//aca tengo el offset de la tabla, me dice que bloque de datos es el que tengo en esta posicion
			estructuraAdministrativa.tablaAsignaciones[offset] = 0xFFFFFFFF;
				//voy a la tabla en ese offset, y como es unico, le pongo 0xFFFFFF

			estructuraAdministrativa.tablaArchivos[i].bloqueInicial = offset;
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

			free(nombreEfectivo);
			free(pathAuxiliar);

			guardarEstructuraEn(mapa);
			log_info(logger,"Directorio creado. Nombre: '%s'",path);
		}
	}
}

void crearArchivo(char* path,char* mapa)
{
	int i = 0;
	int offset = 0;  //con esta recorro el bitmap
	char* nombreEfectivo = malloc(18);
	char* pathAuxiliar = malloc(50);


	strcpy(pathAuxiliar,path);
	sacarNombre(pathAuxiliar,nombreEfectivo);   //ahora tengo el nombre que le quieren dar al archivo

	if(comprobarPathValidoLectura(pathAuxiliar))  //si el path es correcto
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

void borrarArchivo(char* path,char* mapa)
{
	int i = 0;
	int offset = 0;
	int bloqueInicioDatos;
	char* nombreEfectivo = malloc(18);
	char* pathAuxiliar = malloc(50);

	strcpy(pathAuxiliar,path);
	if(comprobarPathValidoLectura(path))  //si el path es correcto
	{
		sacarNombre(pathAuxiliar,nombreEfectivo);   //ahora tengo el nombre del archivo a borrar

		while((i < 2048) && strcmp((char*)estructuraAdministrativa.tablaArchivos[i].nombre,nombreEfectivo))
			i++;

		if(i == 2048)   //si llego a 2048, es porque no lo encuentra
			log_error(logger,"El archivo '%s' a eliminar no existe",path);
		else
		{
			estructuraAdministrativa.tablaArchivos[i].estado = '\0'; //lo borra
			strcpy((char*)estructuraAdministrativa.tablaArchivos[i].nombre,"");
			estructuraAdministrativa.tablaArchivos[i].bloquePadre = 0xFFFF;
			estructuraAdministrativa.tablaArchivos[i].fecha = 0;

					//ahora le tengo que borrar los bloques ocupados
			bloqueInicioDatos = estructuraAdministrativa.header.tamanioFS - estructuraAdministrativa.header.tamanioDatos;
			offset = estructuraAdministrativa.tablaArchivos[i].bloqueInicial;
			while(offset != 0xFFFFFFFF)
			{
				bitarray_clean_bit(estructuraAdministrativa.punteroBitmap,(offset + bloqueInicioDatos)); //limpio el bit
				offset = estructuraAdministrativa.tablaAsignaciones[offset]; //paso al siguiente bloque
			}
				//una vez que limpie el bitmap, termino de acondicionar la estructura de ese archivo eliminado
			estructuraAdministrativa.tablaArchivos[i].bloqueInicial = 0;
			estructuraAdministrativa.tablaArchivos[i].tamanioArchivo = 0;

			free(nombreEfectivo);
			free(pathAuxiliar);

			guardarEstructuraEn(mapa);
			log_info(logger,"Archivo eliminado. Nombre '%s'",path);
		}
	}
}

void borrarDirectorioVacio(char* path,char* mapa)
{
	int i = 0;
	int offset = 0;
	int bloqueInicioDatos;
	char* nombreEfectivo = malloc(18);
	char* pathAuxiliar = malloc(50);

	strcpy(pathAuxiliar,path);
	if(comprobarPathValidoLectura(path))  //si el path es correcto
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
		}
	}
}
