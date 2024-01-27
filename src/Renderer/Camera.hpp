/*
Module: Camera
Author: Sebastian Puchalski
Date: 22.11.2012
*/

#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "../vector3.hpp"

class Camera
{
public:
	Camera();
	virtual ~Camera();

	void setPosition(const vector3f & position){ this->position = position; }
	void setX(const vector3f & x){ this->x = x; }
	void setY(const vector3f & y){ this->y = y; }
	void setZ(const vector3f & z){ this->z = z; }
	void setOrientation(const vector3f & direction, 
		const vector3f & up = vector3f(0.0f, 1.0f, 0.0f)); //left-handed coordinate system
	void lookAt(const vector3f & position, const vector3f & center, 
		const vector3f & up = vector3f(0.0f, 1.0f, 0.0f)); //left-handed coordinate system

	vector3f getPosition() const { return position; }
	vector3f getX() const { return x; }
	vector3f getY() const { return y; }
	vector3f getZ() const { return z; }

private:
	vector3f position;
	vector3f x, y, z; //orientation
};

#endif //CAMERA_HPP_
