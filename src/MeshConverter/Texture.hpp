/*
Module: Texture
Author: Sebastian Puchalski
Date: 15.11.2012
*/

#ifndef TEXTURE_HPP_
#define TEXTURE_HPP_

#include "../common.hpp"
#include "../color.hpp"
#include "../vector2.hpp"

#include <string>

class Texture
{
public:
	Texture();
	virtual ~Texture();

	bool load(const std::string & fileName);
	void release();

	color96 sample(const vector2r & coord, int level = 1024) const; //(0, 0) is left bottom corner
	std::string getFileName() const { return fileName; }

private:
	std::string fileName;
	int levelNumber;
	color24 ** data;

	int log(uint32 arg) const;
	color24 averageColor(
		const color24 & c1, const color24 & c2, 
		const color24 & c3, const color24 & c4) const;
	int pow2(int exponent) const;
	color96 convertColor(const color24 & color) const;
};

#endif //TEXTURE_HPP_
