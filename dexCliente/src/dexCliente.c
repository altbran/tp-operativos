#define FUSE_USE_VERSION 25
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <fuse.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <src/sockets.h>
#include <src/protocolo.h>
#include <src/structs.h>
#include <pthread.h>

int S_POKEDEX_CLIENTE;
pthread_mutex_t mutex;

void enviarPath(const char *path, int socketDestino) {
	void *buffer = malloc(50);
	memcpy(buffer, path,50);
	send(socketDestino, buffer, 50, 0);
	free(buffer);
}

static int f_getattr(const char *path, struct stat *stbuf) {
	/*se llama a esta funcion cuando el sistema trata de obtener los atributos de un archivo*/

	t_privilegiosArchivo privilegios;

	pthread_mutex_lock(&mutex);
	int fecha;

	enviarHeader(S_POKEDEX_CLIENTE, privilegiosArchivo);
	enviarPath(path, S_POKEDEX_CLIENTE);

	fecha = recibirHeader(S_POKEDEX_CLIENTE);

	privilegios.esDir = recibirHeader(S_POKEDEX_CLIENTE);
	privilegios.tamanio = recibirHeader(S_POKEDEX_CLIENTE);

	printf("GETTATR de:   %s\n",path);
	printf("Dir: %d   Tam: %d\n",privilegios.esDir,privilegios.tamanio);

	if (privilegios.esDir == 1)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		stbuf->st_size = 0;
		memcpy(&stbuf->st_mtim,&fecha,sizeof(int));
		pthread_mutex_unlock(&mutex);
		return 0;
	}
	else
	{
		if(privilegios.esDir == 0)
		{
			stbuf->st_mode = S_IFREG | 0777;
			stbuf->st_nlink = 1;
			stbuf->st_size = privilegios.tamanio;
			memcpy(&stbuf->st_mtim,&fecha,sizeof(int));
			pthread_mutex_unlock(&mutex);
			return 0;
		}
		else
		{
		printf("ENOENT: %d\n",-ENOENT);
		pthread_mutex_unlock(&mutex);
		return -ENOENT;
		}
		pthread_mutex_unlock(&mutex);
		return 0;		//este está para que no hinche las bolas
	}
}

static int f_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi)
{
	char* cadenaARecibir;
	int header;
	int i = 0;

	pthread_mutex_lock(&mutex);

	enviarHeader(S_POKEDEX_CLIENTE, contenidoDirectorio);
	enviarPath(path, S_POKEDEX_CLIENTE);

	header = recibirHeader(S_POKEDEX_CLIENTE);
	printf("READDIR:  %s.  Header: %d\n",path,header);

	if(header)
	{
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);

		cadenaARecibir = malloc(18);

		header = recibirHeader(S_POKEDEX_CLIENTE);

		while(i < header)
		{
			recv(S_POKEDEX_CLIENTE, cadenaARecibir, 18,0);
			printf("Nombre recibido: %s\n",cadenaARecibir);

			filler(buf, cadenaARecibir, NULL, 0);
			free(cadenaARecibir);
			cadenaARecibir = malloc(18);
			i++;
		}

		free(cadenaARecibir);
		pthread_mutex_unlock(&mutex);
		return 0;
	}
	else
	{
		pthread_mutex_unlock(&mutex);
		return -ENOENT;
	}
}

static int f_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	pthread_mutex_lock(&mutex);
	void* cadenaARecibir;
	int cantidadBytesARecibir;

	enviarHeader(S_POKEDEX_CLIENTE, contenidoArchivo);
	enviarPath(path, S_POKEDEX_CLIENTE);

	cantidadBytesARecibir = recibirHeader(S_POKEDEX_CLIENTE);

	printf("\nREAD:  %s\n",path);
	printf("Bytes recibidos: %d.    Offset: %d.   Size: %d\n",cantidadBytesARecibir,offset,size);

	if(cantidadBytesARecibir == -1)
	{
		pthread_mutex_unlock(&mutex);
		return -1;
	}else
	{
		cadenaARecibir = malloc(cantidadBytesARecibir);

		printf("\n......va a recibir cosas\n");

		if(!recibirTodo(S_POKEDEX_CLIENTE, cadenaARecibir, cantidadBytesARecibir))
		{
			printf(" - - - - - acaba de recibir cosas, deberia entrar al memcpy\n");
			memcpy(buf,(cadenaARecibir + offset), size);
			printf(" - - - - si no pasa por aca, es porque hay un error\n");
		}

		free(cadenaARecibir);

		pthread_mutex_unlock(&mutex);
		return size;
	}

}

static int f_write(const char *path, const void *buffer, size_t size,off_t offset, struct fuse_file_info *fi)
{
	pthread_mutex_lock(&mutex);

	enviarHeader(S_POKEDEX_CLIENTE, escribirEnFichero);

	enviarPath(path, S_POKEDEX_CLIENTE);
	enviarHeader(S_POKEDEX_CLIENTE,offset);
	enviarHeader(S_POKEDEX_CLIENTE,size);

	send(S_POKEDEX_CLIENTE, buffer, size, 0);

	printf("WRITE:  %s\n",path);
	printf("Size: %d\n",size);

	int res = recibirHeader(S_POKEDEX_CLIENTE);

	pthread_mutex_unlock(&mutex);

	switch(res)
	{
		case 0:
			return size;
		case -1:
			return -1;
		case -2:
			return EFBIG;
	}
	return 0; //asi no rompe las bolas
}

