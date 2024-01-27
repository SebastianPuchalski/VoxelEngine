#include "Camera.hpp"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::setOrientation(const vector3f & direction, 
	const vector3f & up)
{
	z = direction.normalized();
	x = up.cross(z).normalized();
	y = z.cross(x).normalized();
}

void Camera::lookAt(const vector3f & position, const vector3f & center, 
	const vector3f & up)
{
	this->position = position;
	setOrientation(center - position, up);
}
