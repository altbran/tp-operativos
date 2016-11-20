#include "funciones.h"
#include "dibujador.h"

void cargarPokenest(t_metadataPokenest pokenest){
	CrearCaja(items, pokenest.identificador, pokenest.posicionX, pokenest.posicionY, pokenest.cantidad);
}

void cargarEntrenador(t_datosEntrenador entrenador){
	CrearPersonaje(items, entrenador.identificador, entrenador.posicionX, entrenador.posicionY);
}

void restarPokemon(char identificador){
	restarRecurso(items,identificador);
}

void sumarPokemon(char identificador){
	sumarRecurso(items,identificador);
}


void moverEntrenador(t_datosEntrenador entrenador){
	MoverPersonaje(items, entrenador.identificador, entrenador.posicionX, entrenador.posicionY);
}

void eliminarEntrenador(char identificador){
	BorrarItem(items, identificador);
}

void crearItems(){
	items = list_create();
	int rows, cols;
	int q, p;

	nivel_gui_inicializar();

	nivel_gui_get_area_nivel(&rows, &cols);

	p = cols;
	q = rows;

}

void dibujar(char* nombreMapa) {
/*
	int rows, cols;
	int q, p;

	nivel_gui_inicializar();

	nivel_gui_get_area_nivel(&rows, &cols);

	p = cols;
	q = rows;

	CrearPersonaje(items, '@', p, q);
*/


	nivel_gui_dibujar(items, nombreMapa);

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
