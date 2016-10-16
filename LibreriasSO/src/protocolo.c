#include "protocolo.h"
#include "sockets.h"
#include "structs.h"
void enviarHeader(int socketDestino, int header) {
	send(socketDestino, &header, sizeof(int), 0);
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
