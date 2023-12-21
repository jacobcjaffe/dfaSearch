#! make

CC = g++
SOURCE = $(wildcard *.cpp)
TARGETS = $(SOURCE:.cpp=)
HEADERS = $(wildcard *.h)
OBJECTS = main.o dfa.o GenSolution.o

STD = -std=c++20
OPTIMIZER = -O3
CFLAGS = $(STD) -Wall -g $(OPTIMIZER)

main: $(OBJECTS) 
	$(CC) $(CFLAGS) -o main $(OBJECTS)

dfa.o: dfa.cpp dfa.h BitVector.h

Gensolution.o: GenSolution.cpp Gensolution.h dfa.h

clean clobber:
	$(RM) $(OBJECTS)
