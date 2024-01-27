#include "Converter.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

static const std::string CONFIGURATION_FILE_NAME = "MeshConverter.conf";

static const real LEVEL_COEFFICIENT = 0.5; //0 - big faces, 1 - average faces

bool Converter::Configuration::load(const std::string & fileName)
{
	std::ifstream file(fileName);
	if(!file.is_open())
		return false;

	std::string buffer;
	
	if(getline(file, buffer).fail())
		return false;
	{
		std::stringstream ss(buffer);
		ss >> treeDepth;
		if(ss.fail())
			return false;
	}

	if(getline(file, buffer).fail())
		return false;
	{
		std::stringstream ss(buffer);
		ss >> samplingFactor;
		if(ss.fail())
			return false;
	}

	return true;
}

Converter::Converter() : boxes(0), root(0)
{
}

Converter::~Converter()
{
}

bool Converter::convert(const std::string & inputFileName, const std::string & outputFileName)
{
	if(!configuration.load(CONFIGURATION_FILE_NAME))
		return false;

	bool good = true;
	good &= mesh.loadFile(inputFileName);
	if(good)
		std::cout << "Mesh file is loaded!\n";
	else
		std::cout << "Error during loading of *.obj file!\n";
	if(good)
	{
		good &= generateBoxes(); //allocation (boxes)
		if(good)
		{
			good &= buildTree(); //allocation (root)
			if(good)
				std::cout << "Tree is created!\n";
			else
				std::cout << "Error during creating of tree!\n";
			if(good)
			{
				good &= saveTree(outputFileName);
				if(good)
					std::cout << "Tree is saved to file!\n";
				else
					std::cout << "Error during writing to file!\n";
				delete root;
				root = 0;
			}
			delete [] boxes;
			boxes = 0;
		}
	}
	mesh.release(); //even if loading of file failed, mesh contains some data which should be released
	return good;
}

bool Converter::generateBoxes() //this method allocates memory (boxes)
{
	int faceAmount = mesh.getFaces().size();
	if(faceAmount == 0)
		return false;
	boxes = new boxr[faceAmount];
	for(int i = 0; i < faceAmount; i++)
	{
		vector3r vertex = mesh.getVertices()[mesh.getFaces()[i].vertex[0]];
		boxes[i].left = boxes[i].right = vertex.x;
		boxes[i].bottom = boxes[i].top = vertex.y;
		boxes[i].back = boxes[i].front = vertex.z;
		for(int j = 1; j < 3; j++)
		{
			vector3r vertex = mesh.getVertices()[mesh.getFaces()[i].vertex[j]];
			boxes[i].left = min(boxes[i].left, vertex.x);
			boxes[i].right = max(boxes[i].right, vertex.x);
			boxes[i].bottom = min(boxes[i].bottom, vertex.y);
			boxes[i].top = max(boxes[i].top, vertex.y);
			boxes[i].back = min(boxes[i].back, vertex.z);
			boxes[i].front = max(boxes[i].front, vertex.z);
		}
	}
	return true;
}

bool Converter::buildTree() //this method allocates memory (node)
{
	root = new Node();

	Node::VoxelMaterial material; //useless
	std::vector<int32> indexes;
	for(int i = 0; i < mesh.getFaces().size(); i++)
		indexes.push_back(i);

	boxr mainBox = obtainMainBox();
	real sampleDistance = calculateVoxelSize(mainBox, configuration.getTreeDepth()) / 
		configuration.getSamplingFactor();
	return root->create(material, 
		mainBox, sampleDistance, 
		0, configuration.getTreeDepth(), 
		boxes, indexes, 
		mesh, 
		std::vector<Node::Point>());
}

bool Converter::saveTree(const std::string & outputFileName) const
{
	if(root == 0)
		return false;
	std::ofstream file(outputFileName, std::ios::binary);
	if(!file.is_open())
		return false;

	SceneFileHeader header;
	header.setTreeDepth(configuration.getTreeDepth());
	header.setDataSize(POINTER_SIZE + root->getSize());
	if(!header.save(file))
		return false;

	uint32 pointer = VOXEL_SIZE + POINTER_SIZE;
	file.write((char *)&pointer, POINTER_SIZE);
	for(int i = 0; i < configuration.getTreeDepth(); i++)
		root->save(file, 0, i, pointer);
	pointer = 0;
	root->save(file, 0, configuration.getTreeDepth(), pointer);
	return true;
}

boxr Converter::obtainMainBox() const
{
	boxr mainBox;

	vector3r vertex = mesh.getVertices()[0];
	mainBox.left = mainBox.right = vertex.x;
	mainBox.bottom = mainBox.top = vertex.y;
	mainBox.back = mainBox.front = vertex.z;
	for(int i = 1; i < mesh.getVertices().size(); i++)
	{
		vector3r vertex = mesh.getVertices()[i];
		mainBox.left = min(mainBox.left, vertex.x);
		mainBox.right = max(mainBox.right, vertex.x);
		mainBox.bottom = min(mainBox.bottom, vertex.y);
		mainBox.top = max(mainBox.top, vertex.y);
		mainBox.back = min(mainBox.back, vertex.z);
		mainBox.front = max(mainBox.front, vertex.z);
	}

	real length = mainBox.right - mainBox.left;
	length = max(length, mainBox.top - mainBox.bottom);
	length = max(length, mainBox.front - mainBox.back);

	//length *= 1.15;

	mainBox.right = mainBox.left + length;
	mainBox.top = mainBox.bottom + length;
	mainBox.front = mainBox.back + length;

	return mainBox;
}

real Converter::calculateVoxelSize(const boxr & mainBox, int depth) const
{
	return std::pow(0.5, depth) * (mainBox.right - mainBox.left);
}
