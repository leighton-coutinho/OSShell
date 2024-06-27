CC=gcc
CFLAGS= -D

mysh: interpreter.c shellmemory.c shell.c interpreter.h
	$(CC) $(CFLAGS) fSize=$(framesize) $(CFLAGS) vSize=$(varmemsize) -c interpreter.c shellmemory.c shell.c 
	$(CC) $(CFLAGS) fSize=$(framesize) $(CFLAGS) vSize=$(varmemsize) -o mysh interpreter.o shellmemory.o shell.o 

clean: 
	rm mysh; rm *.o
