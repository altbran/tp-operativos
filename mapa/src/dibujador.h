#ifndef DIBUJADOR_H_
#define DIBUJADOR_H_


//variables
t_list* items;

//funciones
void crearItems();
void dibujar(char* nombreMapa);
void cargarPokenest(t_metadataPokenest pokenest);
void cargarEntrenador(t_datosEntrenador entrenador);
void moverEntrenador(t_datosEntrenador entrenador);
void restarPokemon(char * identificador);

/*int posicionInicialX = 1;
int posicionInicialY = 1;*/

#endif /* DIBUJADOR_H_ */
