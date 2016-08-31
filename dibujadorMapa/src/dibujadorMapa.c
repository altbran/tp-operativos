#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <tad_items.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <nivel.h>
#include <unistd.h>
#include <src/sockets.h>
#include <src/structs.h>

int main(int argc, char **argv) {
	int socket_mapa;
	//me conecto al mapa
	if (crearSocket(&socket_mapa)) {
		printf("Error creando socket\n");
		return 1;
	}
	/*if (conectarA(socket_mapa, IP_MAPA, PUERTO_MAPA)) {
		printf("Error al conectar\n");
		return 1;
	}*/
	if (responderHandshake(socket_mapa, IDDIBUJADORMAPA, IDMAPA)) {
		return 1;
	}

	t_metadataPokenest pokenest1;
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

	t_list* items = list_create();

	int rows, cols;
	int q, p;

	nivel_gui_inicializar();

	nivel_gui_get_area_nivel(&rows, &cols);

	p = cols;
	q = rows;

	CrearPersonaje(items, '@', p, q);
	CrearPersonaje(items, '#', 10, 10);

	CrearCaja(items, pokenest1.identificador, pokenest1.posicionX,
			pokenest1.posicionY, 5);
	CrearCaja(items, pokenest2.identificador, pokenest2.posicionX,
			pokenest2.posicionY, 3);
	CrearCaja(items, pokenest3.identificador, pokenest3.posicionX,
			pokenest3.posicionY, 2);

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
