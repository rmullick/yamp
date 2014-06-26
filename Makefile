all:	yamp

thread: 
	gcc -c thread.c

sockint: 
	gcc -c sockint.c

yamp:   sockint.o thread.o
	gcc -g3 -o yamp thread.o sockint.o yamp.c -lpthread

clean:
	rm -rf *o *~ yamp

