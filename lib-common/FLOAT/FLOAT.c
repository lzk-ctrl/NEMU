#include "FLOAT.h"

FLOAT F_mul_F(FLOAT a, FLOAT b) {
	long long c = (long long)a * (long long)b;
	return (FLOAT)(c >> 16);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
	FLOAT p, q;
	asm volatile("idiv %2" : "=a"(p), "=d"(q) : "r"(b), "a"(a << 16), "d"(a >> 16));
	return p;
}

FLOAT f2F(float a) {
	int b = *(int *)&a;
	int sign = b >> 31;
	int exp = (b >> 23) & 0xff;
	FLOAT c = b & 0x7fffff;
	if (exp != 0) {
		c += 1 << 23;
	}
	exp -= 150;
	if (exp < -16) {
		c >>= -16 - exp;
	}
	if (exp > -16) {
		c <<= exp + 16;
	}
	return sign == 0 ? c : -c;
}

FLOAT Fabs(FLOAT a) {
	FLOAT b;
	if (a > 0){
		b = a;
	} else {
		b = -a;

	}
	return b;
}
// ...

/* Functions below are already implemented */

FLOAT sqrt(FLOAT x) {
	FLOAT dt, t = int2F(2);

	do {
		dt = F_div_int((F_div_F(x, t) - t), 2);
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

FLOAT pow(FLOAT x, FLOAT y) {
	/* we only compute x^0.333 */
	FLOAT t2, dt, t = int2F(2);

	do {
		t2 = F_mul_F(t, t);
		dt = (F_div_F(x, t2) - t) / 3;
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}
