#include "regulator.h"
#include <stdio.h>

int main()
{
    printf("hello from SortingController!\n");

	statetype reg;

	regul_init(&reg);

	int i;
	for (i = 0; i < 100; i++) {
		double e = 1;
		regul_out(&reg, e, 85.88);
		printf("Value = %f\n", reg.u);
		regul_update(&reg, e, -73.42, -0.4378);
	}


    return 0;
}