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
	void *buffer = malloc(sizeof(path));
	strcpy(buffer, path);
	send(socketDestino, buffer, sizeof(buffer), 0);
	free(buffer);
}

static int f_getattr(
		const char *path/*ruta del archivo cuyos atributos deben ser retornados*/,
		struct stat *stbuf/*puntero a la estructura que contiene los atributos luego de terminar la ejecucion*/) {
	/*se llama a esta funcion cuando el sistema trata de obtener los atributos de un archivo*/

	int resultado = 0;
	t_privilegiosArchivo privilegios;
	enviarHeader(S_POKEDEX_CLIENTE, privilegiosArchivo);
	enviarPath(path, S_POKEDEX_CLIENTE);
	recibirTodo(S_POKEDEX_CLIENTE,&privilegios.esDir,sizeof(int));
	recibirTodo(S_POKEDEX_CLIENTE,&privilegios.tamanio,sizeof(uint32_t));
	if (privilegios.esDir) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = privilegios.tamanio;
	}
	//Agregar caso fallo
	return resultado;
}

static int f_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi) {
	/*Se llama a esta funcion cuando el usuario trata de mostrar los archivos y directorios en un directorio especifico*/
	int resultado = 0;
	int i;
	int cantidadBytesARecibir;
	char* cadenaARecibir;

	enviarHeader(S_POKEDEX_CLIENTE, contenidoDirectorio);
	enviarPath(path, S_POKEDEX_CLIENTE);

	int cantidadArchivos = recibirHeader(S_POKEDEX_CLIENTE);
	switch (cantidadArchivos) {
	case 0:
		break;
	default:
		for (i = 0; i < cantidadArchivos; i++) {
			cantidadBytesARecibir = recibirHeader(S_POKEDEX_CLIENTE);
			cadenaARecibir = malloc(18);
			recibirTodo(S_POKEDEX_CLIENTE, cadenaARecibir,
					cantidadBytesARecibir);/*Me lo manda X veces?*/
			filler(buf, cadenaARecibir, NULL, 0);
			free(cadenaARecibir);
		}
	}
	return resultado;
}

static int f_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	/*Se llama a esta funcion cuando el sistema trata de leer un pedazo de data de un archivo*/
	char* cadenaARecibir;
	int cantidadBytesARecibir;

	enviarHeader(S_POKEDEX_CLIENTE, contenidoArchivo);
	enviarPath(path, S_POKEDEX_CLIENTE);
	cadenaARecibir = malloc(18);
	cantidadBytesARecibir = recibirHeader(S_POKEDEX_CLIENTE);
	recibirTodo(S_POKEDEX_CLIENTE, cadenaARecibir, cantidadBytesARecibir);/*recibo bytes*/
	memcpy(buf, (char*) cadenaARecibir + offset, size);

	return size;
}

static int f_write(const char *path, const char *path2, size_t size,
		off_t offset, struct fuse_file_info *fi) {

	return 0;
}

static int f_rename(const char *pathAntiguo, const char *pathNuevo) {
	t_cambioDeDirectorios estructura;
	estructura.pathAntiguo = pathAntiguo;
	estructura.pathNuevo = pathNuevo;
	int cursorMemoria = 0;

	void *buffer = malloc(sizeof(const char*) + sizeof(const char*)); //posx + posy
	memcpy(buffer, &estructura.pathAntiguo, sizeof(const char*));
	cursorMemoria += sizeof(const char*);
	memcpy(buffer + cursorMemoria, &estructura.pathNuevo, sizeof(const char*));
	send(S_POKEDEX_CLIENTE, buffer, sizeof(buffer), 0);
	free(buffer);

	return 0;
}
static struct fuse_operations ejemplo_oper = { .readdir = f_readdir, .getattr =
		f_getattr, .read = f_read, .write = f_write, .rename = f_rename, };

int main(int argc, char *argv[]) {
	char *PUERTOSTR = getenv("PUERTO_POKEDEX_SERVIDOR");
	int PUERTO_POKEDEX_SERVIDOR = atoi(PUERTOSTR);
	char *IP_POKEDEX_SERVIDOR = getenv("IP_POKEDEX_SERVIDOR");

	//me conecto al proceso servidor
	if (crearSocket(&S_POKEDEX_CLIENTE)) {
		printf("Error creando socket\n");
		return 1;
	}

	if (conectarA(S_POKEDEX_CLIENTE, IP_POKEDEX_SERVIDOR,
			PUERTO_POKEDEX_SERVIDOR)) {
		printf("Error al conectar\n");
		return 1;
	}

	if (responderHandshake(S_POKEDEX_CLIENTE, IDPOKEDEXCLIENTE,
			IDPOKEDEXSERVER)) {
		return 1;
	}

	return fuse_main(argc, argv, &ejemplo_oper);

}

