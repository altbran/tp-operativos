#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <nivel.h>
#include <unistd.h>


int main(int argc, char **argv) {

		t_list* items = list_create();

		int rows, cols;
		int q, p;

		int x = 1;
		int y = 1;

		nivel_gui_inicializar();
	//	printf("ghola");
		nivel_gui_get_area_nivel(&rows, &cols);

		p = cols;
		q = rows;

		CrearPersonaje(items, '@', p, q);
		CrearPersonaje(items, '#', 20, 20);

		CrearCaja(items, 'H', 26, 10, 5);
		CrearCaja(items, 'M', 8, 15, 3);
		CrearCaja(items, 'F', 19, 9, 2);

		nivel_gui_dibujar(items, "Test Chamber 04");

		/*BorrarItem(items, '#');
			BorrarItem(items, '@');

			BorrarItem(items, '1');
			BorrarItem(items, '2');

			BorrarItem(items, 'H');
			BorrarItem(items, 'M');
			BorrarItem(items, 'F');
	*/
			//nivel_gui_terminar();


		return EXIT_SUCCESS;
}
