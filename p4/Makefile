CC=	g++
CFLAGS=	-Iinclude
PROG=   wallbuster
SRCS=   src/FileParser.cpp src/Primitives.cpp src/main.cpp
OBJS=	$(SRCS:.cpp=.o)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) \
		-framework OpenGL -framework GLUT


%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

Primitives.o: include/Primitives.h math.h 
FileParser.o: include/FileParser.h include/Primitives.h vector iostream fstream sstream 
main.o: include/Primitives.h include/FileParser.h vector string

clean:
	rm -f *~ $(OBJS) $(PROG)
