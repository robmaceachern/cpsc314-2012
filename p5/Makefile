CC=	g++
CFLAGS=	-Iinclude
PROG=   wallbuster
SRCS=   src/main.cpp
OBJS=	$(SRCS:.cpp=.o)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) \
		-framework OpenGL -framework GLUT


%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

main.o: string

clean:
	rm -f *~ $(OBJS) $(PROG)
