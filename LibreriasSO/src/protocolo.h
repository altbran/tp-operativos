#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

enum headers { //Constantes que identifican los headers de los mensajes

	datosInicialesMapa = 1,
	datosPokenest = 2,
	posicionEntrenador = 3,
	capturarPokemon = 7,
	pokemonesDisponibles = 9,
	entrenadorListo = 10,
	finalizoMapa = 11,
	//Headers para comunicacion entre cliente y servidor pokedex
	privilegiosArchivo = 4,
	contenidoDirectorio = 5,
	contenidoArchivo = 6,
	recibirXCantidadDeArchivos = 8,
	//no se confundan, no repitan los numeros!!

};
int recibirHeader(int socketOrigen);
void enviarHeader(int socketDestino, int header);

#endif /* PROTOCOLO_H_ */
