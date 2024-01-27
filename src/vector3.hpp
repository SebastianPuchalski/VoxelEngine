/*
Module: vector3
Author: Sebastian Puchalski
Date: 10.11.2012
*/

#ifndef VECTOR_3_HPP_
#define VECTOR_3_HPP_

#include "common.hpp"

#include <cmath>

template <class T>
class vector3
{
public:
	vector3(){}
	vector3(T x, T y, T z) : x(x), y(y), z(z) {}
	~vector3(){}

	T x, y, z;

	T length() const
	{
		return std::sqrt(x*x + y*y + z*z);
	}

	vector3 abs() const
	{
		return vector3(x > 0 ? x : -x, y > 0 ? y : -y, z > 0 ? z : -z);
	}

	vector3 operator+(const vector3 & rhs) const
	{
		return vector3(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	vector3 operator-(const vector3 & rhs) const
	{
		return vector3(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	vector3 operator*(T rhs) const
	{
		return vector3(x * rhs, y * rhs, z * rhs);
	}

	vector3 operator/(T rhs) const
	{
		return vector3(x / rhs, y / rhs, z / rhs);
	}

	T dot(const vector3 & rhs) const
	{
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	vector3 cross(const vector3 & rhs) const
	{
		return vector3(y * rhs.z - z * rhs.y, x * -rhs.z + z * rhs.x, x * rhs.y - y * rhs.x);
	}

	vector3 normalized() const
	{
		return *this * (1 / length());
	}

	vector3 normalize()
	{
		*this = *this * (1 / length());
		return *this;
	}
};

typedef vector3<float> vector3f;
typedef vector3<double> vector3d;
typedef vector3<long double> vector3ld;
typedef vector3<real> vector3r;
typedef vector3<int> vector3i;

#endif //VECTOR_3_HPP_
