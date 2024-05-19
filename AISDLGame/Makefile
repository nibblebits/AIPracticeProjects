OBJECTS=./build/game.o
CFLAGS=`sdl2-config --cflags --libs` -lSDL2_image -g -lm

all: main

main: main.c $(OBJECTS)
	gcc -o main main.c $(OBJECTS) $(CFLAGS)

./build/game.o: ./game.c ./game.h
	gcc -c ./game.c -o ./build/game.o $(CFLAGS)

clean:
	rm -f main $(OBJECTS)
