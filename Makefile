
CPPFLAGS = -std=c++17

all: main
	./main

main: main.cpp

clean:
	$(RM) main
