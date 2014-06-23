all:	prox

thread: 
	gcc -c thread.c

sockint: 
	gcc -c sockint.c

prox:   sockint.o thread.o
	gcc -o prox thread.o sockint.o prox.c -lpthread

clean:
	rm -rf *o *~ prox

