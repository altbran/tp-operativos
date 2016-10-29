/*
 * deadlock.c
 *
 *  Created on: 15/10/2016
 *      Author: utnso
 */

#include "deadlock.h"

void detectarDeadlock() {


	logDeadlock = log_create("Deadlock.log", "deadlock", 0, log_level_from_string("INFO"));

	sleep(configuracion.tiempoChequeoDeadlock / 1000);

	hayDeadlock = EXIT_FAILURE;

	cantidadDeEntrenadores = list_size(Entrenadores);
	cantidadDePokemones = list_size(Pokenests);

	inicializarMatrices();
	inicializarVectores();

	inicializarAlgoritmoVector();

	noTieneAsignadosOPedidos();

	algoritmo();

	if(hayDeadlock == EXIT_SUCCESS){
		log_info(logDeadlock, "Hay deadlock");
		log_info(logDeadlock, "Matriz de asignados");
		mostrarMatriz(asignadosMatriz);
		log_info(logDeadlock, "Matriz de pedidos");
		mostrarMatriz(pedidosMatriz);
		log_info(logDeadlock, "Entrenadores en deadlock");
		mostrarEntrenadoresEnDeadlock();

		resolverDeadlock();
	}

}

void inicializarMatrices() {
	int i;
	pedidosMatriz = (int **) malloc(cantidadDeEntrenadores * sizeof(int*));
	asignadosMatriz = (int **) malloc(cantidadDeEntrenadores * sizeof(int*));
	for (i = 0; i < cantidadDeEntrenadores; i++){
		inicializarEntrenadorEnMatrices(i);
	}
}

void inicializarEntrenadorEnMatrices(int indice){
	pedidosMatriz[indice] = (int *) malloc(cantidadDePokemones * sizeof(int));
	asignadosMatriz[indice] = (int *) malloc(cantidadDePokemones * sizeof(int));
}

void agregarEntrenadorEnMatrices(){
	pedidosMatriz = (int **) realloc(pedidosMatriz, cantidadDeEntrenadores * sizeof(int*));
	asignadosMatriz = (int **) realloc(asignadosMatriz, cantidadDeEntrenadores * sizeof(int*));
	inicializarEntrenadorEnMatrices(cantidadDeEntrenadores - 1);

}

void inicializarVectores() {
	disponiblesVector = calloc(cantidadDePokemones, sizeof(int*));
	algoritmoVector = calloc(cantidadDePokemones, sizeof(int*));
	recursosVector = calloc(cantidadDePokemones, sizeof(int*));
	entrenadoresEnDeadlock = calloc(cantidadDeEntrenadores, sizeof(int*));
}

void inicializarAlgoritmoVector(){
	int b;
	for(b=0;b<cantidadDePokemones;b++){
		algoritmoVector[b] = disponiblesVector[b];
	}
}

void mostrarMatriz(int** matriz) {
	int i;
	int j;
	char* vector = calloc(cantidadDePokemones, sizeof(int*));
	for (i = 0; i < cantidadDeEntrenadores; i++) {
		for(j=0; j<cantidadDePokemones; j++){
			vector[j] = matriz[i][j] + '0';
		}
		log_info(logDeadlock,"%s" ,vector);

	}
}

void mostrarEntrenadoresEnDeadlock(){
	int p;
	for(p=0;p<cantidadDeEntrenadores;p++){
		if(entrenadoresEnDeadlock[p] == 0){
			//log_info(logDeadlock,"%s", list_get(Entrenadores,p)->nombre);
			notificarDeadlockAEntrenador(p);
			log_info(logDeadlock,"%d", p);
		}
	}
}

void notificarDeadlockAEntrenador(int indice){
	t_datosEntrenador* entrenador;
	entrenador = malloc(sizeof(t_datosEntrenador));
	entrenador = (t_datosEntrenador*) (list_get(Entrenadores, indice));
	enviarHeader(entrenador->socket,notificarDeadlock);
	free(entrenador);
}

void noTieneAsignadosOPedidos(){
	int tienePedido;
	int tieneAsignado;
	int j;
	int i;
	for(i = 0; i < cantidadDeEntrenadores; i++){
		j = 0;
		tienePedido = EXIT_FAILURE;
		tieneAsignado = EXIT_FAILURE;
		while(j < cantidadDePokemones && (tienePedido ==  EXIT_FAILURE|| tieneAsignado ==  EXIT_FAILURE)){
			if (pedidosMatriz[i][j] != 0){
				tienePedido= EXIT_SUCCESS;
			}
			if(asignadosMatriz[i][j] != 0){
				tieneAsignado = EXIT_SUCCESS;
			}
			j++;
		}

		if((tienePedido ==  EXIT_FAILURE || tieneAsignado ==  EXIT_FAILURE)){
			entrenadoresEnDeadlock[i] = 1;
		}
	}
}
void algoritmo(){
	int marcar;
	int j;
	int i;
	int k;
	marcar = EXIT_SUCCESS;
	for(i = 0; i < cantidadDeEntrenadores; i++){
		if(entrenadoresEnDeadlock[i] == 0){
			j = 0;
			while(j < cantidadDePokemones && marcar ==  EXIT_SUCCESS){
				if (pedidosMatriz[i][j] > algoritmoVector[j]){
					marcar = EXIT_FAILURE;
				}
				j++;
			}

			if(marcar ==  EXIT_SUCCESS){
				entrenadoresEnDeadlock[i] = 1;
				for(k = 0;k < cantidadDePokemones; k++){
					algoritmoVector[k] = algoritmoVector[k] + asignadosMatriz[i][k];
				}
				algoritmo();

			}
			else {
				hayDeadlock = EXIT_SUCCESS;
			}
			marcar = EXIT_SUCCESS;
		}
		else {}
	}
}

