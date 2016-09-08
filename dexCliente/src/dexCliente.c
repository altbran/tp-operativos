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

int S_POKEDEX_CLIENTE;


static int f_getattr(const char *path/*ruta del archivo cuyos atributos deben ser retornados*/, struct stat *stbuf/*puntero a la estructura que contiene los atributos luego de terminar la ejecucion*/) {
	/*se llama a esta funcion cuando el sistema trata de obtener los atributos de un archivo*/
	int resultado = 0;
	enviarHeader(0/*socketDestino*/,privilegiosArchivo,sizeof(int));
	recibirTodo(S_POKEDEX_CLIENTE,&stbuf,sizeof(double));


	return resultado;
}

static int f_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi) {
	/*Se llama a esta funcion cuando el usuario trata de mostrar los archivos y directorios en un directorio especifico*/
	int resultado = 0;
	recibirTodo(S_POKEDEX_CLIENTE,&buf,sizeof(double));

	return resultado;
}

static int f_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	/*Se llama a esta funcion cuando el sistema trata de leer un pedazo de data de un archivo*/
	recibirTodo(S_POKEDEX_CLIENTE,&buf,size);
	return 0;
}

static struct fuse_operations ejemplo_oper = { .readdir = f_readdir,
		.getattr = f_getattr, .read = f_read, };

int main(int argc, char *argv[]) {
		char *PUERTOSTR = getenv("PUERTO_POKEDEX_SERVIDOR");
		int PUERTO_POKEDEX_SERVIDOR = atoi(PUERTOSTR);
		char *IP_POKEDEX_SERVIDOR = getenv("IP_POKEDEX_SERVIDOR");

		//me conecto al proceso servidor
		if (crearSocket(&S_POKEDEX_CLIENTE)) {
			printf("Error creando socket\n");
			return 1;
		}

		if (conectarA(S_POKEDEX_CLIENTE, IP_POKEDEX_SERVIDOR, PUERTO_POKEDEX_SERVIDOR)) {
			printf("Error al conectar\n");
			return 1;
		}

		if (responderHandshake(S_POKEDEX_CLIENTE, IDPOKEDEXCLIENTE, IDPOKEDEXSERVER)) {
			return 1;
		}

	return fuse_main(argc, argv, &ejemplo_oper);

}

