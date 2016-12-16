OBJ = ./*.c

all:
	make clean	
	gcc -I"/../../LibreriasSO/"  -c  "Entrenador.c"
	gcc -I"/../../LibreriasSO/"  -c  "funcionesE.c"
	gcc -L"/../../LibreriasSO/" -o "Entrenador"  Entrenador.o  funcionesE.o   -lLibreriasSO -lcommons
	rm -rf *.o

clean:
	rm -rf *.o
	rm -rf Entrenador