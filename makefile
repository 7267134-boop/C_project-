CC = gcc
CFLAGS = -ansi -pedantic -Wall -g -fsanitize=address

OBJFILES = assembler.o precompiler.o firstSyntax.o secendSyntax.o

all: main

main: $(OBJFILES)
	$(CC) $(CFLAGS) -o main $(OBJFILES)

assembler.o: assembler.c precompiler.h
	$(CC) $(CFLAGS) -c assembler.c

precompiler.o: precompiler.c precompiler.h defs.h
	$(CC) $(CFLAGS) -c precompiler.c

firstSyntax.o: firstSyntax.c firstSyntax.h defs.h
	$(CC) $(CFLAGS) -c firstSyntax.c 

secendSyntax.o: secendSyntax.c secendSyntax.h defs.h
	$(CC) $(CFLAGS) -c secendSyntax.c 

clean:
	rm -f *.o main *am *ob *ext *ent

