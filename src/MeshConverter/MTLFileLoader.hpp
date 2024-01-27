/*
Module: MTLFileLoader
Author: Sebastian Puchalski
Date: 11.11.2012
*/

#ifndef MTL_FILE_LOADER_HPP_
#define MTL_FILE_LOADER_HPP_

#include "FileLoader.hpp"
#include "../color.hpp"

#include <vector>

class MTLFileLoader : public FileLoader
{
public:
	class Material
	{
	public:
		Material():
			ambientF(false), diffuseF(false), specularF(false), 
			alphaF(false), shininessF(false), illuminationF(false){}
		~Material(){}

		void setName(const std::string & name);
		bool setParameter(const std::string & parameter);

		std::string getName() const { return name; };
		bool getAmbient(color96 & ambient) const { ambient = this->ambient; return ambientF; }
		bool getDiffuse(color96 & diffuse) const { diffuse = this->diffuse; return diffuseF; }
		bool getSpecular(color96 & specular) const { specular = this->specular; return specularF; }
		bool getAlpha(float & alpha) const { alpha = this->alpha; return alphaF; }
		bool getShininess(float & shininess) const { shininess = this->shininess; return shininessF; }
		bool getIllumination(float & illumination) const { illumination = this->illumination; return illuminationF; }
		std::string getAmbientMap() const { return ambientMap; }
		std::string getDiffuseMap() const { return diffuseMap; }
		std::string getSpecularMap() const { return specularMap; }
		std::string getAlphaMap() const { return alphaMap; }

	private:
		std::string name;
		color96 ambient; bool ambientF;
		color96 diffuse; bool diffuseF;
		color96 specular; bool specularF;
		float alpha; bool alphaF;
		float shininess; bool shininessF;
		float illumination; bool illuminationF;
		std::string ambientMap;
		std::string diffuseMap;
		std::string specularMap;
		std::string alphaMap;
	};

	MTLFileLoader();
	virtual ~MTLFileLoader();

	bool loadFile(const std::string & fileName);
	void release();

	const std::vector<Material> & getMaterials() const;

private:
	std::vector<Material> materials;

	bool loadTextures() const;
	bool loadTexture(const std::string & fileName) const;
	void releaseTextures() const;
	void releaseTexture(const std::string & fileName) const;
};

#endif //MTL_FILE_LOADER_HPP_
