#include "protocolo.h"
#include "sockets.h"
#include "structs.h"
void enviarHeader(int socketDestino, int header, int largo) {
	send(socketDestino, &header, sizeof(int), 0);
}
void enviarPokenestDibujador(int socketDestino, t_metadataPokenest pokenest, int largo) {
	int header = datosInicialesMapa;
	send(socketDestino, &header, sizeof(int), 0);
	//send(socketDestino, &largoCodigo, sizeof(int), 0);
	void *buffer = malloc(sizeof(char[12]) + sizeof(int) + sizeof(int) + 1); //tipo + posx + posy + identif

	//strlen(pokenest.tipo)
	int cursorMemoria = 0;

	strcpy(buffer, pokenest.tipo);
	cursorMemoria += sizeof(char[12]);
	memcpy(buffer + cursorMemoria, &pokenest.posicionX, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(buffer + cursorMemoria, &pokenest.posicionY, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(buffer + cursorMemoria, &pokenest.identificador,sizeof(char));
	//cursorMemoria += strlen(pokenest.identificador);

	send(socketDestino, buffer, 21, 0); //hay que serializar algo acá?
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
