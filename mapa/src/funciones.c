#include "funciones.h"

void receptorSIG() {
	llegoSenial = 1;
	pthread_mutex_lock(&mutex);
	cargarMetadata();
	pthread_mutex_unlock(&mutex);
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
			t_metadataPokenest pokenest;
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
			//todo list_add(Pokenests,pokenest);
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
