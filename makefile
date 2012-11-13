CC=g++
LIBS=-lgdal

quadtree.out: main.cpp Quadtree.h Quadtree.cpp main.h Utility.h
	$(CC) -o quadtree.out main.cpp Quadtree.h Quadtree.cpp $(LIBS)
