OBJ = ./*.c

all:
	make clean	
	gcc -I"/../../LibreriasSO/"  -c  "dexServer.c"
	gcc -L"/../../LibreriasSO/" -o "dexServer"  dexServer.o   -lcommons -pthread -lLibreriasSO 
	rm -rf *.o

clean:
	rm -rf *.o
	rm -rf dexServer