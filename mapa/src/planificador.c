#include <src/funciones.h>

int main(int argc, char **argv) {


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

							log_info(logger, "Nueva entrenador conectado, socket %d", nuevaConexion);
							//Maneja consola

							break;
						default:
							close(nuevaConexion);
							log_error(logger, "Error en el handshake. Conexion inesperada", texto);
							break;
						}

					}
				}
				return 0;
			}
		}
		return 0;
}

