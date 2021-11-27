ifeq ($(DEBUG),true)
	CC = gcc -g
else
	CC = gcc
endif

all: Huffman.o
	$(CC) -o Huffman Huffman.o

Huffman.o: Huffman.c Huffman.h
	$(CC) -c Huffman.c

clean:
	rm *.o