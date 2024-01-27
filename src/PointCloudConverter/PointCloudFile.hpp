/*
Module: PointCloudFile
Author: Sebastian Puchalski
Date: 27.05.2014
*/

#include "../vector3.hpp"
#include "../color.hpp"

#include <string>
#include <fstream>

#ifndef	POINT_CLOUD_FILE_HPP_
#define POINT_CLOUD_FILE_HPP_

class PointCloudFile
{
public:
	struct Point
	{
		vector3r position;
		vector3r normal;
		color96 color;
	};

	PointCloudFile();
	virtual ~PointCloudFile();

	bool open(const std::string & fileName);
	void close();

	uint64 getPointNumber() const;
	Point getPoint();

private:
	std::ifstream file;

	uint64 pointNumber;

	struct FilePoint
	{
		vector3r position;
		vector3r normal;
		color24 color;
	};

	uchar * buffer;
	int firstPoint;
	uint64 readedPointNumber;
};

#endif //POINT_CLOUD_FILE_HPP_
