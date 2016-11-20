/*
 * planificadores.h
 *
 *  Created on: 30/10/2016
 *      Author: utnso
 */

#ifndef PLANIFICADORES_H_
#define PLANIFICADORES_H_

void srdf();
void roundRobin();
void hiloPokenest(void * parametros);
int entrenadorMasCercano(int * movimientos);
void jugada(int miTurno, int * quedoBloqueado, int * i, int total);

#endif /* PLANIFICADORES_H_ */
