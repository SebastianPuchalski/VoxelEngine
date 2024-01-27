/*
Module: LightSource
Author: Sebastian Puchalski
Date: 09.12.2012
*/

#ifndef LIGHT_SOURCE_HPP_
#define LIGHT_SOURCE_HPP_

#include "../vector3.hpp"

struct DirectionalLightSource
{
	DirectionalLightSource(){}
	DirectionalLightSource(vector3f direction, color96 ambient, color96 diffuse, color96 specular) : 
	direction(direction.normalized()), ambient(ambient), diffuse(diffuse), specular(specular){}
	~DirectionalLightSource(){}

	vector3f direction;
	color96 ambient;
	color96 diffuse;
	color96 specular;
};

struct PointLightSource
{
	PointLightSource(){}
	PointLightSource(vector3f position, color96 ambient, color96 diffuse, color96 specular) : 
	position(position), ambient(ambient), diffuse(diffuse), specular(specular){}
	~PointLightSource(){}

	vector3r position;
	color96 ambient;
	color96 diffuse;
	color96 specular;
};

#endif //LIGHT_SOURCE_HPP_
