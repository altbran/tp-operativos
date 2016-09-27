#ifndef DIBUJADOR_H_
#define DIBUJADOR_H_


//variables
t_list* items;

//funciones
void dibujar();
void cargarPokenest(t_metadataPokenest pokenest);
void cargarEntrenador(t_metadataEntrenador entrenador);
void restarPokemon(char identificador);

/*int posicionInicialX = 1;
int posicionInicialY = 1;*/

#endif /* DIBUJADOR_H_ */
