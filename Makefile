CC=gcc
CFLAGS=-c -Wall
SOURCES=thread.c sockint.c yamp.c
LDFLAGS=-lpthread
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=yamp

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE):$(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o *~ yamp

