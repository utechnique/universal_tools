//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "math/ut_trigonometry.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Specialization for double type.
template<> double Sin<double>(const double x) { return sin(x); }
template<> double ArcSin<double>(const double x) { return asin(x); }
template<> double Cos<double>(double x) { return cos(x); }
template<> double ArcCos<double>(double x) { return acos(x); }
template<> double Tan<double>(double x) { return tan(x); }
template<> double ArcTan<double>(double x) { return atan(x); }
template<> double ArcTan2<double>(double y, double x) { return atan2(y, x); }

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
