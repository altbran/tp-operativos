#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

enum headers { //Constantes que identifican los headers de los mensajes

	socketDesconectado = 0,
	datosInicialesMapa = 1,
	datosPokenest = 2,
	enviarDatosPokenest = 17,
	posicionEntrenador = 3,
	capturarPokemon = 7,
	pokemonesDisponibles = 9,
	entrenadorListo = 10,
	finalizoMapa = 11,
	movimientoAceptado= 15,
	mejorPokemon = 19,
	notificarDeadlock= 20,
	notificarResultadoBatalla = 21,
	//Headers para comunicacion entre cliente y servidor pokedex
	privilegiosArchivo = 4,
	contenidoDirectorio = 5,
	contenidoArchivo = 6,
	recibirXCantidadDeArchivos = 8,
	crearCarpeta = 9,
	eliminarArchivo = 12,
	abrirArchivo = 13,
	escribirEnFichero = 14,
	removerDirectorio = 16,
	cerrarArchivo = 18,
	//no se confundan, no repitan los numeros!!

};
int recibirHeader(int socketOrigen);
void enviarHeader(int socketDestino, int header);

#endif /* PROTOCOLO_H_ */
