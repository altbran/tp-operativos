#include "funciones.h"

void dibujar() {

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
}
