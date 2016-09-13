#include "funciones.h"

void receptorSIG(int signum) {
	while (1) {
		pthread_mutex_lock(&mutex);
		signal(SIGUSR2, receptorSIG);
		if (signum == SIGUSR2) {
			printf("Received SIGUSR2!\n");
		}
		pthread_mutex_unlock(&mutex);
	}
}

