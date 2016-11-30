#include "protocolo.h"
#include "sockets.h"
#include "structs.h"
int enviarHeader(int socketDestino, int header) {
	int hayError;
	if ((hayError = send(socketDestino, &header, sizeof(int), 0)) <= 0) {
		return 0;
	} else {
		return 1;
	}
}

int recibirHeader(int socketOrigen) {
	int header;
	int bytesRecibidos;
	if ((bytesRecibidos = recv(socketOrigen, &header, sizeof(int), 0)) <= 0) {
		return 0;
	} else {
		return header;
	}
}
