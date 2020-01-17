//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut_cat.h"
#include "ut_inc.h"
#include "ut_dec.h"
#include "ut_if.h"
//----------------------------------------------------------------------------//
// Emprty macro
#define UT_PP_EMPTY()
#define UT_PP_EMPTY_I(__m0)
#define UT_PP_EMPTY_II(__m0, __m1)
#define UT_PP_EMPTY_III(__m0, __m1, __m2)
#define UT_PP_EMPTY_IV(__m0, __m1, __m2, __m3)
#define UT_PP_EMPTY_V(__m0, __m1, __m2, __m3, __m4)

// Use UT_PP_COMMA_IF macro for the first entry in the UT_PP_ENUM() enumeration.
#define UT_PP_COMMA() ,

#define UT_PP_COMMA_IF(cond) UT_PP_COMMA_IF_I(cond)
#define UT_PP_COMMA_IF_I(cond) UT_PP_IF(cond, UT_PP_COMMA, UT_PP_EMPTY)()

// colon
#define UT_PP_COLON() :
#define UT_PP_COLON_IF(cond) UT_PP_COLON_IF_I(cond)
#define UT_PP_COLON_IF_I(cond) UT_PP_IF(cond, UT_PP_COLON, UT_PP_EMPTY)()

// enumeration
#define UT_PP_ENUM(__count, __macro) UT_PP_ENUM_00(I(UT_PP_ENUM_,__count(__macro)))
#define UT_PP_ENUM_00(__par) UT_PP_ENUM_##__par
#define UT_PP_ENUM_I(a, b) UT_PP_ENUM_II(a##b)
#define UT_PP_ENUM_II(res) res

#define UT_PP_ENUM_0(_m)
#define UT_PP_ENUM_1(_m) _m(0)
#define UT_PP_ENUM_2(_m) _m(0) _m(1)
#define UT_PP_ENUM_3(_m) _m(0) _m(1) _m(2)
#define UT_PP_ENUM_4(_m) _m(0) _m(1) _m(2) _m(3)
#define UT_PP_ENUM_5(_m) _m(0) _m(1) _m(2) _m(3) _m(4)
#define UT_PP_ENUM_6(_m) _m(0) _m(1) _m(2) _m(3) _m(4) _m(5)
#define UT_PP_ENUM_7(_m) _m(0) _m(1) _m(2) _m(3) _m(4) _m(5) _m(6)
#define UT_PP_ENUM_8(_m) _m(0) _m(1) _m(2) _m(3) _m(4) _m(5) _m(6) _m(7)
#define UT_PP_ENUM_9(_m) _m(0) _m(1) _m(2) _m(3) _m(4) _m(5) _m(6) _m(7) _m(8)
#define UT_PP_ENUM_10(_m) _m(0) _m(1) _m(2) _m(3) _m(4) _m(5) _m(6) _m(7) _m(8) _m(9)

// enumeration inside another enumeration
#define UT_PP_ENUM_IN(__count, __macro) UT_PP_ENUM_IN_00(I(UT_PP_ENUM_IN_,__count(__macro)))
#define UT_PP_ENUM_IN_00(__par) UT_PP_ENUM_IN_##__par
#define UT_PP_ENUM_IN_I(a, b) UT_PP_ENUM_IN_II(a##b)
#define UT_PP_ENUM_IN_II(res) res

#define UT_PP_ENUM_IN_0(_m)
#define UT_PP_ENUM_IN_1(_m) _m(0)
#define UT_PP_ENUM_IN_2(_m) _m(0) _m(1)
#define UT_PP_ENUM_IN_3(_m) _m(0) _m(1) _m(2)
#define UT_PP_ENUM_IN_4(_m) _m(0) _m(1) _m(2) _m(3)
#define UT_PP_ENUM_IN_5(_m) _m(0) _m(1) _m(2) _m(3) _m(4)
#define UT_PP_ENUM_IN_6(_m) _m(0) _m(1) _m(2) _m(3) _m(4) _m(5)
#define UT_PP_ENUM_IN_7(_m) _m(0) _m(1) _m(2) _m(3) _m(4) _m(5) _m(6)
#define UT_PP_ENUM_IN_8(_m) _m(0) _m(1) _m(2) _m(3) _m(4) _m(5) _m(6) _m(7)
#define UT_PP_ENUM_IN_9(_m) _m(0) _m(1) _m(2) _m(3) _m(4) _m(5) _m(6) _m(7) _m(8)
#define UT_PP_ENUM_IN_10(_m) _m(0) _m(1) _m(2) _m(3) _m(4) _m(5) _m(6) _m(7) _m(8) _m(9)

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//