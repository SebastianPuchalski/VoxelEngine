/*
Module: vector2
Author: Sebastian Puchalski
Date: 10.11.2012
*/

#ifndef VECTOR_2_HPP_
#define VECTOR_2_HPP_

#include "common.hpp"

#include <cmath>

template <class T>
class vector2
{
public:
	vector2(){}
	vector2(T x, T y) : x(x), y(y){}
	~vector2(){}

	T x, y;

	T length() const
	{
		return std::sqrt(x*x + y*y);
	}

	vector2 abs() const
	{
		return vector3(x > 0 ? x : -x, y > 0 ? y : -y);
	}

	vector2 operator+(const vector2 & rhs) const
	{
		return vector2(x + rhs.x, y + rhs.y);
	}

	vector2 operator-(const vector2 & rhs) const
	{
		return vector2(x - rhs.x, y - rhs.y);
	}

	vector2 operator*(T rhs) const
	{
		return vector2(x * rhs, y * rhs);
	}

	vector2 operator/(T rhs) const
	{
		return vector2(x / rhs, y / rhs);
	}

	vector2 normalized() const
	{
		return *this * (1 / length());
	}

	vector2 normalize()
	{
		*this = *this * (1 / length());
		return *this;
	}
};

typedef vector2<float> vector2f;
typedef vector2<double> vector2d;
typedef vector2<long double> vector2ld;
typedef vector2<real> vector2r;
typedef vector2<int> vector2i;

#endif //VECTOR_2_HPP_
