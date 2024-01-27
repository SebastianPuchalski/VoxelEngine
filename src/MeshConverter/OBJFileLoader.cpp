#include "OBJFileLoader.hpp"

#include <fstream>
#include <sstream>

OBJFileLoader::OBJFileLoader()
{
}

OBJFileLoader::~OBJFileLoader()
{
	release();
}

bool OBJFileLoader::loadFile(const std::string & fileName)
{
	if(vertices.size() > 0)
		return false;

	std::ifstream file;
	file.open(fileName);
	if(!file.is_open())
		return false;

	std::string buffer;
	while(!getline(file, buffer).fail())
	{
		if(buffer.size() == 0) //skip blank line
			continue;
		std::stringstream ss(buffer);
		ss >> buffer;

		if(buffer == "v")
			if(!addVertex(ss))
				return false;
		if(buffer == "vt")
			if(!addCoord(ss))
				return false;
		if(buffer == "vn")
			if(!addNormal(ss))
				return false;
		if(buffer == "f")
			if(!addFace(ss))
				return false;
		if(buffer == "mtllib")
			if(!loadLibrary(ss))
				return false;
		if(buffer == "usemtl")
			if(!useMaterial(ss))
				return false;
		//if(buffer == "o");
		//if(buffer == "g");
		//if(buffer == "s");
	}

	return true;
}

void OBJFileLoader::release()
{
	vertices.clear();
	coords.clear();
	normals.clear();
	faces.clear();
	materialLibrary.release();
	materialGroups.clear();
}

bool OBJFileLoader::addVertex(std::stringstream & ss)
{
	vector3r vertex;
	ss >> vertex.x;
	ss >> vertex.y;
	ss >> vertex.z;
	vertex.x = -vertex.x;
	if(ss.fail())
		return false;
	vertices.push_back(vertex);
	return true;
}

bool OBJFileLoader::addCoord(std::stringstream & ss)
{
	vector2r coord;
	ss >> coord.x;
	ss >> coord.y;
	if(ss.fail())
		return false;
	coords.push_back(coord);
	return true;
}

bool OBJFileLoader::addNormal(std::stringstream & ss)
{
	vector3r normal;
	ss >> normal.x;
	ss >> normal.y;
	ss >> normal.z;
	normal.x = -normal.x;
	if(ss.fail())
		return false;
	normals.push_back(normal);
	return true;
}

bool OBJFileLoader::addFace(std::stringstream & ss)
{
	Face face;

	for(int i = 0; i < 3; i++)
	{
		face.coord[i] = 0;
		face.normal[i] = 0;

		std::string buffer;
		ss >> buffer;
		if(ss.fail())
			return false;
		if(i == 2 && !ss.eof()) //only triangles are supported
		{
			std::string tempBuffer;
			ss >> tempBuffer;
			if(tempBuffer != "")
				return false;
		}

		for(int j = 0; j < buffer.size(); j++)
		{
			if(buffer[j] == '/')
			{
				buffer[j] = ' ';
				if(j+1 < buffer.size())
					if(buffer[j+1] == '/')
						buffer.insert(j+1, "0");
			}
		}

		std::stringstream ssv(buffer);
		ssv >> face.vertex[i];
		if(ssv.fail())
			return false;
		if(ssv.eof())
			continue;
		ssv >> face.coord[i];
		if(ssv.fail())
			return false;
		if(ssv.eof())
			continue;
		ssv >> face.normal[i];
		if(ssv.fail())
			return false;
		if(!ssv.eof())
			return false;
	}

	for(int i = 0; i < 3; i++)
	{
		if(face.vertex[i] > vertices.size())
			return false;
		if(face.coord[i] > coords.size())
			return false;
		if(face.normal[i] > normals.size())
			return false;

		face.vertex[i]--;
		face.coord[i]--;
		face.normal[i]--;
	}

	faces.push_back(face);
	return true;
}

bool OBJFileLoader::loadLibrary(std::stringstream & ss)
{
	std::string fileName;
	ss >> fileName;
	if(ss.fail() || !ss.eof())
		return false;
	return materialLibrary.loadFile(fileName);
}

bool OBJFileLoader::useMaterial(std::stringstream & ss)
{
	std::string name;
	ss >> name;
	if(ss.fail() || !ss.eof())
		return false;

	for(int i = 0; i < materialLibrary.getMaterials().size(); i++)
	{
		if(materialLibrary.getMaterials()[i].getName() == name)
		{
			materialGroups.push_back(std::pair<int32, int32>(faces.size(), i));
			return true;
		}
	}
	return false;
}
