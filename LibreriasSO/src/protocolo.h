#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

enum headers { //Constantes que identifican los headers de los mensajes

	datosInicialesMapa = 1,
	datosPokenest = 2,
	posicionEntrenador = 3,

};
int recibirHeader(int socketOrigen);


#endif /* PROTOCOLO_H_ */
