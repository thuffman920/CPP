CC= gcc

CFLAGS= -Wall -std=c99 -g

mygrep: mygrep.o pattern.o

pattern.o: pattern.c pattern.h
mygrep.o: mygrep.c pattern.h

clean:
	rm -f mygrep.o
	rm -f pattern.o
	rm -f mygrep
