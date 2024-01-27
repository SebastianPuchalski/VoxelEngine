#include "TextureManager.hpp"

TextureManager TextureManager::instance;

TextureManager::TextureManager()
{
}

TextureManager::~TextureManager()
{
	for(int i = 0; i < textures.size(); i++)
		delete textures[i].first;
}

bool TextureManager::loadTexture(const std::string & fileName, bool mipmapping)
{
	for(int i = 0; i < textures.size(); i++)
	{
		if(textures[i].first->getFileName() == fileName)
		{
			textures[i].second++;
			return true;
		}
	}

	Texture * texture = new Texture;
	if(!texture->load(fileName))
	{
		delete texture;
		return false;
	}
	textures.push_back(std::pair<Texture *, uint>(texture, 1));
	return true;
}

void TextureManager::releaseTexture(const std::string & fileName)
{
	for(int i = 0; i < textures.size(); i++)
	{
		if(textures[i].first->getFileName() == fileName)
		{
			textures[i].second--;
			if(textures[i].second == 0)
			{
				delete textures[i].first;
				textures.erase(textures.begin()+i);
			}
		}
	}
}

Texture * TextureManager::getTexture(const std::string & fileName) const
{
	if(fileName.size() == 0)
		return 0;
	for(int i = 0; i < textures.size(); i++)
		if(textures[i].first->getFileName() == fileName)
			return textures[i].first;
	return 0;
}
