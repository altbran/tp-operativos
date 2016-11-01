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

int S_POKEDEX_CLIENTE;

void enviarPath(const char *path, int socketDestino) {

	void *buffer = malloc(50);
	memcpy(buffer, path,50);
	send(socketDestino, buffer, 50, 0);
	free(buffer);
}

static int f_getattr(const char *path, struct stat *stbuf) {
	/*se llama a esta funcion cuando el sistema trata de obtener los atributos de un archivo*/

	int resultado = 0;
	t_privilegiosArchivo privilegios;

	enviarHeader(S_POKEDEX_CLIENTE, privilegiosArchivo);
	enviarPath(path, S_POKEDEX_CLIENTE);

	printf("Path enviado. getattr: %s\n",path);

	privilegios.esDir = recibirHeader(S_POKEDEX_CLIENTE);//recibirTodo(S_POKEDEX_CLIENTE,&privilegios.esDir,sizeof(int));
	privilegios.tamanio = recibirHeader(S_POKEDEX_CLIENTE);//recibirTodo(S_POKEDEX_CLIENTE,&privilegios.tamanio,sizeof(uint32_t));

	if (privilegios.esDir) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if(privilegios.esDir == 0){
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = privilegios.tamanio;
	}else {
		resultado = -ENOENT;
	}

	printf("Atributos recibidos para: %s\n",path);
	printf("Dir: %d   Tam: %d\n",privilegios.esDir,privilegios.tamanio);

	//Agregar caso fallo
	return resultado;
}

static int f_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi)
{
	char* cadenaARecibir;
	int header;
	int i = 0;

	enviarHeader(S_POKEDEX_CLIENTE, contenidoDirectorio);
	enviarPath(path, S_POKEDEX_CLIENTE);

	printf("Path enviado readdir: %s\n",path);

	header = recibirHeader(S_POKEDEX_CLIENTE);
	printf("Header recibido para el readdir: %d   PATH: %s\n",header,path);

	if(header)
	{
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);

		cadenaARecibir = malloc(18);

		header = recibirHeader(S_POKEDEX_CLIENTE);

		while(i < header)
		{
			recv(S_POKEDEX_CLIENTE, cadenaARecibir, 18,0);
			printf("Readdir. Cadena recibida: %s\n",cadenaARecibir);

			filler(buf, cadenaARecibir, NULL, 0);
			free(cadenaARecibir);
			cadenaARecibir = malloc(18);
			i++;
		}

		free(cadenaARecibir);
		return 0;
	}
	else
	{
		return -ENOENT;
	}




	/*int cantidadArchivos = recibirHeader(S_POKEDEX_CLIENTE);
	switch (cantidadArchivos) {
	case 0:
		break;
	case -1:
		resultado = -ENOENT;
		break;
	default:
		for (i = 0; i < cantidadArchivos; i++) {
			cantidadBytesARecibir = recibirHeader(S_POKEDEX_CLIENTE);
			cadenaARecibir = malloc(18);
			recibirTodo(S_POKEDEX_CLIENTE, cadenaARecibir,
					cantidadBytesARecibir);
			filler(buf, cadenaARecibir, NULL, 0);
			free(cadenaARecibir);
		}
	}*/
}

static int f_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	/*Se llama a esta funcion cuando el sistema trata de leer un pedazo de data de un archivo*/
	char* cadenaARecibir;
	int cantidadBytesARecibir;

	enviarHeader(S_POKEDEX_CLIENTE, contenidoArchivo);
	enviarPath(path, S_POKEDEX_CLIENTE);

	cantidadBytesARecibir = recibirHeader(S_POKEDEX_CLIENTE);
	cadenaARecibir = malloc(cantidadBytesARecibir);

	recibirTodo(S_POKEDEX_CLIENTE, cadenaARecibir, cantidadBytesARecibir);/*recibo bytes*/
	memcpy(buf, cadenaARecibir + offset, size);

	return size;
}

static int f_write(const void *buffer, const char *path, size_t size,
		off_t offset, struct fuse_file_info *fi) {
	int cursorMemoria;
	int longitud;

	enviarHeader(S_POKEDEX_CLIENTE, escribirEnFichero);
	enviarPath(path, S_POKEDEX_CLIENTE);
	longitud = strlen(path);
	void *buf = malloc(sizeof(longitud) + sizeof(buffer));
	memcpy(buf, &path, sizeof(longitud));
	cursorMemoria += sizeof(longitud);
	memcpy(buf + cursorMemoria, &buffer, sizeof(buffer));
	send(S_POKEDEX_CLIENTE, buf, sizeof(buf), 0);
	free(buf);
	return size;
}

static int f_crearCarpeta(const char *nombreFichero, mode_t modo) {
	int res;

	enviarHeader(S_POKEDEX_CLIENTE, crearCarpeta);
	void *buffer = malloc(sizeof(const char*));
	memcpy(buffer, &nombreFichero, sizeof(const char*));
	send(S_POKEDEX_CLIENTE, buffer, sizeof(buffer), 0);
	free(buffer);
	recibirTodo(S_POKEDEX_CLIENTE,&res,sizeof(int));//resultado de la operacion
	if (res){
		return 0;
	}else{
		return -1;
	}

}

static int f_unlink(const char *path) {
	int res;
	enviarHeader(S_POKEDEX_CLIENTE, eliminarArchivo);
	enviarPath(path, S_POKEDEX_CLIENTE);
	recibirTodo(S_POKEDEX_CLIENTE,&res,sizeof(int));//resultado de la operacion
		if (res){
			return 0;
		}else{
			return -1;
		}
	return 0;
}

static int f_open(const char *path, struct fuse_file_info *fi) {
	int res;
	enviarHeader(S_POKEDEX_CLIENTE, abrirArchivo);
	enviarPath(path, S_POKEDEX_CLIENTE);
	recibirTodo(S_POKEDEX_CLIENTE,&res,sizeof(int));//resultado de la operacion
		if (res){
			return 0;
		}else{
			return -1;
		}
}

static int f_close(const char *path, struct fuse_file_info *fi) {
	enviarHeader(S_POKEDEX_CLIENTE, cerrarArchivo);
	enviarPath(path, S_POKEDEX_CLIENTE);
	return 0;
}

static int f_rename(const char *pathAntiguo, const char *pathNuevo)
{
	t_cambioDeDirectorios estructura;
	estructura.pathAntiguo = pathAntiguo;
	estructura.pathNuevo = pathNuevo;
	int cursorMemoria = 0;

	void *buffer = malloc(sizeof(const char*) + sizeof(const char*)); //path viejo + path nuevo
	memcpy(buffer, &estructura.pathAntiguo, sizeof(const char*));
	cursorMemoria += sizeof(const char*);
	memcpy(buffer + cursorMemoria, &estructura.pathNuevo, sizeof(const char*));
	send(S_POKEDEX_CLIENTE, buffer, sizeof(buffer), 0);
	free(buffer);

	return 0;
}

static int f_removerDirectorio(const char *path) {
	enviarHeader(S_POKEDEX_CLIENTE, removerDirectorio);
	enviarPath(path, S_POKEDEX_CLIENTE);
	return 0;
}

static struct fuse_operations ejemplo_oper = { .readdir = f_readdir,
		.getattr = f_getattr,
		.read = f_read,
		.write = f_write,
		.rename = f_rename,
		.unlink = f_unlink,
		.mkdir = f_crearCarpeta,
		.open = f_open,
		.rmdir = f_removerDirectorio,
		.release = f_close,
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

