#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

enum headers { //Constantes que identifican los headers de los mensajes

	datosInicialesMapa = 1,
	datosPokenest = 2,
	posicionEntrenador = 3,
	//Headers para comunicacion entre cliente y servidor
	privilegiosArchivo = 4,
	contenidoDirectorio = 5,
	contenidoArchivo = 6,

};
int recibirHeader(int socketOrigen);
void enviarHeader(int socketDestino, int header, int largo);

#endif /* PROTOCOLO_H_ */
