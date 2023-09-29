#include "FLOAT.h"

FLOAT F_mul_F(FLOAT a, FLOAT b) {
	long long temp=(long long)a*(long long)b;
	FLOAT ans=(FLOAT)(temp>>=16);
	return ans;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
	/* Dividing two 64-bit integers needs the support of another library
	 * `libgcc', other than newlib. It is a dirty work to port `libgcc'
	 * to NEMU. In fact, it is unnecessary to perform a "64/64" division
	 * here. A "64/32" division is enough.
	 *
	 * To perform a "64/32" division, you can use the x86 instruction
	 * `div' or `idiv' by inline assembly. We provide a template for you
	 * to prevent you from uncessary details.
	 *
	 *     asm volatile ("??? %2" : "=a"(???), "=d"(???) : "r"(???), "a"(???), "d"(???));
	 *
	 * If you want to use the template above, you should fill the "???"
	 * correctly. For more information, please read the i386 manual for
	 * division instructions, and search the Internet about "inline assembly".
	 * It is OK not to use the template above, but you should figure
	 * out another way to perform the division.
	 */
	if(b==0) nemu_assert(0);
	int flag1=0,flag2=0;
 	if(a<0) {
		a=-a;
		flag1=1;
	}
	if(b<0) {
		b=-b;
		flag2=1;
	}
	int ans=a/b;
	int remain=a%b;
	int i=16;
	while(i--) {
		ans<<=1;
		remain<<=1;
		if(remain>=b) {
			ans+=(remain/b);
			remain%=b;
		}
	}
	if(flag1^flag2) ans=-ans;
	return ans;
}

FLOAT f2F(float a) {
	/* You should figure out how to convert `a' into FLOAT without
	 * introducing x87 floating point instructions. Else you can
	 * not run this code in NEMU before implementing x87 floating
	 * point instructions, which is contrary to our expectation.
	 *
	 * Hint: The bit representation of `a' is already on the
	 * stack. How do you retrieve it to another variable without
	 * performing arithmetic operations on it directly?
	 */

	int temp=*(int*)&a;
	int flag=(temp>>31)&0x1;
	int exp=(temp>>23)&0xff;
    	int ans=temp&0x7fffff;
	ans+=0x800000;
	if(exp==0) return 0;
	else if(exp==0xff) {
		if(flag) return -0x7fffffff;
		else return 0x7fffffff;
	}
	else {
		exp-=127;
		exp-=23;
		if(exp>-16) ans<<=(exp+16);
		else ans>>=(-exp-16);
	}
	if(flag) ans=-ans;
	return ans;
}

FLOAT Fabs(FLOAT a) {
	if(a<0) a=-a;
	return a;
}

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

