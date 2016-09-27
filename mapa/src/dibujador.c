#include "funciones.h"
#include "dibujador.h"

void cargarPokenest(t_metadataPokenest pokenest){
	CrearCaja(items, pokenest.identificador, pokenest.posicionX, pokenest.posicionY, pokenest.cantidad);
}

void cargarEntrenador(t_metadataEntrenador entrenador){
	CrearPersonaje(items, entrenador.simbolo, 1, 1);
}

void restarPokemon(char identificador){
	restarRecurso(items,identificador);
}

void dibujar() {

	int rows, cols;
	int q, p;

	nivel_gui_inicializar();

	nivel_gui_get_area_nivel(&rows, &cols);

	p = cols;
	q = rows;

	CrearPersonaje(items, '@', p, q);



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
