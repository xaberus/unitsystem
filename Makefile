
CCFLAGS=-std=c99 -Wall -Wextra -pedantic -D_GNU_SOURCE -g -ggdb -DTEST -fPIC #-Werror
LDFLAGS=-lelf -ldl -Wl,-rpath,. -lmpfr -lgmp -lsexp -rdynamic

INCLUDES=-I . -I/repos/butcher -I /usr/include/sexpr
CC=gcc
USLDFLAGS=-L . -lunitsystem
BUTCHER=/repos/butcher/butcher -b /repos/butcher/bexec 
BFLAGS=-n

.PHONY: all
all: libunitsystem.so

rational.o: rational.c unitsystem.h err.h
		$(CC) $(CCFLAGS) $(INCLUDES) -c rational.c -o rational.o
prefix.o: prefix.c unitsystem.h err.h
		$(CC) $(CCFLAGS) $(INCLUDES) -c prefix.c -o prefix.o
atom.o: atom.c unitsystem.h err.h
		$(CC) $(CCFLAGS) $(INCLUDES) -c atom.c -o atom.o
part.o: part.c unitsystem.h err.h
		$(CC) $(CCFLAGS) $(INCLUDES) -c part.c -o part.o
base_unit.o: base_unit.c unitsystem.h err.h
		$(CC) $(CCFLAGS) $(INCLUDES) -c base_unit.c -o base_unit.o
unit.o: unit.c unitsystem.h err.h
		$(CC) $(CCFLAGS) $(INCLUDES) -c unit.c -o unit.o
library.o: library.c unitsystem.h err.h
		$(CC) $(CCFLAGS) $(INCLUDES) -c library.c -o library.o

libunitsystem.so: unitsystem.c unitsystem.h rational.o prefix.o atom.o part.o base_unit.o unit.o library.o /repos/butcher/bt.h
		$(CC) $(CCFLAGS) $(INCLUDES) unitsystem.c -o libunitsystem.so rational.o prefix.o atom.o part.o base_unit.o unit.o library.o -shared $(LDFLAGS)

chop: libunitsystem.so
		$(BUTCHER) $(BFLAGS) libunitsystem.so

.PHONY: clean
clean:
		rm *.o *.so crap -f
