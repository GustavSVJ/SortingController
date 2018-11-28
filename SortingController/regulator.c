#include "regulator.h"
#include "fixpoint.h"

void regul_init(statetype *p) {
	/* This function should initialise the state variables of the controller */
	p->dummy = 0;
	p->x = 0;
	p->u = 0;
}

void regul_out(statetype *p, short e, short b0, short b0exp) {
	/*e is the input to the controller and b0 is a parameter*/
	/* the function should return the controller output in the statetype variable */
	short input = e << 4;
	p->x = add16(input, p->dummy);
	p->u = mul16(b0, p->x, b0exp) >> 4;
}

void regul_update(statetype *p, short e, short b1, short a1, short b1exp, short a1exp) {
	/* this function should update the state variables of the controller */
	short input = e << 4;
	p->dummy = sub16(mul16(b1, input, b1exp), mul16(a1, p->x, a1exp));
}