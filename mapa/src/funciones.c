#include "funciones.h"

void receptorSIG() {
	llegoSenial = 1;
}

void cargarMetadata(){
	t_config config = config_create(ruta);
	configuracion.tiempoChequeoDeadlock = config_get_int_value(config,"TiempoChequeoDeadlock");
	configuracion.batalla = config_get_int_value(config, "Batalla");
	configuracion.algoritmo = config_get_string_value(config,"algoritmo");
	configuracion.quantum = config_get_int_value(config,"quantum");
	configuracion.retardo = config_get_int_value(config,"retardo");
	configuracion.ip = config_get_int_value(config, "IP");
	configuracion.puerto = config_get_int_value(config, "Puerto");
}
