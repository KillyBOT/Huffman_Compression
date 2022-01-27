ifeq ($(DEBUG),true)
	CC = gcc -g -Wall
else
	CC = gcc
endif

CFLAG = -Wall

huffman: huffman.o

huffman.o: huffman.c huffman.h

.PHONY: clean
clean:
	rm *.o