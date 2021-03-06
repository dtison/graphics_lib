// GTOOLS.CPP - G42Trash derived from G42PathComponent
/*-----------------------------------------------------------------------------
	Modifications:
-----------------------------------------------------------------------------*/
#include "gtools.h"
#include "math.h"
/*-----------------------------------------------------------------------------
	Function - G42SquareRoot
	Parameters:
		int32 val - The value to find the square root of.
	Returns:
		int16 - The "integer" square root of val.
-----------------------------------------------------------------------------*/
int16 G42SquareRoot(int32 val)
{
	uintf mask = 0x4000;   // Bit mask to shift right
	int best = 0;         // Best estimate so far
	for (; mask; mask >>= 1)
		if (((long)best + mask) * (best + mask) <= val)
			best |= mask;
	return int16(best);
}
/*-----------------------------------------------------------------------------
	Function - G42SquareRoot
	Parameters:
		double val - The value to find the square root of.
	Returns:
		double - The square root of val.
-----------------------------------------------------------------------------*/
double G42SquareRoot (double val)
{
	return sqrt(val);
}
/*-----------------------------------------------------------------------------
	Function - G42SquareRoot
	Parameters:
		long double val - The value to find the square root of.
	Returns:
		long double - The square root of val.
-----------------------------------------------------------------------------*/
long double G42SquareRoot (long double val)
{
	#ifdef __WATCOMC__
	return (long double)sqrt((double)val);
	#else
	return sqrtl(val);
	#endif
}
