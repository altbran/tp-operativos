#include "funciones.h"

void receptorSIG() {
	//todo
	pthread_mutex_lock(&mutex);
	cargarMetadata();
	pthread_cancel(planificador);
	iniciarPlanificador();
	pthread_mutex_unlock(&mutex);
}

void iniciarPlanificador() {
	if(configuracion->algoritmo == 'R'){
		pthread_create(&planificador,NULL,(void*)roundRobin(),NULL);
	}else{
		pthread_create(&planificador,NULL,(void*) sjfs(),NULL);
	}

}
//funciones entrenador

int recibirEntrenador(int socketOrigen,t_datosEntrenador *entrenador) {
	int i= 0;
	i = recibirTodo(socketOrigen, &entrenador->nombre, 18);
	i += recibirTodo(socketOrigen, &entrenador->identificador, 1);
	i += recibirTodo(socketOrigen, &entrenador->posicionX, sizeof(uint32_t));
	i += recibirTodo(socketOrigen, &entrenador->posicionY, sizeof(uint32_t));

	entrenador->socket = socketOrigen;
	cargarEntrenador(*entrenador);
	dibujar(nombreMapa);
	return i;
}

t_datosEntrenador devolverEntrenador(int socket) {
	int i;
	for (i = 0; i < list_size(Entrenadores); i++) {
		t_datosEntrenador entrenador = *(t_datosEntrenador*) (list_get(Entrenadores, i));
		if (entrenador.socket == socket) {
			return entrenador;
		}
	}
}

int devolverIndiceEntrenador(int socket) {
	int i;
	for (i = 0; i < list_size(Entrenadores); i++) {
		t_datosEntrenador entrenador = *(t_datosEntrenador*) (list_get(Entrenadores, i));
		if (entrenador.socket == socket) {
			return i;
		}
	}
}

int movimientoValido(int socket, int posX, int posY) {
	t_datosEntrenador entrenador = devolverEntrenador(socket);
	int i = entrenador.posicionX - posX + entrenador.posicionY - posY;
	if (i == 1 || i == -1) {
		entrenador.posicionX = posX;
		entrenador.posicionY = posY;
		list_replace(Entrenadores, devolverIndiceEntrenador(socket), &entrenador);
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}

//funciones mapa
void cargarMetadata() {
	t_config * config = config_create(concat(2, ruta, "metadata"));
	configuracion = malloc(sizeof(t_metadataMapa));
	configuracion->tiempoChequeoDeadlock = config_get_int_value(config, "TiempoChequeoDeadlock");
	configuracion->batalla = config_get_int_value(config, "Batalla");
	configuracion->algoritmo = *(config_get_string_value(config, "algoritmo"));
	configuracion->quantum = config_get_int_value(config, "quantum");
	configuracion->retardo = config_get_int_value(config, "retardo");
	//configuracion.ip = config_get_string_value(config, "IP");
	strcpy(configuracion->ip,config_get_string_value(config, "IP"));
	configuracion->puerto = config_get_int_value(config, "Puerto");
}

//funciones de pokenest

void cargarRecursos() {
	Pokenests = list_create();
	recursosTotales = list_create();
	pokemones = list_create();
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(concat(2, ruta, "Pokenests/"))) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
				// do nothing (straight logic)
			} else {
				t_metadataPokenest pokenest = malloc(sizeof(t_metadataPokenest));
				char ** tokens;
				t_config * config = config_create(concat(4, ruta, "Pokenests/", ent->d_name, "/metadata"));
				pokenest.identificador = *(config_get_string_value(config, "Identificador"));
				pokenest.tipo = config_get_string_value(config, "Tipo");
				tokens = str_split(config_get_string_value(config, "Posicion"), ';');
				pokenest.posicionX = atoi(*tokens);
				pokenest.posicionY = atoi(*(tokens + 1));
				int cantidad = malloc(sizeof(int));
				cantidad = contadorDePokemon(concat(4, ruta, "Pokenests/", ent->d_name, "/"));
				pokenest.cantidad = cantidad;
				int i;
				for(i=1;i<=cantidad;i++){
					t_pokemon pokemon = malloc(sizeof(t_pokemon));
					pokemon.socketEntrenador = -1;
					pokemon.identificadorPokemon = pokenest.identificador;
					pokemon.numeroPokemon = i;
					list_add(pokemones,&pokemon);
				}
				list_add(recursosTotales, &cantidad);
				list_add(Pokenests, &pokenest);
				cargarPokenest(pokenest);
			}
		}
		closedir(dir);
	} else {
		/* could not open directory */
		perror("");
		log_info(logger, "no se pudo abrir el directorio");
		//return EXIT_FAILURE;
	}
}

