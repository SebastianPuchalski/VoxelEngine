#include "PointCloudConverter.hpp"
#include "../SceneFile.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

static const std::string CONFIGURATION_FILE_NAME = "PointCloudConverter.conf";

static const int PROGRESS_BAR_SIZE = 40;
static const std::string BAR_ELEMENT = "#";

bool PointCloudConverter::Configuration::load(const std::string & fileName)
{
	std::ifstream file(fileName);
	if(!file.is_open())
		return false;

	std::string buffer;
	
	if(getline(file, buffer).fail())
		return false;

	std::stringstream ss(buffer);
	ss >> treeDepth;
	if(ss.fail())
		return false;

	return true;
}

PointCloudConverter::PointCloudConverter() : root(0)
{
}

PointCloudConverter::~PointCloudConverter()
{
}

bool PointCloudConverter::convert(const std::string & inputFileName, const std::string & outputFileName)
{
	if(!configuration.load(CONFIGURATION_FILE_NAME))
		return false;

	bool good = true;
	good = good && buildTree(inputFileName);
	if(!good)
		std::cout << "\nError during creating of tree!\n";

	//uint lmin, lmax, lsum, lweight;
	//root->getStats(lmin, lmax, lsum, lweight);

	//std::cout << "Stats:\nmin: " << lmin << ", max: " << lmax << ", average: " << lsum / (float)lweight << std::endl;

	if(good)
	{
		good = good && saveTree(outputFileName);
		if(!good)
			std::cout << "\nError during writing to file!\n";
		delete root;
		root = 0;
	}
	return good;
}

bool PointCloudConverter::buildTree(const std::string & inputFileName)
{
	PointCloudFile pointCloud;

	if(!pointCloud.open(inputFileName))
		return false;
	vector3r mainBoxPosition; //left bottom back corner
	real mainBoxSize;
	if(!obtainMainBox(pointCloud, mainBoxPosition, mainBoxSize))
		return false;
	pointCloud.close();

	root = new Node;
	if(!pointCloud.open(inputFileName))
		return false;

	int lastUnitProgress = 0;
	std::cout << "Tree building progress:         ";

	for(uint i = 0; i < pointCloud.getPointNumber(); i++)
	{
		float progress = (i + 1.0f) / pointCloud.getPointNumber();
		int unitProgress = (int)(progress * PROGRESS_BAR_SIZE);
		if(unitProgress != lastUnitProgress)
		{
			std::cout << BAR_ELEMENT;
			lastUnitProgress = unitProgress;
		}

		PointCloudFile::Point point = pointCloud.getPoint();
		point.position = (point.position - mainBoxPosition) / mainBoxSize;
		root->addPoint(point, configuration.getTreeDepth());
	}

	std::cout << " DONE!\n";

	pointCloud.close();

	return true;
}

bool PointCloudConverter::saveTree(const std::string & outputFileName) const
{
	if(root == 0)
		return false;

	std::ofstream file(outputFileName, std::ios::binary);
	if(!file.is_open())
		return false;

	std::cout << "Tree saving progress:           ";

	SceneFileHeader header;
	header.setTreeDepth(configuration.getTreeDepth());
	header.setDataSize(POINTER_SIZE + root->getSize());
	if(!header.save(file))
		return false;

	for(int i = 0; i < PROGRESS_BAR_SIZE / 3; i++)
		std::cout << BAR_ELEMENT;

	uint32 pointer = VOXEL_SIZE + POINTER_SIZE;
	file.write((char *)&pointer, POINTER_SIZE);
	for(int i = 0; i < configuration.getTreeDepth(); i++)
		root->save(file, 0, i, pointer);
	pointer = 0;
	root->save(file, 0, configuration.getTreeDepth(), pointer);

	for(int i = 0; i < (PROGRESS_BAR_SIZE * 2 / 3 + PROGRESS_BAR_SIZE % 3); i++)
		std::cout << BAR_ELEMENT;
	std::cout << " DONE!\n";

	return true;
}

bool PointCloudConverter::obtainMainBox(PointCloudFile & pointCloud, vector3r & position, real & size) const
{
	if(pointCloud.getPointNumber() == 0)
		return false;

	boxr mainBox;

	vector3r point = pointCloud.getPoint().position;
	mainBox.left = mainBox.right = point.x;
	mainBox.bottom = mainBox.top = point.y;
	mainBox.back = mainBox.front = point.z;

	int lastUnitProgress = 0;
	std::cout << "Bounding box building progress: ";

	for(uint i = 1; i < pointCloud.getPointNumber(); i++)
	{
		float progress = (i + 1.0f) / pointCloud.getPointNumber();
		int unitProgress = (int)(progress * PROGRESS_BAR_SIZE);
		if(unitProgress != lastUnitProgress)
		{
			std::cout << BAR_ELEMENT;
			lastUnitProgress = unitProgress;
		}

		vector3r point = pointCloud.getPoint().position;
		mainBox.left = min(mainBox.left, point.x);
		mainBox.right = max(mainBox.right, point.x);
		mainBox.bottom = min(mainBox.bottom, point.y);
		mainBox.top = max(mainBox.top, point.y);
		mainBox.back = min(mainBox.back, point.z);
		mainBox.front = max(mainBox.front, point.z);
	}

	std::cout << " DONE!\n";

	size = mainBox.right - mainBox.left;
	size = max(size, mainBox.top - mainBox.bottom);
	size = max(size, mainBox.front - mainBox.back);

	position = vector3r(mainBox.left, mainBox.bottom, mainBox.back);

	return true;
}
