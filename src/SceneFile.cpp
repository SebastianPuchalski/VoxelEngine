#include "SceneFile.hpp"

static const std::string HEAD = "SCN";
static const int HEAD_SIZE = 3;

SceneFileHeader::SceneFileHeader()
{
}

SceneFileHeader::~SceneFileHeader()
{
}

bool SceneFileHeader::load(std::ifstream & file)
{
	char head[HEAD_SIZE+1];
	head[HEAD_SIZE] = 0;
	file.read(head, HEAD_SIZE);
	if(head != HEAD)
		return false;

	uint8 depth;
	file.read((char *)&depth, 1);
	uint32 size;
	file.read((char *)&size, 4);
	if(file.fail() || file.eof())
		return false;
	treeDepth = depth;
	dataSize = size;
	return true;
}

bool SceneFileHeader::save(std::ofstream & file) const
{
	file.write(HEAD.c_str(), HEAD_SIZE);

	file.write((char *)&treeDepth, 1);
	file.write((char *)&dataSize, 4);
	if(file.fail())
		return false;
	return true;
}
