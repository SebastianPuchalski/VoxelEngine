/*
Module: box
Author: Sebastian Puchalski
Date: 11.11.2012
*/

#ifndef BOX_HPP_
#define BOX_HPP_

#include "common.hpp"

template <typename T>
class box
{
public:
	box(){}
	box(T left, T right, T bottom, T top, T back, T front) : 
	left(left), right(right), bottom(bottom), top(top), back(back), front(front){}
	~box(){}

	T left, right;
	T bottom, top;
	T back, front;

	T lengthX() const { return right - left; }
	T lengthY() const { return top - bottom; }
	T lengthZ() const { return front - back; }
	T minLength() const { return min(lengthX(), min(lengthY(), lengthZ())); }
	T maxLength() const { return max(lengthX(), max(lengthY(), lengthZ())); }

	bool checkIntersection(const box & b) const
	{
		return (
			left <= b.right && b.left <= right && 
			bottom <= b.top && b.bottom <= top &&
			back <= b.front && b.back <= front);
	}
	bool checkIntersection(const vector3<T> & point) const
	{
		return (
			(point.x >= left && point.x <= right) && 
			(point.y >= bottom && point.y <= top) && 
			(point.z >= back && point.z <= front));
	}
};

typedef box<float> boxf;
typedef box<double> boxd;
typedef box<long double> boxld;
typedef box<real> boxr;
typedef box<int> boxi;

#endif //BOX_HPP_
