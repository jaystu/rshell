FLAGS = -g  -Wall -Werror -ansi -pedantic 
COMPILE = g++

SOURCES= src/main.cpp

all: 
	mkdir -p bin
	$(COMPILE) $(FLAGS) $(SOURCES) -o bin/rshell

clean: 
	rm bin/rshell
	rm -rf bin

