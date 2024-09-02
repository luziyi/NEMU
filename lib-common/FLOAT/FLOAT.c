#include "FLOAT.h"

FLOAT F_mul_F(FLOAT a, FLOAT b)
{
    long long c = (long long)a * (long long)b;
    return (FLOAT)(c >> 16);
}

FLOAT F_div_F(FLOAT a, FLOAT b)
{
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

    FLOAT p, q;
    asm volatile("idiv %2" : "=a"(p), "=d"(q) : "r"(b), "a"(a << 16), "d"(a >> 16));
    return p;
}

FLOAT f2F(float a)
{
    /* You should figure out how to convert `a' into FLOAT without
     * introducing x87 floating point instructions. Else you can
     * not run this code in NEMU before implementing x87 floating
     * point instructions, which is contrary to our expectation.
     *
     * Hint: The bit representation of `a' is already on the
     * stack. How do you retrieve it to another variable without
     * performing arithmetic operations on it directly?
     */

    int b = *(int *)&a;
    int sign = b >> 31;
    int exp = (b >> 23) & 0xff;
    FLOAT c = b & 0x7fffff;
    if (exp != 0)
    {
        c += 1 << 23;
    }
    exp -= 150;
    if (exp < -16)
    {
        c >>= -16 - exp;
    }
    if (exp > -16)
    {
        c <<= exp + 16;
    }
    return sign == 0 ? c : -c;
}

FLOAT Fabs(FLOAT a)
{
    FLOAT b;
    if (a > 0)
    {
        b = a;
    }
    else
    {
        b = -a;
    }
    return b;
}

/* Functions below are already implemented */

FLOAT sqrt(FLOAT x)
{
    FLOAT dt, t = int2F(2);

    do
    {
        dt = F_div_int((F_div_F(x, t) - t), 2);
        t += dt;
    } while (Fabs(dt) > f2F(1e-4));

    return t;
}

FLOAT pow(FLOAT x, FLOAT y)
{
    /* we only compute x^0.333 */
    FLOAT t2, dt, t = int2F(2);

    do
    {
        t2 = F_mul_F(t, t);
        dt = (F_div_F(x, t2) - t) / 3;
        t += dt;
    } while (Fabs(dt) > f2F(1e-4));

    return t;
}
