#include "regulator.h"

void regul_init(statetype *p) {
	/* This function should initialise the state variables of the controller */
	p->dummy = 0;
	p->u = 0;
}

void regul_out(statetype *p, double e, double b0) {
	/*e is the input to the controller and b0 is a parameter*/
	/* the function should return the controller output in the statetype variable */
	p->u = b0 * e + p->dummy;
}

void regul_update(statetype *p, double e, double b1, double a1) {
	/* this function should update the state variables of the controller */
	p->dummy = (b1 * e) - (a1 * p->u);
}