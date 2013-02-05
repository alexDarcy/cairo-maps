CFLAGS=-Wall -pedantic -g -I/usr/X11R6/include `pkg-config --cflags cairo`
LDFLAGS=-Wall -g `pkg-config --libs cairo` -L/usr/X11R6/lib -lX11 -lm
CC = gcc

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all: projection

projection: $(OBJ)
	@echo $(OBJ)
	$(CC) -o $@ ${LDFLAGS} $^
exec:
	./projection

clean:
	rm *.o
