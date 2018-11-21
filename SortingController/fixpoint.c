#include <stdio.h>
#include <math.h>

short mul16(short fac1, short fac2, short exp) {
	long f1;
	f1 = fac1;
	f1 *= fac2;
	f1 += (1l << (14 - exp));
	if (exp > 15)
		f1 <<= 1;
	else
		f1 >>= (15 - exp);
	if (f1 > 32767l)
		f1 = 32767l;
	else
		if (f1 < -32768l)
			f1 = -32768l;
	return(f1);
}

short add16(short add1, short add2) {
	long t;
	t = add1;
	t += add2;
	if (t > 32767l)
		t = 32767l;
	else
		if (t < -32768l)
			t = -32768l;
	return(t);
}

short sub16(short min, short sub) {
	long t;
	t = min;
	t -= sub;
	if (t > 32767l)
		t = 32767l;
	else
		if (t < -32768l)
			t = -32768l;
	return(t);
}

/*
void main(void){
short i1,i2,exp;
float f1,f2;
f1=1;
while (f1 != 0){
printf("enter factors\n");

scanf("%f %f",&f1,&f2);
i1=f1*32768.0+0.5;
i2=f2*32768.0+0.5;
printf("%d %d  \n",i1,i2);
exp=0;
i1=sub16(i1,i2);
printf("%d %f %f\n",i1,i1/32768.0,(i1/32768.0-(f1-f2)*pow(2,exp))*32768 )  ;
}

}

*/