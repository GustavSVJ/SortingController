#include "regulator.h"
#include "fixpoint.h"

void regul_init(statetype *p) {
	/* This function should initialise the state variables of the controller */
	p->dummy = 0;
	p->u = 0;
}

void regul_out(statetype *p, short e, short b0, short b0exp) {
	/*e is the input to the controller and b0 is a parameter*/
	/* the function should return the controller output in the statetype variable */
	p->u = add16(mul16(b0, e, b0exp), p->dummy);
}

void regul_update(statetype *p, short e, short b1, short a1, short b1exp, short a1exp) {
	/* this function should update the state variables of the controller */
	p->dummy = sub16(mul16(b1, e, b1exp), mul16(a1, p->u, a1exp));
}