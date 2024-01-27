/*
Module: FileLoader
Author: Sebastian Puchalski
Date: 10.11.2012
*/

#ifndef FILE_LOADER_HPP_
#define FILE_LOADER_HPP_

#include <string>

class FileLoader
{
public:
	virtual bool loadFile(const std::string & fileName) = 0;
	virtual void release() = 0;
};

#endif //FILE_LOADER_HPP_
