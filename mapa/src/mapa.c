#include "funciones.h"

int main(int argc, char **argv) {
	//inicializo el mutex
	pthread_mutex_init(&mutex,NULL);
	//busco las configuraciones
	int PUERTO_MAPA_SERVIDOR = getenv("PUERTO_MAPA_SERVIDOR");

	//Creo log para el mapa

	logger = log_create("Mapa.log", "MAPA", 0, log_level_from_string("INFO"));

	//me intento conectar a la PokeDex Cliente SOY HOST
	/*
	 if (crearSocket(&clientePokeDex)) {
	 printf("Error creando socket\n");
	 log_error(logger, "Se produjo un error creando el socket de PokeDex", texto);
	 return 1;
	 }
	 if (conectarA(clientePokeDex, IP_POKEDEX, PUERTO_POKEDEX)) {
	 printf("Error al conectar\n");
	 log_error(logger, "Se produjo un error conectandose a la PokeDex", texto);
	 return 1;
	 }
	 log_info(logger, "Se establecio la conexion con la PokeDex");

	 if (responderHandshake(clientePokeDex, IDMAPA, IDPOKEDEXCLIENTE)) {
	 log_error(logger, "Error en el handshake", texto);
	 return 1;
	 }
	 */
	//creo el hilo para reconocer señales SIGUSR2

	signal(SIGUSR2, receptorSIG);

	//creo socket servidor
	if (crearSocket(&servidorMapa)) {
		printf("Error creando socket");
		return 1;
	}
	if (escucharEn(servidorMapa, PUERTO_MAPA_SERVIDOR)) {
		printf("Error al conectar");
		log_error(logger, "Se produjo un error creando el socket servidor", texto);
		return 1;
	}

	log_info(logger, "Se estableció correctamente el socket servidor", texto);
	log_info(logger, "Escuchando nuevas conexiones");


	int nuevaConexion;
	struct sockaddr_in direccionCliente;

	//listaConsolas = list_create();
	//listaFinalizacionesPendientes = list_create();

	// temp file descriptor list for select()
	int listener = servidorMapa;     // listening socket descriptor

	FD_ZERO(&bolsaDeSockets);    // clear the master and temp sets
	FD_ZERO(&bolsaAuxiliar);

	FD_SET(listener, &bolsaDeSockets);
	//FD_SET(inotify, &bolsaDeSockets);

	int fdmax;        // maximum file descriptor number
	/*
	 if (listener > inotify) {
	 fdmax = listener;
	 } else {
	 fdmax = inotify;
	 }
	 */
	fdmax = listener; //lo agregue para que no marque error pero es lo de arriba
	int i;

	while (1) {
		bolsaAuxiliar = bolsaDeSockets;
		if (select(fdmax + 1, &bolsaAuxiliar, NULL, NULL, NULL) == -1) {
			perror("select");
			return 1;
		}

		// run through the existing connections looking for data to read
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &bolsaAuxiliar)) { // we got one!!
				if (i == listener) {
					// handle new connections
					//Espera hasta que el hilo haya guardado el valor que se le paso como parametro
					//antes de sobreEscribir la variable nuevaConexion
					nuevaConexion = aceptarConexion(servidorMapa, &direccionCliente);

					if (nuevaConexion > fdmax) {
						fdmax = nuevaConexion;
					}

					int idRecibido = iniciarHandshake(nuevaConexion, IDMAPA);

					switch (idRecibido) {

					case IDERROR:
						log_info(logger, "Se desconecto el socket", texto);
						close(nuevaConexion);
						break;

					case IDENTRENADOR:

						FD_SET(nuevaConexion, &bolsaDeSockets);

						log_info(logger, "Nuevo entrenador conectado, socket %d", nuevaConexion);
						//Maneja consola

						break;
						/*
					case IDDIBUJADORMAPA:

						FD_SET(nuevaConexion, &bolsaDeSockets);
						t_metadataPokenest pk;
						char prueba[12] = "pika";
						pk.identificador = '$';
						pk.posicionX = 10;
						pk.posicionY = 10;
						strcpy(pk.tipo,prueba);
						enviarPokenestDibujador(nuevaConexion,pk,21);


						log_info(logger, "Dibujador conectado, socket %d", nuevaConexion);

						break;
						*/
					default:
						close(nuevaConexion);
						log_error(logger, "Error en el handshake. Conexion inesperada", texto);
						break;
					}

				} else {
					/*
					int header = recibirHeader(i);

					switch (header) {

					case dimensionesMapa:
						break;

					}
					*/
				}
			}
		}
	}

}
