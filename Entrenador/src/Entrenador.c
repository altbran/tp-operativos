/*
 ============================================================================
 Name        : Entrenador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>


#define PACKAGESIZE 1024

int main(){

	struct addrinfo serverAux;
	struct addrinfo *serverInfo;

	memset(&serverAux, 0, sizeof(serverAux));
	serverAux.ai_family = AF_INET;
	serverAux.ai_socktype = SOCK_STREAM;

	char ip[20];
	char puerto[7];

	printf("Ingrese el ip al que se desea conectar: ");
	scanf("%s", ip);
	printf("Ingrese el puerto del servidor: ");
	scanf("%s", puerto);

	getaddrinfo(ip, puerto, &serverAux, &serverInfo);	// Carga en serverInfo los datos de la conexion

	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if (serverSocket == -1){
		printf("Hubo un error al crear el socket servidor");
		return 1;
	}

	if (-1 == connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)){
		printf("No se pudo establecer conexion con el serivdor");
		return 1;
	}
	freeaddrinfo(serverInfo);


	int enviar = 1;
	char message[PACKAGESIZE];

	printf("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");

	while(enviar){
		fgets(message, PACKAGESIZE, stdin);
		if (!strcmp(message,"exit\n")) enviar = 0;
		if (enviar) {
			int envio = send(serverSocket, message, strlen(message) + 1, 0);
					if (envio == -1){
						printf("Error al enviar mensaje");
					}
		}
	}
}
