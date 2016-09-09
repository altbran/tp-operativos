#include <stdlib.h>
#include <curses.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <nivel.h>
#include <unistd.h>
#include <src/sockets.h>
#include <src/structs.h>
#include <commons/config.h>
#include <src/protocolo.h>

int main(int argc, char **argv) {

	//busco las configuraciones
	t_config * config;
	if (argc != 2) {
		//printf("Número incorrecto de parámetros\n");
		//return -1;
		config = config_create("./Configuracion/config");
		char * ruta = "./Configuracion/config";
		argv[1] = ruta;
	} else {

		config = config_create(argv[1]);
	}

	int PUERTO_MAPA = config_get_int_value(config, "PUERTO_MAPA");
	char* IP_MAPA = config_get_string_value(config, "IP_MAPA");

	//int PUERTO_MAPA = 8080;
	//char* IP_MAPA = "127.0.0.1";

	//me conecto al proceso mapa
	int clienteMapa;
	if (crearSocket(&clienteMapa)) {
		printf("Error creando socket\n");
		return 1;
	}
	if (conectarA(clienteMapa, IP_MAPA, PUERTO_MAPA)) {
		printf("Error al conectar\n");
		return 1;
	}

	if (responderHandshake(clienteMapa, IDDIBUJADORMAPA, IDMAPA)) {
		return 1;
	}




	int header = recibirHeader(clienteMapa);

	t_metadataPokenest pk;

	switch (header){

		case datosInicialesMapa :
			void  buffer = malloc(21);
			recibirTodo(clienteMapa, buffer, 21);
			int cursorMemoria = 0;


			memcpy(pk.tipo, buffer, sizeof(char[12]));
			cursorMemoria += sizeof(char[12]);
			memcpy(pk.posicionX, buffer + cursorMemoria, sizeof(uint32_t));
			cursorMemoria += sizeof(uint32_t);
			memcpy(pk.posicionY, buffer + cursorMemoria, sizeof(uint32_t));
			cursorMemoria += sizeof(uint32_t);
			memcpy(pk.identificador, buffer + cursorMemoria, sizeof(char));
			cursorMemoria += sizeof(char);

			break;
	}



	printf("hola");

	/*t_metadataPokenest pokenest1;
	t_metadataPokenest pokenest2;
	t_metadataPokenest pokenest3;

	pokenest1.posicionX = 3;
	pokenest1.posicionY = 3;
	pokenest2.posicionX = 6;
	pokenest2.posicionY = 6;
	pokenest3.posicionX = 9;
	pokenest3.posicionY = 9;
	pokenest1.identificador = 'A';
	pokenest2.identificador = 'B';
	pokenest3.identificador = 'C';
*/
	t_list* items = list_create();

	int rows, cols;
	int q, p;

	nivel_gui_inicializar();

	nivel_gui_get_area_nivel(&rows, &cols);

	p = cols;
	q = rows;

	CrearPersonaje(items, '@', p, q);
	CrearPersonaje(items, '#', 10, 10);

	/*CrearCaja(items, pokenest1.identificador, pokenest1.posicionX,
			pokenest1.posicionY, 5);
	CrearCaja(items, pokenest2.identificador, pokenest2.posicionX,
			pokenest2.posicionY, 3);
	CrearCaja(items, pokenest3.identificador, pokenest3.posicionX,
			pokenest3.posicionY, 2);
*/

	CrearCaja(items, pk.identificador, pk.posicionX, pk.posicionY, 10);

	nivel_gui_dibujar(items, "Test Chamber 04");

	/*	BorrarItem(items, '#');
	 BorrarItem(items, '@');

	 BorrarItem(items, '1');
	 BorrarItem(items, '2');

	 BorrarItem(items, 'H');
	 BorrarItem(items, 'M');
	 BorrarItem(items, 'F');

	 //nivel_gui_terminar();

	 */

	return EXIT_SUCCESS;
}
