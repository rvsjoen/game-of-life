CFLAGS 	= 
LIBS 	= -lncurses
CC 		= gcc
EXEC 	= gol

all: gol

gol: gol.o
	$(CC) -o $(EXEC) $(LIBS) $(CFLAGS) gol.o

gol.o: gol.c
	$(CC) $(CFLAGS) -c gol.c

clean:
	rm -rf *.o $(EXEC)
