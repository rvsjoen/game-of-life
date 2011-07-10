CFLAGS 	= -g
LIBS 		= -lncurses
CC 			= gcc
EXEC 		= gol
TEST		= glider.gol
VALOPTS = --leak-check=full --show-reachable=yes --track-origins=yes \
					--log-file=$(VALFILE)
VALFILE = report

all: gol

gol: gol.o
	$(CC) -o $(EXEC) $(LIBS) $(CFLAGS) gol.o

gol.o: gol.c
	$(CC) $(CFLAGS) -c gol.c

clean:
	rm -rf *.o $(EXEC) $(VALFILE)

test: all
	./$(EXEC) $(TEST)

val: all
	valgrind $(VALOPTS) ./$(EXEC) $(TEST) 
	less $(VALFILE)
