#include "funciones.h"

int main(int argc, char **argv) {

	//Creo log para el mapa

	logger = log_create("Mapa.log", "MAPA", 0, log_level_from_string("INFO"));
	printf("%d", argc);

	//busco las configuraciones
	if (argc != 3) {
		ruta = concat(4, "/home/utnso/tp-2016-2c-A-cara-de-rope/mapa", "/Mapas/", "Paleta", "/");
		log_info(logger, "La ruta es: ", ruta);
		nombreMapa = malloc(sizeof(argv[1]));
		nombreMapa = "Paleta";
	} else {
		ruta = concat(4, argv[2], "/Mapas/", argv[1], "/");
		log_info(logger, "La ruta es: ", ruta);
		nombreMapa = malloc(sizeof(argv[1]));
		nombreMapa = argv[1];
	}

	//inicializo el mutex
	pthread_mutex_init(&mutex, NULL);

	//inicio colas
	listos = queue_create();
	bloqueados = queue_create();

	//cargo recursos de mapa
	cargarMetadata();
	crearItems();
	cargarRecursos();

	//inicializo hilos
	iniciarPlanificador();
	//pthread_create(&deadlock,NULL,(void*)detectarDeadlock,NULL); //todo falta el semaforo!!!
	pthread_create(&atrapadorPokemon, NULL, (void*) atraparPokemon, NULL); //todo falta semaforo tmb!!
	log_info(logger, "Arranque hilo atrapador");

	//reconocer señales SIGUSR2
	signal(SIGUSR2, receptorSIG);

	//creo socket servidor
	if (crearSocket(&servidorMapa)) {
		printf("Error creando socket");
		return 1;
	}
	if (escucharEn(servidorMapa, configuracion->puerto)) {
		printf("Error al conectar");
		log_error(logger, "Se produjo un error creando el socket servidor", texto);
		return 1;
	}

	log_info(logger, "Se estableció correctamente el socket servidor", texto);
	log_info(logger, "Escuchando nuevas conexiones");

	int nuevaConexion;
	struct sockaddr_in direccionCliente;

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
	dibujar(nombreMapa);
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
						t_datosEntrenador * entrenador = malloc(sizeof(t_datosEntrenador));
						int * socketNuevo = malloc(sizeof(int));
						*socketNuevo = nuevaConexion;

						//recibir datos del entrenador nuevo
						if (recibirEntrenador(*socketNuevo, entrenador)) {
							log_info(logger, "error en el recibir entrenador, socket %d", *socketNuevo);
						} else {
							list_add(Entrenadores, &entrenador);
						}
						//envio la posicion de la pokenest
						char identificador;
						if (recibirTodo(*socketNuevo, &identificador, sizeof(char))) {
							log_info(logger, "error al recibir identificador pokenest, socket %d", *socketNuevo);
						} else {
							t_metadataPokenest * pokenest = devolverPokenest(&identificador);
							enviarCoordPokenest(*socketNuevo, pokenest);
						}
						if (recibirHeader(*socketNuevo) == entrenadorListo) { //me fijo cuando el entrenador esta listo para agregarlo a la lista de listos
							recibirTodo(*socketNuevo, &entrenador->distanciaAPokenest, sizeof(int));
							queue_push(listos, &socketNuevo);
							dibujar(nombreMapa);
							log_info(logger, "Nuevo entrenador conectado, socket %d", *socketNuevo);
						} else {
							log_info(logger, "error en el listo al conectar entrenador, socket %d", *socketNuevo);
							list_remove(Entrenadores,devolverIndiceEntrenador(*socketNuevo));
							free(socketNuevo);
							free(entrenador);
						}
						break;
					default:
						close(nuevaConexion);
						log_error(logger, "Error en el handshake. Conexion inesperada", texto);
						break;
					}

				}
			}
		}
	}

}
