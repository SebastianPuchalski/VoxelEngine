/*
Module: TextureManager
Author: Sebastian Puchalski
Date: 14.11.2012
*/

#ifndef TEXTURE_MANAGER_HPP_
#define TEXTURE_MANAGER_HPP_

#include "../common.hpp"
#include "Texture.hpp"

#include <vector>
#include <utility>

class TextureManager
{
	static TextureManager instance;

	TextureManager();
	TextureManager(const TextureManager &);
	virtual ~TextureManager();
	void operator=(const TextureManager &);

public:
	static TextureManager & get(){ return instance; }

	bool loadTexture(const std::string & fileName, bool mipmapping = true);
	void releaseTexture(const std::string & fileName);

	Texture * getTexture(const std::string & fileName) const;

private:
	std::vector<std::pair<Texture *, uint> > textures;
};

#endif //TEXTURE_MANAGER_HPP_
