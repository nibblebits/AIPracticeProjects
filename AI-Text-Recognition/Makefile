OBJECTS=./build/image.o ./build/train.o
CFLAGS=`sdl2-config --cflags --libs` -lSDL2_image -g -lm

all: main

main: main.c $(OBJECTS)
	gcc -o main main.c $(OBJECTS) $(CFLAGS)

./build/image.o: image.c
	gcc -c image.c -o ./build/image.o $(CFLAGS)

./build/train.o: train.c
	gcc -c train.c -o ./build/train.o $(CFLAGS)

clean:
	rm -f main $(OBJECTS)
