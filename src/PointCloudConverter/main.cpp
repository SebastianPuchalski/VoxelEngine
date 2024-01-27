/*
Module: main
Author: Sebastian Puchalski
Date: 27.05.2014
*/

#include "../common.hpp"
#include "PointCloudConverter.hpp"

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
			PointCloudConverter converter;
			if(converter.convert(argv[1], changeExtension(argv[1])))
				std::cout << "Convert succeeded!\n";
			else
				std::cout << "\nConvert failed!\n";
		}
		catch(std::string e)
		{
			std::cout << "\n" << e;
		}
		catch(...)
		{
			std::cout << "\nUnknown error!\n";
		}
	}
	else
		std::cout << "Wrong number of arguments!\n";

	char buffer;
	std::cin >> buffer;
	return 0;
}

/*
#include <iostream>

int main()
{
	unsigned __int64 kb = 1024;
	char * data;
	try
	{
		while(true)
		{
			data = new char[kb * 1024];
			kb *= 1.1;
			delete [] data;
		}
	}
	catch(...)
	{
		std::cout << "Memory block size: " << kb / 1024 << "MB" << std::endl;
	}
	
	unsigned __int64 mb2;
	try
	{
		for(mb2 = 0;; mb2++)
			data = new char[1024 * 1024];
	}
	catch(...)
	{
		delete [] data;
		std::cout << "Total allocated memory: " << mb2 << "MB" << std::endl;
	}
	
	return 0;
}
*/
