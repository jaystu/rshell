FLAGS = -g  -Wall -Werror -ansi -pedantic 
COMPILE = g++

SOURCES= src/main.cpp

all: 
	mkdir -p bin
	$(COMPILE) $(FLAGS) $(SOURCES) -o bin/a.out

clean: 
	rm bin/a.out
	rm -rf bin

