#include "Node.hpp"
#include "SimpleObjectPool.hpp"

#include <cassert>

const int NODE_POOL_SIZE = 1048576;
SimpleObjectPool<Node> nodePool(NODE_POOL_SIZE);

Node::Node() : children(0), childrenMask(0), 
	normal(0, 0, 0), color(0, 0, 0), weight(0)
{
}

Node::~Node()
{
}

void Node::addPoint(PointCloudFile::Point & point, int depth)
{
	normal = normal + point.normal;
	color = color + point.color;
	weight++;

	if(depth == 0)
		return;

	int node = 0;
	point.position = point.position * 2.0;
	int flag;

	flag = min((int)point.position.x, 1);
	point.position.x -= flag;
	node += flag;
	flag = min((int)point.position.y, 1);
	point.position.y -= flag;
	node += flag * 2;
	flag = min((int)point.position.z, 1);
	point.position.z -= flag;
	node += flag * 4;

	if(children == 0)
		children = nodePool.create(8);

	childrenMask |= (1 << node);
	children[node].addPoint(point, depth-1);
}

/*uint32 Node::getSize() const
{
	uint32 size = VOXEL_SIZE;
	if(childrenMask != 0)
	{
		size += POINTER_SIZE;
		int node = 0;
		for(int i = 0; i < 8; i++)
		{
			if(childrenMask & (1 << i))
			{
				size += children[node].getSize();
				node++;
			}
		}
	}
	return size;
}*/

uint32 Node::getSize() const
{
	uint32 size = VOXEL_SIZE;
	if(childrenMask != 0)
	{
		size += POINTER_SIZE;
		for(int i = 0; i < 8; i++)
			if(childrenMask & (1 << i))
				size += children[i].getSize();
	}
	return size;
}

/*void Node::save(std::ofstream & file, int depth, int saveDepth, uint32 & pointer) const
{
	if(depth == saveDepth)
	{
		Voxel voxel;
		voxel.mask = childrenMask;
		voxel.material = getMaterial();
		file.write((char *)&voxel, VOXEL_SIZE);

		if(childrenMask != 0)
			pointer += countChildren(childrenMask) * VOXEL_SIZE + POINTER_SIZE;
	}
	else
	{
		if(depth == saveDepth-1)
			if(childrenMask != 0)
				file.write((char *)&pointer, POINTER_SIZE);
		int node = 0;
		for(int i = 0; i < 8; i++)
		{
			if(childrenMask & (1 << i))
			{
				children[node].save(file, depth+1, saveDepth, pointer);
				node++;
			}
		}
	}
}*/

void Node::save(std::ofstream & file, int depth, int saveDepth, uint32 & pointer) const
{
	if(depth == saveDepth)
	{
		Voxel voxel;
		voxel.mask = childrenMask;
		voxel.material = getMaterial();
		file.write((char *)&voxel, VOXEL_SIZE);

		if(childrenMask != 0)
			pointer += countChildren(childrenMask) * VOXEL_SIZE + POINTER_SIZE;
	}
	else
	{
		if(depth == saveDepth-1)
			if(childrenMask != 0)
				file.write((char *)&pointer, POINTER_SIZE);
		for(int i = 0; i < 8; i++)
			if(childrenMask & (1 << i))
				children[i].save(file, depth+1, saveDepth, pointer);
	}
}

void Node::getStats(uint & vmin, uint & vmax, uint & vsum, uint & vweight) const
{
	vmin = -1;
	vmax = vsum = vweight = 0;

	if(children == 0)
	{
		vmin = vmax = vsum = (uint)weight;
		vweight = 1;
	}
	else
	{
		for(int i = 0; i < 8; i++)
		{
			if(childrenMask & (1 << i))
			{
				uint lmin, lmax, lsum, lweight;
				children[i].getStats(lmin, lmax, lsum, lweight);
				vmin = min(vmin, lmin);
				vmax = max(vmax, lmax);
				vsum += lsum;
				vweight += lweight;
			}
		}
	}
}

Voxel::Material Node::getMaterial() const
{
	Voxel::Material material;
	float w = 255.0f / weight;
	material.diffuse.r = (uint8)(color.r * w);
	material.diffuse.g = (uint8)(color.g * w);
	material.diffuse.b = (uint8)(color.b * w);
	return material;
}

int Node::countChildren(uint8 mask) const
{
	int number = 0;
	for(int i = 0; i < 8; i++)
		if(mask & (1 << i))
			number++;
	return number;
}
