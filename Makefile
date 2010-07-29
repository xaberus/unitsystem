
CCFLAGS=-std=c99 -Wall -Wextra -pedantic -D_GNU_SOURCE -g -ggdb -I/repos/butcher
LDFLAGS=-lelf -ldl -Wl,-rpath,. -rdynamic
INCLUDES=-I .
CC=gcc

libunitsystem.so: unitsystem.c unitsystem.h rational.c prefix.c atom.c part.c unit.c library.c
		$(CC) $(CCFLAGS) $(LDFLAGS) $(INCLUDES) -fPIC -shared unitsystem.c -o libunitsystem.so

runner: runner.c unitsystem.h
		$(CC) $(CCFLAGS) $(LDFLAGS) $(INCLUDES) -o runner runner.c libunitsystem.so

all: libunitsystem.so runner
