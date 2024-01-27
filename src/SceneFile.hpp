/*
Module: SceneFile
Author: Sebastian Puchalski
Date: 18.11.2012
*/

#ifndef SCENE_FILE_HPP_
#define SCENE_FILE_HPP_

#include "common.hpp"
#include "color.hpp"
#include "vector3.hpp"

#include <string>
#include <fstream>

class Voxel
{
public:
	struct Material
	{
		color24 diffuse;
		//uint8 alpha;
		//vector3<int8> normal;
		//uint8 specular;
	};
	Material material;
	uint8 mask;
};

typedef uint32 Pointer;

const int VOXEL_SIZE = 4; //size of voxel in file can be different than sizeof(Voxel) - careful alignment
const int POINTER_SIZE = sizeof(Pointer); //size of pointer in file

class SceneFileHeader
{
public:
	SceneFileHeader();
	virtual ~SceneFileHeader();

	bool load(std::ifstream & file);
	bool save(std::ofstream & file) const;

	uint8 getTreeDepth() const { return treeDepth; }
	uint32 getDataSize() const { return dataSize; }
	void setTreeDepth(uint8 depth){ treeDepth = depth; }
	void setDataSize(uint32 size){ dataSize = size; }

private:
	uint8 treeDepth;
	uint32 dataSize;
};

#endif //SCENE_FILE_HPP_
