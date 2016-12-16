OBJ = ./*.c

all:
	make clean	
	gcc -I"/../../LibreriasSO/"  -c  "mapa.c"
	gcc -I"/../../LibreriasSO/"  -c  "deadlock.c"
	gcc -I"/../../LibreriasSO/"  -c  "dibujador.c"
	gcc -I"/../../LibreriasSO/"  -c  "funciones.c"
	gcc -I"/../../LibreriasSO/"  -c  "planificadores.c"
	gcc -L"/../../LibreriasSO/" -o "mapa"  mapa.o deadlock.o dibujador.o funciones.o planificadores.o  -lnivel-gui -lcurses -lcommons -lpkmm-battle -lLibreriasSO -pthread
	rm -rf *.o

clean:
	rm -rf *.o
	rm -rf mapa