int contadorDePokemon(char * directorio) {
	int file_count = 0;
	DIR * dirp;
	struct dirent * entry;

	dirp = opendir(directorio); /* There should be error handling after this */
	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_type == DT_REG) { /* If the entry is a regular file */
			file_count++;
		}
	}
	closedir(dirp);
	return file_count - 1; //descarto el archivo metadata
}

int pokemonDisponible(int indicePokenest, char identificador,int * numeroPokemon, int * indice) {
	if (*((int*)list_get(recursosTotales, indicePokenest)) >= 1) {
		int i;
		for(i=0;i<list_size(pokemones);i++){
			t_pokemon pokemon = list_get(pokemones,i);
			if(pokemon.identificadorPokemon == identificador && pokemon.socketEntrenador == -1){
				numeroPokemon = pokemon.numeroPokemon;
				i = list_size(pokemones);
			}
		}
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}

void restarRecursoDisponible(int indicePokenest) {
	int cantidad;
	cantidad = *((int*)list_get(recursosTotales, indicePokenest)) - 1;
	list_replace(recursosTotales, indicePokenest, &cantidad);
}

t_metadataPokenest devolverPokenest(char * identificador) {
	int i;
	for (i = 0; i < list_size(Pokenests); i++) {
		t_metadataPokenest pokenest = *(t_metadataPokenest*) (list_get(Pokenests, i));
		if (pokenest.identificador == *identificador) {
			return pokenest;
		}
	}
}

int devolverIndicePokenest(char * identificador) {
	int i;
	for (i = 0; i < list_size(Pokenests); i++) {
		t_metadataPokenest pokenest = *(t_metadataPokenest*) (list_get(Pokenests, i));
		if (pokenest.identificador == *identificador) {
			return i;
		}
	}
}

int enviarCoordPokenest(int socketDestino, t_metadataPokenest * pokenest) {

	void *buffer = malloc(sizeof(int) + sizeof(int)); //posx + posy

	//envio header
	enviarHeader(socketDestino,enviarDatosPokenest);

	//serializo contenido
	int cursorMemoria = 0;
	memcpy(buffer, &pokenest->posicionX, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(buffer + cursorMemoria, &pokenest->posicionY, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);

	int i = enviarTodo(socketDestino,buffer,cursorMemoria);
	//send(socketDestino, buffer, cursorMemoria, 0); //hay que serializar algo acá?
	free(buffer);
	return i;
}

char* concat(int count, ...) {
	va_list ap;
	int i;

	// Find required length to store merged string
	int len = 1; // room for NULL
	va_start(ap, count);
	for (i = 0; i < count; i++)
		len += strlen(va_arg(ap, char*));
	va_end(ap);

	// Allocate memory to concat strings
	char *merged = calloc(sizeof(char), len);
	int null_pos = 0;

	// Actually concatenate strings
	va_start(ap, count);
	for (i = 0; i < count; i++) {
		char *s = va_arg(ap, char*);
		strcpy(merged + null_pos, s);
		null_pos += strlen(s);
	}
	va_end(ap);

	return merged;
}

char** str_split(char* a_str, const char a_delim) {
	char** result = 0;
	size_t count = 0;
	char* tmp = a_str;
	char* last_comma = 0;
	char delim[2];
	delim[0] = a_delim;
	delim[1] = 0;

	/* Count how many elements will be extracted. */
	while (*tmp) {
		if (a_delim == *tmp) {
			count++;
			last_comma = tmp;
		}
		tmp++;
	}

	/* Add space for trailing token. */
	count += last_comma < (a_str + strlen(a_str) - 1);

	/* Add space for terminating null string so caller
	 knows where the list of returned strings ends. */
	count++;

	result = malloc(sizeof(char*) * count);

	if (result) {
		size_t idx = 0;
		char* token = strtok(a_str, delim);

		while (token) {
			assert(idx < count);
			*(result + idx++) = strdup(token);
			token = strtok(0, delim);
		}
		assert(idx == count - 1);
		*(result + idx) = 0;
	}

	return result;
}
