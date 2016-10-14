#include "protocolo.h"
#include "sockets.h"
#include "structs.h"
void enviarHeader(int socketDestino, int header) {
	send(socketDestino, &header, sizeof(int), 0);
}
void enviarPath(const char *path, int socketDestino) {
	void *buffer = malloc(sizeof(path));
	strcpy(buffer, path);
	send(socketDestino, buffer, sizeof(buffer), 0);
	free(buffer);
}

int recibirHeader(int socketOrigen) {
	int header;
	int bytesRecibidos;
	if ((bytesRecibidos = recv(socketOrigen, &header, sizeof(int), 0)) <= 0) {
		return bytesRecibidos;
	} else {
		return header;
	}
}
