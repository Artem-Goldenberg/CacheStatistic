
CPPFLAGS = -std=c++17 -O2

all: main
	./main

main: main.cpp

clean:
	$(RM) main
