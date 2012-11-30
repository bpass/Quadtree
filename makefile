CC=g++
LIBS=-lgdal
CFLAGS=-c

all: quadtree

quadtree: main.o Quadtree.o Utility.h
	$(CC) main.o Quadtree.o Utility.h $(LIBS) -o quadtree

main.o: main.h main.cpp
	$(CC) main.h main.cpp $(CFLAGS) $(LIBS) 

quadtree.o: Quadtree.h Quadtree.cpp
	$(CC) Quadtree.h Quadtree.cpp $(CFLAGS) $(LIBS) 

clean:
	rm -rf *.o quadtree.out
