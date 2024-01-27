/*
Module: OBJFileLoader
Author: Sebastian Puchalski
Date: 10.11.2012
*/

#ifndef OBJ_FILE_LOADER_HPP_
#define OBJ_FILE_LOADER_HPP_

#include "FileLoader.hpp"
#include "../vector2.hpp"
#include "../vector3.hpp"
#include "MTLFileLoader.hpp"

#include <vector>

class OBJFileLoader : public FileLoader
{
public:
	struct Face
	{
		int32 vertex[3]; //if index is equal to -1 it is not specified in *.obj file
		int32 coord[3];
		int32 normal[3];
	};

	OBJFileLoader();
	virtual ~OBJFileLoader();

	bool loadFile(const std::string & fileName);
	void release();

	const std::vector<vector3r> & getVertices() const { return vertices; }
	const std::vector<vector2r> & getCoords() const { return coords; }
	const std::vector<vector3r> & getNormals() const { return normals; }
	const std::vector<Face> & getFaces() const { return faces; }
	const MTLFileLoader & getMaterialLibrary() const { return materialLibrary; }
	const std::vector<std::pair<int32, int32>> & getMaterialGroups() const { return materialGroups; }

private:
	bool addVertex(std::stringstream & ss);
	bool addCoord(std::stringstream & ss);
	bool addNormal(std::stringstream & ss);
	bool addFace(std::stringstream & ss);
	bool loadLibrary(std::stringstream & ss);
	bool useMaterial(std::stringstream & ss);

	std::vector<vector3r> vertices;
	std::vector<vector2r> coords;
	std::vector<vector3r> normals;
	std::vector<Face> faces;
	MTLFileLoader materialLibrary;
	std::vector<std::pair<int32, int32>> materialGroups; //pair(first index of face, material index)
};

#endif //OBJ_FILE_LOADER_HPP_
