/*
Module: common
Author: Sebastian Puchalski
Date: 10.11.2012
*/

#ifndef COMMON_HPP_
#define COMMON_HPP_

typedef float real;

typedef unsigned int uint;
typedef unsigned char uchar;

typedef __int8 int8;
typedef unsigned __int8 uint8;
typedef __int16 int16;
typedef unsigned __int16 uint16;
typedef __int32 int32;
typedef unsigned __int32 uint32;
typedef __int64 int64;
typedef unsigned __int64 uint64;

template <class T>
T min(T arg1, T arg2){ return (arg1 < arg2) ? arg1 : arg2; }

template <class T>
T max(T arg1, T arg2){ return (arg1 > arg2) ? arg1 : arg2; }

template <class T>
T abs(T arg){ return arg > 0 ? arg : -arg; }

const long double LOG2E = 1.44269504088896340736;
template <class T>
T log2(T arg){ return std::log(arg) * LOG2E; }

const float FLOAT_MIN = -1E+37F;
const float FLOAT_MAX = 1E+37F;

const float PI = 3.14159265f;

#endif //COMMON_HPP_
