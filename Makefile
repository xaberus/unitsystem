
CCFLAGS=-std=c99 -Wall -Wextra -pedantic -D_GNU_SOURCE -g -ggdb -I/repos/butcher -DTEST
LDFLAGS=-lelf -ldl -Wl,-rpath,. -rdynamic -lmpfr
INCLUDES=-I .
CC=gcc
USLDFLAGS=-L . -lunitsystem
BUTCHER=/repos/butcher/butcher -b /repos/butcher/bexec -v

rational.o: rational.c unitsystem.h
		$(CC) $(CCFLAGS) $(INCLUDES) -fPIC -c rational.c -o rational.o
prefix.o: prefix.c unitsystem.h
		$(CC) $(CCFLAGS) $(INCLUDES) -fPIC -c prefix.c -o prefix.o
atom.o: atom.c unitsystem.h
		$(CC) $(CCFLAGS) $(INCLUDES) -fPIC -c atom.c -o atom.o
part.o: part.c unitsystem.h
		$(CC) $(CCFLAGS) $(INCLUDES) -fPIC -c part.c -o part.o
unit.o: unit.c unitsystem.h
		$(CC) $(CCFLAGS) $(INCLUDES) -fPIC -c unit.c -o unit.o
library.o: library.c unitsystem.h
		$(CC) $(CCFLAGS) $(INCLUDES) -fPIC -c library.c -o library.o


libunitsystem.so: unitsystem.c unitsystem.h rational.o prefix.o atom.o part.o unit.o library.o
		$(CC) $(CCFLAGS) $(LDFLAGS) $(INCLUDES) -fPIC -shared unitsystem.c -o libunitsystem.so rational.o prefix.o atom.o part.o unit.o library.o

runner: runner.c unitsystem.h
		$(CC) $(CCFLAGS) $(LDFLAGS) $(USLDFLAGS) $(INCLUDES) -o runner runner.c

all: libunitsystem.so

butcher: libunitsystem.so
		$(BUTCHER) libunitsystem.so

clean:
		rm *.o *.so -f
