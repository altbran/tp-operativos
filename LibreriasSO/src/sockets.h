#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>

enum {

	IDERROR = 0, IDMAPA = 1, IDENTRENADOR = 2, IDPOKEDEXCLIENTE = 3, IDPOKEDEXSERVER = 4, IDDIBUJADORMAPA = 5
};

int crearSocket(int *unSocket);
int escucharEn(int unSocket, int puerto);
int conectarA(int unSocket, char* ip, int puerto);
int iniciarHandshake(int socketDestino, uint8_t idOrigen);
int responderHandshake(int socketDestino, uint8_t idOrigen, uint8_t idEsperado);
int aceptarConexion(int socketServidor, struct sockaddr_in * direccionCliente);
int recibirTodo(int socketOrigen,void * buffer, int largo);
int enviarTodo(int socketDestino, void * buffer, int largo);
