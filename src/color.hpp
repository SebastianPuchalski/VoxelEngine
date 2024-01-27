/*
Module: color
Author: Sebastian Puchalski
Date: 11.11.2012
*/

#ifndef COLOR_HPP_
#define COLOR_HPP_

#include "common.hpp"

template <typename T>
class colorRGB
{
public:
	colorRGB(){}
	colorRGB(T r, T g, T b) : r(r), g(g), b(b){}
	~colorRGB(){}

	T r, g, b;

	colorRGB operator+(const colorRGB & rhs) const
	{
		return colorRGB(r + rhs.r, g + rhs.g, b + rhs.b);
	}
	colorRGB operator*(T rhs) const
	{
		return colorRGB(r * rhs, g * rhs, b * rhs);
	}
	colorRGB operator*(const colorRGB & rhs) const
	{
		return colorRGB(r * rhs.r, g * rhs.g, b * rhs.b);
	}
};

template <typename T>
class colorRGBA
{
public:
	colorRGBA(){}
	colorRGBA(T r, T g, T b, T a) : r(r), g(g), b(b), a(a){}
	~colorRGBA(){}

	T r, g, b, a;

	colorRGBA operator+(const colorRGBA & rhs) const
	{
		return colorRGBA(r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a);
	}
	colorRGBA operator*(T rhs) const
	{
		return colorRGBA(r * rhs, g * rhs, b * rhs, a *rhs);
	}
	colorRGBA operator*(const colorRGBA & rhs) const
	{
		return colorRGB(r * rhs.r, g * rhs.g, b * rhs.b, a * rhs.a);
	}
};

typedef colorRGB<uint8> color24; //24bit color
typedef colorRGB<float> color96; //96bit color

typedef colorRGBA<uint8> color32; //32bit color
typedef colorRGBA<float> color128; //128bit color

#endif //COLOR_HPP_
