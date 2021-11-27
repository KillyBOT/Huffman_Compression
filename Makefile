ifeq ($(DEBUG),true)
	CC = gcc -g
else
	CC = gcc
endif

all: huffman.o
	$(CC) -o huffman huffman.o

huffman.o: huffman.c huffman.h
	$(CC) -c huffman.c

clean:
	rm *.o