static int f_crearCarpeta(const char *path, mode_t modo) {

	pthread_mutex_lock(&mutex);

	enviarHeader(S_POKEDEX_CLIENTE, crearCarpeta);
	enviarPath(path,S_POKEDEX_CLIENTE);

	int res = recibirHeader(S_POKEDEX_CLIENTE);
	pthread_mutex_unlock(&mutex);
	switch(res)
	{
		case -1:
			return -1;
		case -2:
			return ENAMETOOLONG;
		case -3:
			return EDQUOT;
		case 0:
			return 0;
		default:
			return -1;
	}
}

static int f_unlink(const char *path) {
	pthread_mutex_lock(&mutex);

	int res;

	enviarHeader(S_POKEDEX_CLIENTE, eliminarArchivo);
	enviarPath(path, S_POKEDEX_CLIENTE);
	printf("UNLINK: %s\n",path);
	res = recibirHeader(S_POKEDEX_CLIENTE);

	pthread_mutex_unlock(&mutex);

	return res;
}

static int f_open(const char *path, struct fuse_file_info *fi) {

	pthread_mutex_lock(&mutex);

	int res;

	enviarHeader(S_POKEDEX_CLIENTE, abrirArchivo);
	enviarPath(path, S_POKEDEX_CLIENTE);
	printf("OPEN: %s\n",path);

	res = recibirHeader(S_POKEDEX_CLIENTE);    //0 OK, -1 NO OK

	pthread_mutex_unlock(&mutex);

	return res;
}

static int f_close(const char *path, struct fuse_file_info *fi) {
	pthread_mutex_lock(&mutex);

	int res;

	enviarHeader(S_POKEDEX_CLIENTE, cerrarArchivo);
	enviarPath(path, S_POKEDEX_CLIENTE);
	printf("CLOSE: %s\n",path);

	res = recibirHeader(S_POKEDEX_CLIENTE);    //0 OK, -1 NO OK

	pthread_mutex_unlock(&mutex);

	return res;
}

static int f_rename(const char *pathAntiguo, const char *pathNuevo)
{
	pthread_mutex_lock(&mutex);

	enviarHeader(S_POKEDEX_CLIENTE,renombrarCosas);

	enviarPath(pathAntiguo,S_POKEDEX_CLIENTE);
	enviarPath(pathNuevo,S_POKEDEX_CLIENTE);

	printf("RENAME:     %s    por        %s\n",pathAntiguo,pathNuevo);

	int res = recibirHeader(S_POKEDEX_CLIENTE);

	pthread_mutex_unlock(&mutex);

	switch(res)
	{
		case 0:
			return 0;
		case -1:
			return -1;
		case -2:
			return ENAMETOOLONG;
		default:
			return -1;
	}
}

static int f_removerDirectorio(const char *path, mode_t modo) {
	pthread_mutex_lock(&mutex);

	enviarHeader(S_POKEDEX_CLIENTE, removerDirectorio);
	enviarPath(path, S_POKEDEX_CLIENTE);

	int res = recibirHeader(S_POKEDEX_CLIENTE);

	printf("RMDIR:  %s\n",path);

	pthread_mutex_unlock(&mutex);

	switch(res)
	{
		case 0:
			return 0;
		case -1:
			return -1;
		case -2:
			return ENOTEMPTY;
	}
	return 0;  //asi no jode
}

static int f_crearArchivo(const char *path,  mode_t modo, dev_t dev) {

	int res;

	pthread_mutex_lock(&mutex);

	enviarHeader(S_POKEDEX_CLIENTE, crearFichero);
	enviarPath(path, S_POKEDEX_CLIENTE);

	res = recibirHeader(S_POKEDEX_CLIENTE);

	printf("MKNOD:  %s\n",path);

	pthread_mutex_unlock(&mutex);

	switch(res)
	{
		case 1:
			return 0;
		case -1:
			return -1;
		case -2:
			return ENAMETOOLONG;
		case -3:
			return EDQUOT;
	}
	return 0;	//idem a los anteriores, me rompe las bolas el switch
}

static int f_truncate (const char* path,off_t size)
{
	pthread_mutex_lock(&mutex);

	enviarHeader(S_POKEDEX_CLIENTE,truncarArchivo);
	enviarPath(path,S_POKEDEX_CLIENTE);
	enviarHeader(S_POKEDEX_CLIENTE,size);

	int res = recibirHeader(S_POKEDEX_CLIENTE);

	printf("TRUNCATE:  %s     a %d bytes\n",path,size);

	pthread_mutex_unlock(&mutex);

	return res;
}

static struct fuse_operations ejemplo_oper = {
		.readdir = f_readdir,
		.getattr = f_getattr,
		.read = f_read,
		.write = f_write,
		.rename = f_rename,
		.unlink = f_unlink,
		.mkdir = f_crearCarpeta,
		.open = f_open,
		.rmdir = f_removerDirectorio,
		.release = f_close,
		.mknod = f_crearArchivo,
		.truncate = f_truncate,
};

int main(int argc, char *argv[])
{
	char *PUERTOSTR = getenv("PUERTO_POKEDEX_SERVIDOR");
	int PUERTO_POKEDEX_SERVIDOR = atoi(PUERTOSTR);
	char *IP_POKEDEX_SERVIDOR = getenv("IP_POKEDEX_SERVIDOR");

	//me conecto al proceso servidor
	if (crearSocket(&S_POKEDEX_CLIENTE))
		printf("Error creando socket\n");

	if (conectarA(S_POKEDEX_CLIENTE, IP_POKEDEX_SERVIDOR, PUERTO_POKEDEX_SERVIDOR))
		printf("Error al conectar\n");

	if (responderHandshake(S_POKEDEX_CLIENTE, IDPOKEDEXCLIENTE, IDPOKEDEXSERVER))
		printf("Error, id no esperado\n");

	return fuse_main(argc, argv, &ejemplo_oper);
}