void resolverDeadlock(){

	crearPokemones();

	cantidadDeEntrenadoresEnDeadlock = 4;

	printf("alalal");

	pokemonA = create_pokemon(fabrica, "Pikachu", 200);
	pokemonB = create_pokemon(fabrica, "Squirtle", 500);
	pokemonC = create_pokemon(fabrica, "Rhyhorn", 15);
	pokemonD = create_pokemon(fabrica, "Charmander", 100);

	mejoresPokemones = list_create();

	list_add(mejoresPokemones, pokemonA);
	list_add(mejoresPokemones, pokemonB);
	list_add(mejoresPokemones, pokemonC);
	list_add(mejoresPokemones, pokemonD);

	pokemonPerdedor = list_get(mejoresPokemones, 0);
	int indiceDeEntrenadorPerdedor = 0;
	int h = 1;
	t_pokemon* pokemonPerdedorAnterior = pokemonPerdedor;
	while(h < cantidadDeEntrenadoresEnDeadlock){
		pokemonPerdedor = batallaPokemon(pokemonPerdedor,list_get(mejoresPokemones,h),indiceDeEntrenadorPerdedor,h);
		if(pokemonPerdedor != pokemonPerdedorAnterior){
			pokemonPerdedorAnterior = pokemonPerdedor;
			indiceDeEntrenadorPerdedor = h;
		}
		h++;
	}
	char str[100];
	char str2[2];
	str2[0] = indiceDeEntrenadorPerdedor + '0';
	strcat(str,pokemonPerdedor->species);
	strcat(str," del entrenador ");
	strcat(str,str2);

	log_info(logDeadlock,str);

	//free(str);
	free(algoritmoVector);
	free(entrenadoresEnDeadlock);
	list_destroy(mejoresPokemones);
	destroy_pkmn_factory(fabrica);
}

t_pokemon* batallaPokemon(t_pokemon* pkmnA, t_pokemon* pkmnB, int indiceA, int indiceB){
	pokemonPerdedor = pkmn_battle(pkmnA,pkmnB);
	if(pokemonPerdedor == pkmnA){
		notificarResultadoBatalla(indiceA, EXIT_SUCCESS);
		notificarResultadoBatalla(indiceB, EXIT_FAILURE);
	}
	else{
			notificarResultadoBatalla(indiceB, EXIT_SUCCESS);
			notificarResultadoBatalla(indiceA, EXIT_FAILURE);
		}
	return pokemonPerdedor;
}

void notificarResultadoBatalla(int indice, int gano){
	t_datosEntrenador* entrenador;
	entrenador = malloc(sizeof(t_datosEntrenador));
	entrenador = (t_datosEntrenador*) (list_get(Entrenadores, indice));
	if(gano){
		enviarHeader(entrenador->socket,entrenadorGanador);
	}
	else {
		enviarHeader(entrenador->socket,entrenadorPerdedor);
	}
	free(entrenador);
}

void crearPokemones(){
	fabrica = create_pkmn_factory();
	int i;
	for(i = 0; i < cantidadDeEntrenadores;i++){
		if(entrenadoresEnDeadlock[i] == 0){
			cantidadDeEntrenadoresEnDeadlock++;
			//todo obtengo el indice, el socket, y le pido el pokemon mas fuerte
			t_datosEntrenador* entrenador;
			entrenador = malloc(sizeof(t_datosEntrenador));
			t_metadataPokemon* pokemon;
			pokemon = malloc(sizeof(t_metadataPokemon));
			entrenador = (t_datosEntrenador*) (list_get(Entrenadores, i));
			enviarHeader(entrenador->socket,mejorPokemon);
			recibirTodo(entrenador->socket,pokemon,sizeof(t_metadataPokemon));
			list_add(mejoresPokemones,pokemon);
			free(entrenador);
		}
	}
}

void sumarPedidosMatriz(int indiceEntrenador, int indicePokenest){
	pedidosMatriz[indiceEntrenador][indicePokenest] = pedidosMatriz[indiceEntrenador][indicePokenest] + 1;
}

void sumarAsignadosMatriz(int indiceEntrenador, int indicePokenest){
	asignadosMatriz[indiceEntrenador][indicePokenest] = asignadosMatriz[indiceEntrenador][indicePokenest] + 1;
	pedidosMatriz[indiceEntrenador][indicePokenest] = pedidosMatriz[indiceEntrenador][indicePokenest] - 1;
}

void restarAsignadosMatriz(int indiceEntrenador, int indicePokenest){
	asignadosMatriz[indiceEntrenador][indicePokenest] = asignadosMatriz[indiceEntrenador][indicePokenest] - 1;
}

void liberarRecursosEntrenador(int indiceEntrenador){
	int i;
	int j;
	for(i = indiceEntrenador; i < cantidadDeEntrenadores-1;i++){
		for(j = 0; j < cantidadDePokemones;j++){
			asignadosMatriz[i][j] = asignadosMatriz[i+1][j];
		}
	}
}
