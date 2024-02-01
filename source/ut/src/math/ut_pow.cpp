//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "math/ut_pow.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Exp specialization for 'double' type.
template<> double Exp<double>(double a)
{
	return exp(a);
}

// Square root specialization for 'double' type.
template<> double Sqrt<double>(double a)
{
	return sqrt(a);
}

// Power specialization for 'double' type.
template<> double Pow(double base, double exp)
{
	return pow(base, exp);
}

// Logarithm specialization for 'double' type.
template<> double Logarithm<double>(double a)
{
	return log(a);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
