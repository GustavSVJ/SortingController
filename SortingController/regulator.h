#ifndef REGULATOR_H_
#define REGULATOR_H_

typedef struct {
	/*controller state variables should be declared here*/
	double dummy, u;
}statetype;

void regul_init(statetype *p);
void regul_out(statetype *p, double e, double b0);
void regul_update(statetype *p, double e, double b1, double a1);

#endif