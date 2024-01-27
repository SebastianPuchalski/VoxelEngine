/*
Module: ImageFileLoader
Author: Sebastian Puchalski
Date: 10.11.2012
*/

#ifndef IMAGE_FILE_LOADER_HPP_
#define IMAGE_FILE_LOADER_HPP_

#include "FileLoader.hpp"

class ImageFileLoader : public FileLoader
{
public:
	ImageFileLoader();
	virtual ~ImageFileLoader();

	bool loadFile(const std::string & fileName);
	void release();

private:
};

#endif //IMAGE_FILE_LOADER_HPP_
