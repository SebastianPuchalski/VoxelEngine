/*
Module: Converter
Author: Sebastian Puchalski
Date: 11.11.2012
*/

#ifndef CONVERTER_HPP_
#define CONVERTER_HPP_

#include "../common.hpp"
#include "OBJFileLoader.hpp"
#include "../box.hpp"
#include "Node.hpp"

#include <string>

class Converter
{
	class Configuration
	{
	public:
		bool load(const std::string & fileName);

		int getTreeDepth() const { return treeDepth; }
		real getSamplingFactor() const { return samplingFactor; }

	private:
		int treeDepth;
		real samplingFactor;
	};

public:
	Converter();
	virtual ~Converter();

	bool convert(const std::string & inputFileName, 
		const std::string & outputFileName);

private:
	Configuration configuration;

	OBJFileLoader mesh;
	boxr * boxes;

	Node * root;

	bool generateBoxes();
	bool buildTree();
	bool saveTree(const std::string & outputFileName) const;

	boxr obtainMainBox() const;
	real calculateVoxelSize(const boxr & mainBox, int depth) const;
};

#endif //CONVERTER_HPP_
