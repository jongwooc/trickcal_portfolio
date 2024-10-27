#ifdef FIXED_POINT_ARITHMETIC

#define    FIXED_FRACBITS 25
#define    FIXED_INTBITS (32 - FIXED_FRACBITS)

#define FIXED_RESOLUTION (1 << FIXED_FRACBITS)
#define FIXED_INT_MASK (0xffffffffL << FIXED_FRACBITS)
#define FIXED_FRAC_MASK (~FIXED_INT_MASK)

typedef long fixedp;

// conversions for arbitrary fracbits
#define _short2q(x, fb)          ((fixedp)((x) << (fb)))
#define _int2q(x, fb)            ((fixedp)((x) << (fb)))
#define _long2q(x, fb)           ((fixedp)((x) << (fb)))
#define _float2q(x, fb)          ((fixedp)((x) * (1 << (fb))))
#define _double2q(x, fb)     ((fixedp)((x) * (1 << (fb))))

// conversions for default fracbits
#define short2q(x)           _short2q(x, FIXED_FRACBITS)
#define int2q(x)         _int2q(x, FIXED_FRACBITS)
#define long2q(x)            _long2q(x, FIXED_FRACBITS)
#define float2q(x)           _float2q(x, FIXED_FRACBITS)
#define double2q(x)          _double2q(x, FIXED_FRACBITS)

// conversions for arbitrary fracbits
#define _q2short(x, fb)      ((short)((x) >> (fb)))
#define _q2int(x, fb)        ((int)((x) >> (fb)))
#define _q2long(x, fb)       ((long)((x) >> (fb)))
#define _q2float(x, fb)      ((float)(x) / (1 << (fb)))
#define _q2double(x, fb) ((double)(x) / (1 << (fb)))

// conversions for default fracbits
#define q2short(x)           _q2short(x, FIXED_FRACBITS)
#define q2int(x)         _q2int(x, FIXED_FRACBITS)
#define q2long(x)            _q2long(x, FIXED_FRACBITS)
#define q2float(x)           _q2float(x, FIXED_FRACBITS)
#define q2double(x)          _q2double(x, FIXED_FRACBITS) 
// evaluates to the whole (integer) part of x
#define qipart(x)            q2long(x)

// evaluates to the fractional part of x
#define qfpart(x)            ((x) & FIXED_FRAC_MASK)

// Both operands in addition and subtraction must have the same fracbits.
// If you need to add or subtract fixed point numbers with different
// fracbits, then use q2q to convert each operand beforehand.
#define qadd(a, b) ((a) + (b))
#define qsub(a, b) ((a) - (b))

/**
 * q2q - convert one fixed point type to another
 * x - the fixed point number to convert
 * xFb - source fracbits
 * yFb - destination fracbits
 */
static INLINE fixedp q2q(fixedp x, int xFb, int yFb)
{
    if(xFb == yFb) {
        return x;
    } else if(xFb < yFb) {
        return x << (yFb - xFb);
    } else {
        return x >> (xFb - yFb);
    }
}

/**
 * Multiply two fixed point numbers with arbitrary fracbits
 * x - left operand
 * y - right operand
 * xFb - number of fracbits for X
 * yFb - number of fracbits for Y
 * resFb - number of fracbits for the result
 * 
 */
#define _qmul(x, y, xFb, yFb, resFb) ((fixedp)(((long long)(x) * (long long)(y)) >> ((xFb) + (yFb) - (resFb))))

/**
 * Fixed point multiply for default fracbits.
 */
#define qmul(x, y) _qmul(x, y, FIXED_FRACBITS, FIXED_FRACBITS, FIXED_FRACBITS)

// overflow check version
#define FADD(x, y) (q2float(qadd(float2q(x), float2q(y))) > (1 << FIXED_INTBITS))? (float)((1 << FIXED_INTBITS): q2float(qadd(float2q(x), float2q(y)))
#define FSUB(x, y) (q2float(qsub(float2q(x), float2q(y))) < (-1 << FIXED_INTBITS))? (float)(-1 << FIXED_INTBITS): q2float(qsub(float2q(x), float2q(y)))
#define FMUL(x, y) (q2float(qmul(float2q(x), float2q(y))) > (1 << FIXED_INTBITS))? (q2float(qmul(float2q(x), float2q(y))) < (-1 << FIXED_INTBITS))? (float)(1 << FIXED_INTBITS): (float)(-1 << FIXED_INTBITS): q2float(qmul(float2q(x), float2q(y)))

//#define FADD(x, y) q2float((qadd(float2q(x), float2q(y))))
//#define FSUB(x, y) q2float((qsub(float2q(x), float2q(y))))
//#define FMUL(x, y) q2float((qmul(float2q(x), float2q(y))))

#else

#define FADD(x, y) (x + y)
#define FSUB(x, y) (x - y)
#define FMUL(x, y) (x * y)

#endif
