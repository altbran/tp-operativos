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

static int ejemplo_getattr(const char *path, struct stat *stbuf) {
	int res = 0;

	return res;
}

static int ejemplo_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi) {
	int res = 0;


	return res;
}

static int ejemplo_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {

	return 0;
}

static struct fuse_operations ejemplo_oper = { .readdir = ejemplo_readdir,
		.getattr = ejemplo_getattr, .read = ejemplo_read, };

int main(int argc, char *argv[]) {

		int PUERTO_POKEDEX_SERVIDOR = getenv("PUERTO_POKEDEX_SERVIDOR");
		int IP_POKEDEX_SERVIDOR = getenv("IP_POKEDEX_SERVIDOR");

		//me conecto al proceso servidor
		int S_POKEDEX_CLIENTE;
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


		int header = recibirHeader(S_POKEDEX_CLIENTE);



	return fuse_main(argc, argv, &ejemplo_oper);

}

