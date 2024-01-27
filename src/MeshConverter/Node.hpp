/*
Module: Node
Author: Sebastian Puchalski
Date: 14.11.2012
*/

#ifndef NODE_HPP_
#define NODE_HPP_

#include "../common.hpp"
#include "../color.hpp"
#include "../vector3.hpp"
#include "../box.hpp"
#include "OBJFileLoader.hpp"
#include "Texture.hpp"
#include "../SceneFile.hpp"

#include <fstream>

class Node
{
public:
	struct VoxelMaterial
	{
		color96 diffuse;
		float alpha;
		vector3r normal;
		float specular;
	};
	struct Point
	{
		vector3r position;
		VoxelMaterial material;
	};

	Node();
	~Node();

	void operator=(Node & rhs);

	//operates on triangles:
	bool create(VoxelMaterial & material, 
		const boxr & box, real sampleDistance, 
		int depth, int maxDepth, 
		const boxr * boxes, const std::vector<int32> & indexes, 
		const OBJFileLoader & mesh, 
		const std::vector<Point> & points);

	uint32 getSize() const;
	void save(std::ofstream & file, 
		int depth, int saveDepth, 
		uint32 & pointer) const;

private:
	Node * children;
	Voxel::Material material;
	uint8 childrenMask;

	boxr getChildBox(const boxr & parent, uint number) const; //number = {0, ..., 7}
	void refreshChildrenList(uint8 newChildrenMask);
	void calculateParentMaterial(VoxelMaterial & outputMaterial, 
		const VoxelMaterial * childrenMaterial) const;
	void trianglesToPoints(std::vector<Point> & points, real sampleDistance, 
		const std::vector<int32> & faceIndexes, const OBJFileLoader & mesh) const;
	void triangleToPoints(std::vector<Point> & points, real sampleDistance, 
		const OBJFileLoader::Face & face, const OBJFileLoader & mesh, 
		const MTLFileLoader::Material * material, 
		const Texture * ambientTexture, const Texture * diffuseTexture, 
		const Texture * specularTexture, const Texture * alphaTexture) const;
	int findLongestEdge(const OBJFileLoader::Face & face, const OBJFileLoader & mesh) const;
	vector3r obtainWeights(real a, real b, real f, real x, real y) const;
	void encodeMaterial(const VoxelMaterial & mat);
	void combinePoints(VoxelMaterial & outputMaterial, const std::vector<Point> points) const;
	int countChildren(uint8 mask) const;
};

#endif //NODE_HPP_
