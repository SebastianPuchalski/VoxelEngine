/*
Module: PointCloudConverter
Author: Sebastian Puchalski
Date: 27.05.2014
*/

#include "../common.hpp"
#include "PointCloudFile.hpp"
#include "../box.hpp"
#include "Node.hpp"

#include <string>

#ifndef	POINT_CLOUD_CONVERTER_HPP_
#define POINT_CLOUD_CONVERTER_HPP_

class PointCloudConverter
{
	class Configuration
	{
	public:
		bool load(const std::string & fileName);
		int getTreeDepth() const { return treeDepth; }

	private:
		int treeDepth;
	};

public:
	PointCloudConverter();
	virtual ~PointCloudConverter();

	bool convert(const std::string & inputFileName, 
		const std::string & outputFileName);

private:
	Configuration configuration;
	Node * root;

	bool buildTree(const std::string & inputFileName);
	bool saveTree(const std::string & outputFileName) const;

	bool obtainMainBox(PointCloudFile & pointCloud, vector3r & position, real & size) const;
};

#endif //POINT_CLOUD_CONVERTER_HPP_
