CCFLAGS = -g -pg

default: P2

P2: main.cpp
	g++ ${CCFLAGS_MAKE_TEXTURES} main.cpp -o P2 \
		-framework OpenGL -framework GLUT

FileParser.o: include/FileParser.h
main.o: include/Primitives.h

clean:
	touch P2
	rm -rf P2 *.o
