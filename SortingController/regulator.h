#ifndef REGULATOR_H_
#define REGULATOR_H_

typedef struct {
	/*controller state variables should be declared here*/
	short dummy, u, x;
}statetype;

void regul_init(statetype *p);
void regul_out(statetype *p, short e, short b0, short b0exp);
void regul_update(statetype *p, short e, short b1, short a1, short b1exp, short a1exp);

#endif