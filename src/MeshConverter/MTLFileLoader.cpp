#include "MTLFileLoader.hpp"

#include "TextureManager.hpp"

#include <fstream>
#include <sstream>

void MTLFileLoader::Material::setName(const std::string & name)
{
	this->name = name;
}

bool MTLFileLoader::Material::setParameter(const std::string & parameter)
{
	std::stringstream ss(parameter);
	std::string buffer;
	ss >> buffer;

	if(buffer == "Ka")
	{
		ss >> ambient.r >> ambient.g >> ambient.b;
		ambientF = true;
		if(ss.fail() || !ss.eof())
			return false;
	}
	if(buffer == "Kd")
	{
		ss >> diffuse.r >> diffuse.g >> diffuse.b;
		diffuseF = true;
		if(ss.fail() || !ss.eof())
			return false;
	}
	if(buffer == "Ks")
	{
		ss >> specular.r >> specular.g >> specular.b;
		specularF = true;
		if(ss.fail() || !ss.eof())
			return false;
	}
	if(buffer == "d" || buffer == "Tr")
	{
		ss >> alpha;
		alphaF = true;
		if(ss.fail() || !ss.eof())
			return false;
	}
	if(buffer == "Ns")
	{
		ss >> shininess;
		shininessF = true;
		if(ss.fail() || !ss.eof())
			return false;
	}
	if(buffer == "illum")
	{
		ss >> illumination;
		illuminationF = true;
		if(ss.fail() || !ss.eof())
			return false;
	}
	if(buffer == "map_Ka")
	{
		ss >> ambientMap;
		if(ss.fail() || !ss.eof())
			return false;
	}
	if(buffer == "map_Kd")
	{
		ss >> diffuseMap;
		if(ss.fail() || !ss.eof())
			return false;
	}
	if(buffer == "map_Ks")
	{
		ss >> specularMap;
		if(ss.fail() || !ss.eof())
			return false;
	}
	if(buffer == "map_d")
	{
		ss >> alphaMap;
		if(ss.fail() || !ss.eof())
			return false;
	}

	return true;
}

MTLFileLoader::MTLFileLoader()
{
}

MTLFileLoader::~MTLFileLoader()
{
	release();
}

bool MTLFileLoader::loadFile(const std::string & fileName)
{
	if(materials.size() > 0)
		return false;

	std::ifstream file(fileName);
	if(!file.is_open())
		return false;

	std::string line;
	while(!getline(file, line).fail())
	{
		if(line.size() == 0)
			continue;
		std::stringstream ss(line);
		std::string buffer;
		ss >> buffer;

		if(buffer == "newmtl")
		{
			ss >> buffer;
			if(ss.fail() || !ss.eof())
				return false;
			Material material;
			material.setName(buffer);
			materials.push_back(material);
		}
		else
			if(materials.size() > 0)
				if(!materials[materials.size()-1].setParameter(line))
					return false;
	}
	return loadTextures();
}

void MTLFileLoader::release()
{
	releaseTextures();
	materials.clear();
}

const std::vector<MTLFileLoader::Material> & MTLFileLoader::getMaterials() const
{
	return materials;
}

bool MTLFileLoader::loadTextures() const
{
	for(int i = 0; i < materials.size(); i++)
	{
		if(!loadTexture(materials[i].getAmbientMap()))
			return false;
		if(!loadTexture(materials[i].getDiffuseMap()))
			return false;
		if(!loadTexture(materials[i].getSpecularMap()))
			return false;
		if(!loadTexture(materials[i].getAlphaMap()))
			return false;
	}
	return true;
}

bool MTLFileLoader::loadTexture(const std::string & fileName) const
{
	if(fileName.size() > 0)
		if(!TextureManager::get().loadTexture(fileName))
			return false;
	return true;
}

void MTLFileLoader::releaseTextures() const
{
	for(int i = 0; i < materials.size(); i++)
	{
		releaseTexture(materials[i].getAmbientMap());
		releaseTexture(materials[i].getDiffuseMap());
		releaseTexture(materials[i].getSpecularMap());
		releaseTexture(materials[i].getAlphaMap());
	}
}

void MTLFileLoader::releaseTexture(const std::string & fileName) const
{
	if(fileName.size() > 0)
		TextureManager::get().releaseTexture(fileName);
}
