OBJ = ./*.c

all:
	make clean	
	gcc -I"/../../LibreriasSO/"  -c  "dexCliente.c"
	gcc -L"/../../LibreriasSO/" -o "dexCliente"  dexCliente.o   -lLibreriasSO -pthread -lfuse
	rm -rf *.o

clean:
	rm -rf *.o
	rm -rf dexCliente