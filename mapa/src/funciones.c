#include "funciones.h"

void receptorSIG() {
	//todo
	pthread_mutex_lock(&mutex);
	cargarMetadata();
	pthread_mutex_unlock(&mutex);
}

t_posicionEntrenador recibirEntrenador(int socketOrigen){
	t_posicionEntrenador entrenador;
	recibirTodo(socketOrigen,&entrenador.identificador,sizeof(char));
	recibirTodo(socketOrigen,&entrenador.posicionX,sizeof(uint32_t));
	recibirTodo(socketOrigen,&entrenador.posicionY,sizeof(uint32_t));
	cargarEntrenador(entrenador);
	return entrenador;
}


void cargarMetadata() {
	t_config * config = config_create(concat(2, ruta, "metadata"));
	configuracion.tiempoChequeoDeadlock = config_get_int_value(config, "TiempoChequeoDeadlock");
	configuracion.batalla = config_get_int_value(config, "Batalla");
	configuracion.algoritmo = config_get_string_value(config, "algoritmo");
	configuracion.quantum = config_get_int_value(config, "quantum");
	configuracion.retardo = config_get_int_value(config, "retardo");
	configuracion.ip = config_get_string_value(config, "IP");
	configuracion.puerto = config_get_int_value(config, "Puerto");
}

void cargarRecursos() {
	Pokenests = list_create();
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(concat(2, ruta, "Pokenests/"))) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			t_metadataPokenest  pokenest;
			t_config * config = config_create(concat(4, ruta, "Pokenests/", ent->d_name,"/metadata"));
			pokenest.identificador = config_get_string_value(config, "Identificador");
			pokenest.tipo = config_get_string_value(config, "Tipo");
			char * posicion = strtok(config_get_string_value(config, "Posicion"), ";");
			//todo revisar el while
			while (posicion != NULL) {
				pokenest.posicionX = strdup(posicion);
				posicion = strtok(NULL, ";");
				pokenest.posicionY = strdup(posicion);
			}
			pokenest.cantidad = contadorDePokemon(concat(4, ruta, "Pokenests/", ent->d_name,"/"));
			list_add(Pokenests,&pokenest);
			cargarPokenest(pokenest);
		}
		closedir(dir);
	} else {
		/* could not open directory */
		perror("");
		//return EXIT_FAILURE;
	}
}

int contadorDePokemon(char * directorio){
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
	return file_count-1; //descarto el archivo metadata
}

t_metadataPokenest devolverPokenest(char identificador){
	//todo
	int i;
	for(i = 0; i < list_size(Pokenests); i++){
		t_metadataPokenest pokenest = list_get(Pokenests,i);
		if(pokenest.identificador == identificador){
			return pokenest;
		}else {return false;}
	}

}

void enviarCoordPokenest(int socketDestino, t_metadataPokenest pokenest) {

	void *buffer = malloc(sizeof(int) + sizeof(int)); //posx + posy

	int cursorMemoria = 0;

	memcpy(buffer, &pokenest.posicionX, sizeof(uint32_t));
	cursorMemoria += sizeof(uint32_t);
	memcpy(buffer + cursorMemoria, &pokenest.posicionY, sizeof(uint32_t));

	send(socketDestino, buffer, 21, 0); //hay que serializar algo acá?
	free(buffer);
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
