all: c8

c8: run.o screen.o chip8.o
	g++ run.o screen.o chip8.o -o c8 -g -lncurses
run.o: run.cpp
	g++ -c run.cpp
screen.o: screen.cpp screen.h
	g++ -c screen.h screen.cpp
c8.o: c8.cpp c8.h
	g++ -c c8.cpp c8.h
clean:
	rm *.o *.gch .chip8*

