/*
 * deadlock.h
 *
 *  Created on: 15/10/2016
 *      Author: utnso
 */

#ifndef DEADLOCK_H_
#define DEADLOCK_H_
#include "funciones.h"
#include <pkmn/battle.h>
#include <pkmn/factory.h>

t_log* logDeadlock;
int** pedidosMatriz;
int** asignadosMatriz;
int* recursosVector;
int* disponiblesVector;
int* algoritmoVector;
int* entrenadoresEnDeadlock;
int cantidadDeEntrenadoresEnDeadlock;
int cantidadDeEntrenadores;
int cantidadDePokemones;
int hayDeadlock;
t_pkmn_factory* fabrica;
t_pokemon* pokemonA;
t_pokemon* pokemonB;
t_pokemon* pokemonC;
t_pokemon* pokemonD;
t_pokemon* pokemonPerdedor;
t_list * mejoresPokemones;

void sumarPedidosMatriz(int indiceEntrenador, int indicePokenest);
void restarPedidosMatriz(int indiceEntrenador, int indicePokenest);
void sumarAsignadosMatriz(int indiceEntrenador, int indicePokenest);
void restarAsignadosMatriz(int indiceEntrenador, int indicePokenest);
void liberarRecursosEntrenador(int indiceEntrenador);
void inicializarMatrices();
void inicializarEntrenadorEnMatrices(int indice);
void agregarEntrenadorEnMatrices();
void inicializarVectores();
void noTieneAsignadosOPedidos();
void algoritmo();
void mostrarMatriz(int**);
void mostrarEntrenadoresEnDeadlock();
void inicializarAlgoritmoVector();
void batallaPokemon();
void crearPokemones();

#endif /* DEADLOCK_H_ */
