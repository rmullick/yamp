all:	prox

prox:   sockint.o
	gcc -o prox sockint.o prox.c

sockint: 
	gcc -c sockint.c

clean:
	rm -rf *o *~ prox

