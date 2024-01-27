/*
Module: Node
Author: Sebastian Puchalski
Date: 27.05.2014
*/

#ifndef NODE_HPP_
#define NODE_HPP_

#include "PointCloudFile.hpp"
#include "../SceneFile.hpp"

class Node
{
public:
	Node();
	~Node();

	void addPoint(PointCloudFile::Point & point, int depth);

	uint32 getSize() const;
	void save(std::ofstream & file, 
		int depth, int saveDepth, 
		uint32 & pointer) const;

	void getStats(uint & min, uint & max, uint & sum, uint & weight) const;

private:
	Node * children;
	uint8 childrenMask;

	vector3r normal;
	color96 color;
	real weight;

	Voxel::Material getMaterial() const;
	int countChildren(uint8 mask) const;
};

#endif //NODE_HPP_
