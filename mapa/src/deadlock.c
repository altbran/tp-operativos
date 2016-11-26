/*
 * deadlock.c
 *
 *  Created on: 15/10/2016
 *      Author: utnso
 */

#include "deadlock.h"

void detectarDeadlock() {

	logers = log_create("Pruebas.log", "deadlock", 0, log_level_from_string("INFO"));

	inicializarMatrices();
	inicializarVectores();

	log_info(logers, "Empezó");

	cantidadDeEntrenadores = list_size(Entrenadores);

	while(1){
		//sem_wait(&contadorEntrenadoresBloqueados);
		sleep(configuracion->tiempoChequeoDeadlock / 1000);
		if(list_size(Entrenadores) > 1){

			cantidadDePokemones = list_size(Pokenests);

			inicializarAlgoritmoVector();

			log_info(logers, "Entró");

			batallaActivada = true;

			logDeadlock = log_create("Deadlock.log", "deadlock", 0, log_level_from_string("INFO"));

			hayDeadlock = false;

			noTieneAsignadosOPedidos();

			algoritmo();

			if(hayDeadlock){
				log_info(logDeadlock, "Hay deadlock");
				log_info(logDeadlock, "Matriz de asignados");
				mostrarMatriz(asignadosMatriz);
				log_info(logDeadlock, "Matriz de pedidos");
				mostrarMatriz(pedidosMatriz);
				log_info(logDeadlock, "Entrenadores en deadlock");
				mostrarEntrenadoresEnDeadlock();

				resolverDeadlock();
			}

			else{
				log_info(logDeadlock, "No hay deadlock");
			}
		}

	}
}

void inicializarMatrices() {
	int i;
	log_info(logers, "inicializa");
	pedidosMatriz = (int **) malloc(cantidadDeEntrenadores * sizeof(int*));
	asignadosMatriz = (int **) malloc(cantidadDeEntrenadores * sizeof(int*));
}

void inicializarEntrenadorEnMatrices(int indice){
	pedidosMatriz[indice] = (int *) malloc(cantidadDePokemones * sizeof(int));
	asignadosMatriz[indice] = (int *) malloc(cantidadDePokemones * sizeof(int));

}

void agregarEntrenadorEnMatrices(){
	cantidadDeEntrenadores++;
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
			t_datosEntrenador* entrenador;
			entrenador = (t_datosEntrenador*) (list_get(Entrenadores, p));
			log_info(logDeadlock,"%s", entrenador->nombre);
			log_info(logDeadlock,"%d", p);
			notificarDeadlockAEntrenador(p);
		}
	}
}

void notificarDeadlockAEntrenador(int indice){
	t_datosEntrenador* entrenador;
	entrenador = (t_datosEntrenador*) (list_get(Entrenadores, indice));
	enviarHeader(entrenador->socket,notificarDeadlock);
}

void notificarMuerteAEntrenador(int indice){
	t_datosEntrenador* entrenador;
	entrenador = (t_datosEntrenador*) (list_get(Entrenadores, indice));
	enviarHeader(entrenador->socket,entrenadorMuerto);
	cantidadDeEntrenadores--;

}

void noTieneAsignadosOPedidos(){
	bool tienePedido;
	bool tieneAsignado;
	int j;
	int i;
	for(i = 0; i < cantidadDeEntrenadores; i++){
		j = 0;
		tienePedido = false;
		tieneAsignado = false;
		while(j < cantidadDePokemones && (!tienePedido || !tieneAsignado)){
			if (pedidosMatriz[i][j] != 0){
				tienePedido= true;
			}
			if(asignadosMatriz[i][j] != 0){
				tieneAsignado = true;
			}
			j++;
		}

		if((!tienePedido || !tieneAsignado)){
			entrenadoresEnDeadlock[i] = 1;
		}
	}
}
void algoritmo(){
	bool marcar;
	int j;
	int i;
	int k;
	marcar = true;
	for(i = 0; i < cantidadDeEntrenadores; i++){
		if(entrenadoresEnDeadlock[i] == 0){
			j = 0;
			while(j < cantidadDePokemones && marcar){
				if (pedidosMatriz[i][j] > algoritmoVector[j]){
					marcar = false;
				}
				j++;
			}

			if(marcar){
				entrenadoresEnDeadlock[i] = 1;
				for(k = 0;k < cantidadDePokemones; k++){
					algoritmoVector[k] = algoritmoVector[k] + asignadosMatriz[i][k];
				}
				algoritmo();

			}
			else {
				hayDeadlock = true;
			}
			marcar = true;
		}
		else {}
	}
}

void resolverDeadlock(){

	if(batallaActivada)
	{
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
		notificarMuerteAEntrenador(indiceDeEntrenadorPerdedor);
		log_info(logDeadlock,str);

		//free(str);
		free(algoritmoVector);
		free(entrenadoresEnDeadlock);
		list_destroy(mejoresPokemones);
		destroy_pkmn_factory(fabrica);
	}
	else
	{
		resolverDeadlockAMiManera();
	}
}

void resolverDeadlockAMiManera(){
	int indiceEntrenadorMuerto;
	indiceEntrenadorMuerto = obtenerPrimerEntrenadorEnDeadlock();
	notificarMuerteAEntrenador(indiceEntrenadorMuerto);
}

int obtenerPrimerEntrenadorEnDeadlock(){
	int i;
	for(i = 0;i < cantidadDeEntrenadores;i++){
		if(entrenadoresEnDeadlock[i] == 0)
			return i;
	}
}

t_pokemon* batallaPokemon(t_pokemon* pkmnA, t_pokemon* pkmnB, int indiceA, int indiceB){
	pokemonPerdedor = pkmn_battle(pkmnA,pkmnB);
	if(pokemonPerdedor == pkmnA){
		notificarResultadoBatalla(indiceA, false);
		notificarResultadoBatalla(indiceB, true);
	}
	else{
			notificarResultadoBatalla(indiceB, false);
			notificarResultadoBatalla(indiceA, true);
		}
	return pokemonPerdedor;
}

void notificarResultadoBatalla(int indice, bool gano){
	t_datosEntrenador* entrenador;
	entrenador = (t_datosEntrenador*) (list_get(Entrenadores, indice));
	if(gano){
		enviarHeader(entrenador->socket,entrenadorGanador);
	}
	else {
		enviarHeader(entrenador->socket,entrenadorPerdedor);
		close(entrenador->socket);
	}
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
	log_info(logDeadlock,"pedidos: %d", pedidosMatriz[indiceEntrenador][indicePokenest]);
}

void sumarAsignadosMatriz(int indiceEntrenador, int indicePokenest){
	asignadosMatriz[indiceEntrenador][indicePokenest] = asignadosMatriz[indiceEntrenador][indicePokenest] + 1;
	pedidosMatriz[indiceEntrenador][indicePokenest] = pedidosMatriz[indiceEntrenador][indicePokenest] - 1;
	log_info(logDeadlock,"pedidos: %d", pedidosMatriz[indiceEntrenador][indicePokenest]);
	log_info(logDeadlock,"asignados: %d", asignadosMatriz[indiceEntrenador][indicePokenest]);
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
			pedidosMatriz[i][j] = pedidosMatriz[i+1][j];
		}
	}
}
