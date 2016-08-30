/*
 * Dibujador.c
 *
 *  Created on: 4/9/2016
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <tad_items.h>
#include <curses.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <nivel.h>

int main(int argc, char** argv){

	  	t_list* items = list_create();

		int rows, cols;
		int q, p;

		int x = 1;
		int y = 1;
		nivel_gui_inicializar();

	    nivel_gui_get_area_nivel(&rows, &cols);

		p = cols;
		q = rows;

		CrearPersonaje(items, '@', p, q);
		CrearPersonaje(items, '#', x, y);

		CrearCaja(items, 'H', 26, 10, 5);
		CrearCaja(items, 'M', 8, 15, 3);
		CrearCaja(items, 'F', 19, 9, 2);

		nivel_gui_dibujar(items, "Test Chamber 04");

		nivel_gui_terminar();

	return EXIT_SUCCESS;
	}

