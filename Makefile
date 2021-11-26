ifeq ($(DEBUG),true)
	CC = gcc -g
else
	CC = gcc
endif

all: HLZ.o
	$(CC) -o HLZ HLZ.o

HLZ.o: HLZ.c HLZ.h
	$(CC) -c HLZ.c

clean:
	rm *.o