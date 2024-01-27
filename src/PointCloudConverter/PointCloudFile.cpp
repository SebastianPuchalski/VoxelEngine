#include "PointCloudFile.hpp"

#include <iostream>
#include <sstream>

const int BUFFER_SIZE = 1024 * 256; //number of points

const int FILE_POINT_SIZE = 27;

PointCloudFile::PointCloudFile() : pointNumber(0), buffer(0)
{
}

PointCloudFile::~PointCloudFile()
{
}

bool PointCloudFile::open(const std::string & fileName)
{
	file.open(fileName, std::ios::binary);
	if(!file.is_open())
		return false;

	std::string line;
	do
	{
		std::stringstream ss;
		std::string word;

		std::getline(file, line);
		if(file.eof() || file.fail() || file.bad())
			return false;

		ss << line;
		ss >> word;
		if(word == "element")
		{
			ss >> word;
			if(word == "vertex")
				ss >> pointNumber;
		}

	}while(line != "end_header");

	if(buffer == 0)
		buffer = new uchar[(BUFFER_SIZE - 1) * FILE_POINT_SIZE + sizeof(FilePoint)];
	firstPoint = BUFFER_SIZE;
	readedPointNumber = 0;

	return true;
}

void PointCloudFile::close()
{
	if(buffer)
	{
		delete [] buffer;
		buffer = 0;
	}

	file.close();
	pointNumber = 0;
}

uint64 PointCloudFile::getPointNumber() const
{
	return pointNumber;
}

PointCloudFile::Point PointCloudFile::getPoint()
{
	if(firstPoint == BUFFER_SIZE)
	{
		int pointsToRead = (int)min((uint64)BUFFER_SIZE, pointNumber - readedPointNumber);
		readedPointNumber += pointsToRead;
		file.read((char *)buffer, pointsToRead * FILE_POINT_SIZE);
		firstPoint = 0;
	}

	Point dest;
	FilePoint & src = *(FilePoint *)((uchar *)buffer + firstPoint * FILE_POINT_SIZE);
	firstPoint++;

	dest.position.x = -src.position.y;
	dest.position.y = -src.position.x;
	dest.position.z = src.position.z;

	dest.normal = src.normal;
	dest.color = color96(
		src.color.r / 255.0f, 
		src.color.g / 255.0f, 
		src.color.b / 255.0f);

	return dest;
}
