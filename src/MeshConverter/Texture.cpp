#include "Texture.hpp"

#include <stb_image.c>

Texture::Texture() : levelNumber(0), data(0)
{
}

Texture::~Texture()
{
	release();
}

bool Texture::load(const std::string & fileName)
{
	FILE * file = fopen(fileName.c_str(), "rb");
	if(!file)
		return false;

	int comp;
	int width, height;
	uchar * fileData;
	fileData = stbi_load_from_file(file, &width, &height, &comp, 0);
	fclose(file);
	if(fileData == 0)
		return false;

	if(width != height) //width must be equal to height
	{
		stbi_image_free(fileData);
		return false;
	}
	int level = log(width);
	if(level == -1) //size is power of 2
	{
		stbi_image_free(fileData);
		return false;
	}
	if(comp != 3) //supports only RGB images
	{
		stbi_image_free(fileData);
		return false;
	}

	this->fileName = fileName;
	levelNumber = level+1;
	data = new color24 * [levelNumber];

	int size = width;
	for(int i = level; i >= 0; i--)
	{
		data[i] = new color24[size*size];
		if(i == level)
		{
			for(int j = 0; j < size*size; j++)
			{
				data[i][j].r = fileData[j*3];
				data[i][j].g = fileData[j*3+1];
				data[i][j].b = fileData[j*3+2];
			}
		}
		else
		{
			for(int y = 0; y < size; y++)
				for(int x = 0; x < size; x++)
					data[i][y*size+x] = averageColor(
						data[i+1][(y*2)*size*2+x*2], 
						data[i+1][(y*2)*size*2+x*2+1], 
						data[i+1][(y*2+1)*size*2+x*2], 
						data[i+1][(y*2+1)*size*2+x*2+1]);
		}
		size /= 2;
	}

	stbi_image_free(fileData);
	return true;
}

void Texture::release()
{
	if(fileName.size() > 0)
	{
		for(int i = 0; i < levelNumber; i++)
			delete [] data[i];
		delete [] data;
		data = 0;
		fileName = "";
		levelNumber = 0;
	}
}

color96 Texture::sample(const vector2r & coord, int level) const
{
	if(level >= levelNumber)
		level = levelNumber-1;

	vector2r c(coord.x - floor(coord.x), coord.y - floor(coord.y)); //repeat
	//vector2r c(min(max(coord.x, (real)0), (real)1), min(max(coord.y, (real)0), (real)1)); //clamp
	c.y = 1-c.y; //(0, 0) is left bottom corner

	int size = pow2(level);
	vector2r tsc(c.x * size - 0.5, c.y * size - 0.5);
	vector2i indexes(tsc.x, tsc.y);
	vector2r weights(tsc.x - indexes.x, tsc.y - indexes.y);

	int fxi = indexes.x, sxi = indexes.x+1;
	int fyi = indexes.y, syi = indexes.y+1;
	if(fxi < 0) fxi = 0;
	if(fyi < 0) fyi = 0;
	if(sxi == size) sxi = size-1;
	if(syi == size) syi = size-1;

	return
		convertColor(data[level][fyi*size+fxi])*((1-weights.x)*(1-weights.y)) + 
		convertColor(data[level][fyi*size+sxi])*(weights.x*(1-weights.y)) +
		convertColor(data[level][syi*size+fxi])*((1-weights.x)*weights.y) + 
		convertColor(data[level][syi*size+sxi])*(weights.x*weights.y);
}

int Texture::log(uint32 arg) const
{
	for(int i = 31; i >= 0; i--)
	{
		if(arg & (1 << i))
		{
			for(int j = i-1; j >= 0; j--)
				if(arg & (1 << j))
					return -1;
			return i;
		}
	}
	return -1; //arg is equal to 0
}

color24 Texture::averageColor(
		const color24 & c1, const color24 & c2, 
		const color24 & c3, const color24 & c4) const
{
	uint r = c1.r + c2.r + c3.r + c4.r;
	uint g = c1.g + c2.g + c3.g + c4.g;
	uint b = c1.b + c2.b + c3.b + c4.b;
	return color24((r+2)/4, (g+2)/4, (b+2)/4);
}

int Texture::pow2(int exponent) const
{
	int value = 1;
	for(int i = 0; i < exponent; i++)
		value *= 2;
	return value;
}

color96 Texture::convertColor(const color24 & color) const
{
	return color96(color.r/255.0, color.g/255.0, color.b/255.0);
}
