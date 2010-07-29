
#include "unitsystem.h"

#include <stdio.h>

#define stz(type) printf("sizeof("#type") = %lu\n", sizeof(type))

int main(int argc, char * argv[])
{
	for (int i = 0; i < argc; i++) {
		printf("argv[%d]: %s\n", i, argv[i]);
	}
	stz(struct us_prefix);
	stz(struct us_atom);

	return 0;
}
