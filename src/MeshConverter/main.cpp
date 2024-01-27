/*
Module: main
Author: Sebastian Puchalski
Date: 11.11.2012
*/

#include "../common.hpp"
#include "Converter.hpp"

#include <iostream>

static const std::string OUTPUT_FILE_EXTENSION = ".scn";

std::string changeExtension(const std::string & fileName)
{
	int length = fileName.size();
	for(int i = length-1; i >= 0; i--)
		if(fileName[i] == '.')
			length = i;

	std::string newFileName = fileName;
	newFileName.erase(length, fileName.size() - length);
	newFileName += OUTPUT_FILE_EXTENSION;
	return newFileName;
}

int main(int argc, char * argv[])
{
	if(argc == 2)
	{
		try
		{
			Converter converter;
			if(converter.convert(argv[1], changeExtension(argv[1])))
				std::cout << "Convert succeeded!\n";
			else
				std::cout << "Convert failed!\n";
		}
		catch(...)
		{
			std::cout << "Unknown error!\n";
		}
	}
	else
		std::cout << "Wrong number of arguments!\n";

	char buffer;
	std::cin >> buffer;
	return 0;
}
