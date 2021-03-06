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
t_log * logers;
int** pedidosMatriz;
int** asignadosMatriz;
int** pedidosMatrizClonada;
int** asignadosMatrizClonada;
int* recursosVector;
int* disponiblesVector;
int* algoritmoVector;
int* vectorAuxiliar;
int* entrenadoresEnDeadlock;
int cantidadDeEntrenadoresEnDeadlock;
int cantidadDeEntrenadores;
int cantidadDeEntrenadoresClonada;
int cantidadDePokemones;
bool hayDeadlock;
bool batallaActivada;
t_pkmn_factory* fabrica;
t_pokemon* pokemonA;
t_pokemon* pokemonB;
t_pokemon* pokemonC;
t_pokemon* pokemonD;
t_list * entrenadoresEnDeadlockLista;
t_list * entrenadoresClonada;
t_list * mejoresPokemones;

void detectarDeadlock();
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
void resolverDeadlock();
t_pokemon* batallaPokemon(t_pokemon* pkmnA,t_pokemon* pkmnB, int indiceA, int indiceB);
void crearPokemones();
void notificarDeadlockAEntrenador(int indice);
void notificarResultadoBatalla(int indice, bool gano);
void resolverDeadlockAMiManera();
int obtenerPrimerEntrenadorEnDeadlock();
void inicalizarEntrenadoresEnDeadlock();
int traerIndiceEntrenadorPerdedor(int indice);
void cargarMatrices();
void cargarDisponiblesVector();
int notificarGanadoresEntrenadores(int indiceDeEntrenadorPerdedor);
void copiarEntrenadores();

#endif /* DEADLOCK_H_ */
