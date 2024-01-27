#include "Node.hpp"

#include "TextureManager.hpp"

const real TRIANGLE_SIZE_MULTIPLIER = 4.0; //1.0 - triangle is translated when its max length is equal to node size

Node::Node() : children(0), childrenMask(0)
{
}

Node::~Node()
{
	if(children != 0)
		delete [] children;
}

void Node::operator=(Node & rhs)
{
	children = rhs.children;
	material.diffuse = rhs.material.diffuse;
	//material.alpha = rhs.material.alpha;
	//material.normal = rhs.material.normal;
	//czy mozna przepisac caly material zamiast poszczegolnych komponentow?
	childrenMask = rhs.childrenMask;

	rhs.children = 0; //rhs get rid of their children
}

bool Node::create(VoxelMaterial & material, 
	const boxr & box, real sampleDistance, 
	int depth, int maxDepth, 
	const boxr * boxes, const std::vector<int32> & indexes, 
	const OBJFileLoader & mesh, 
	const std::vector<Point> & points)
{
	if(depth == maxDepth)
	{
		combinePoints(material, points);
		encodeMaterial(material);
		return true;
	}

	boxr childrenBoxes[8];
	std::vector<int32> childrenIndexes[8];
	std::vector<Point> childrenPoints[8];
	int number = 0;
	for(int i = 0; i < 8; i++)
	{
		childrenBoxes[i] = getChildBox(box, i);
		std::vector<int32> trianglesToPointsIndexes;
		for(int j = 0; j < indexes.size(); j++)
		{
			if(childrenBoxes[i].checkIntersection(boxes[indexes[j]]))
			{
				if(boxes[indexes[j]].maxLength() * TRIANGLE_SIZE_MULTIPLIER < 
					childrenBoxes[i].maxLength() && depth+1 != maxDepth)
				{
					childrenIndexes[i].push_back(indexes[j]);
				}
				else
					trianglesToPointsIndexes.push_back(indexes[j]);
			}
		}
		if(trianglesToPointsIndexes.size() > 0)
		{
			std::vector<Point> moreThanChildrenPoints;
			trianglesToPoints(moreThanChildrenPoints, sampleDistance, trianglesToPointsIndexes, mesh);
			for(int j = 0; j < moreThanChildrenPoints.size(); j++)
			{
				if(childrenBoxes[i].checkIntersection(moreThanChildrenPoints[j].position))
					childrenPoints[i].push_back(moreThanChildrenPoints[j]);
			}
		}
		for(int j = 0; j < points.size(); j++)
		{
			if(childrenBoxes[i].checkIntersection(points[j].position))
				childrenPoints[i].push_back(points[j]);
		}
		if(childrenIndexes[i].size() > 0 || childrenPoints[i].size() > 0)
		{
			number++;
			childrenMask |= (1 << i);
		}
	}
	children = new Node[number];

	uint8 newChildrenMask = 0;
	VoxelMaterial childrenMaterial[8];
	
	int node = 0;
	for(int i = 0; i < 8; i++)
	{
		if(childrenMask & (1 << i))
		{
			if(children[node].create(childrenMaterial[i], 
				childrenBoxes[i], sampleDistance, 
				depth+1, maxDepth, 
				boxes, childrenIndexes[i], 
				mesh, 
				childrenPoints[i]))
			{
				newChildrenMask |= (1 << i);
			}
			node++;
		}
	}

	if(newChildrenMask == 0)
	{
		delete [] children;
		children = 0;
		return false;
	}
	else
		refreshChildrenList(newChildrenMask);

	calculateParentMaterial(material, childrenMaterial);
	encodeMaterial(material);
	return true;
}

uint32 Node::getSize() const
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
}

void Node::save(std::ofstream & file, 
		int depth, int saveDepth, 
		uint32 & pointer) const
{
	if(depth == saveDepth)
	{
		Voxel voxel;
		voxel.mask = childrenMask;
		voxel.material = material;
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
}

boxr Node::getChildBox(const boxr & parent, uint number) const
{
	//child sequence:
	bool x = number & 0x1;
	bool y = number & 0x2;
	bool z = number & 0x4;

	return boxr(
		x ? (parent.left + parent.right) * 0.5 : parent.left,
		x ? parent.right : (parent.left + parent.right) * 0.5,
		y ? (parent.bottom + parent.top) * 0.5 : parent.bottom,
		y ? parent.top : (parent.bottom + parent.top) * 0.5,
		z ? (parent.back + parent.front) * 0.5 : parent.back,
		z ? parent.front : (parent.back + parent.front) * 0.5);
}

void Node::refreshChildrenList(uint8 newChildrenMask)
{
	if(childrenMask != newChildrenMask)
	{
		int number = 0; //new children number
		for(int i = 0; i < 8; i++)
			if(newChildrenMask & (1 << i))
				number++;
		Node * newChildren = new Node[number];

		int oldIndex = 0;
		int newIndex = 0;
		for(int i = 0; i < 8; i++)
		{
			if(childrenMask & (1 << i))
			{
				if(newChildrenMask & (1 << i))
				{
					newChildren[newIndex] = children[oldIndex];
					newIndex++;
				}
				oldIndex++;
			}
		}

		delete [] children;
		children = newChildren;
		childrenMask = newChildrenMask;
	}
}

void Node::calculateParentMaterial(VoxelMaterial & outputMaterial, const VoxelMaterial * childrenMaterial) const
{
	//temporary solution:

	outputMaterial.diffuse = color96(0, 0, 0);
	outputMaterial.alpha = 0;
	outputMaterial.normal = vector3r(0, 0, 0);
	outputMaterial.specular = 0;

	int number = countChildren(childrenMask);

	real weight = 1.0 / number;
	for(int i = 0; i < 8; i++)
	{
		if(childrenMask & (1 << i))
		{
			outputMaterial.diffuse = outputMaterial.diffuse + childrenMaterial[i].diffuse * weight;
			outputMaterial.alpha = outputMaterial.alpha + childrenMaterial[i].alpha * weight;
			outputMaterial.normal = outputMaterial.normal + childrenMaterial[i].normal * weight;
			outputMaterial.specular = outputMaterial.specular + childrenMaterial[i].specular * weight;
		}
	}
}

void Node::trianglesToPoints(std::vector<Point> & points, real sampleDistance, 
		const std::vector<int32> & faceIndexes, const OBJFileLoader & mesh) const
{
	int materialGroup = -1;
	const MTLFileLoader::Material * material = 0;
	const Texture * ambientTexture = 0;
	const Texture * diffuseTexture = 0;
	const Texture * specularTexture = 0;
	const Texture * alphaTexture = 0;

	for(int i = 0; i < faceIndexes.size(); i++)
	{
		bool materialChange = false;
		while(materialGroup+1 < mesh.getMaterialGroups().size() && 
			faceIndexes[i] >= mesh.getMaterialGroups()[materialGroup+1].first)
		{
			materialGroup++;
			materialChange = true;
		}
		if(materialChange)
		{
			int materialIndex = mesh.getMaterialGroups()[materialGroup].second;
			material = &(mesh.getMaterialLibrary().getMaterials()[materialIndex]);
			ambientTexture = TextureManager::get().getTexture(material->getAmbientMap());
			diffuseTexture = TextureManager::get().getTexture(material->getDiffuseMap());
			specularTexture = TextureManager::get().getTexture(material->getSpecularMap());
			alphaTexture = TextureManager::get().getTexture(material->getAlphaMap());
		}
		triangleToPoints(points, sampleDistance, 
			mesh.getFaces()[faceIndexes[i]], mesh, material, 
			ambientTexture, diffuseTexture, specularTexture, alphaTexture);
	}
}

void Node::triangleToPoints(std::vector<Point> & points, real sampleDistance, 
		const OBJFileLoader::Face & face, const OBJFileLoader & mesh, 
		const MTLFileLoader::Material * material, 
		const Texture * ambientTexture, const Texture * diffuseTexture, 
		const Texture * specularTexture, const Texture * alphaTexture) const
{
	int shift = findLongestEdge(face, mesh);

	real a, b;
	vector3r x, y;
	real f;
	vector3r p;

	bool isCoords = true;
	bool isNormals = true;
	for(int i = 0; i < 3; i++)
	{
		if(face.coord[i] == -1)
			isCoords = false;
		if(face.normal[i] == -1)
			isNormals = false;
	}

	vector3r va = mesh.getVertices()[face.vertex[shift]];
	vector3r vb = mesh.getVertices()[face.vertex[(1+shift)%3]];
	vector3r vc = mesh.getVertices()[face.vertex[(2+shift)%3]];
	vector2r ca, cb, cc;
	if(isCoords)
	{
		ca = mesh.getCoords()[face.coord[shift]];
		cb = mesh.getCoords()[face.coord[(1+shift)%3]];
		cc = mesh.getCoords()[face.coord[(2+shift)%3]];
	}
	vector3r na, nb, nc;
	if(isNormals)
	{
		na = mesh.getNormals()[face.normal[shift]];
		nb = mesh.getNormals()[face.normal[(1+shift)%3]];
		nc = mesh.getNormals()[face.normal[(2+shift)%3]];
	}

	vector3r edge = vb - va;
	a = edge.length();
	x = edge * (1 / a);
	f = (vc - va).dot(x);
	p = x * (-f) + vc;
	vector3r vap = va - p;
	b = vap.length();
	y = vap * (1 / b);

	int xNumber = (a / sampleDistance) + 1;
	int yNumber = (b / sampleDistance) + 1;

	int level;
	if(isCoords)
	{
		real fd = (ca - cb).length() * (sampleDistance / (va - vb).length());
		real sd = (cb - cc).length() * (sampleDistance / (vb - vc).length());
		real td = (cc - ca).length() * (sampleDistance / (vc - va).length());
		real dist = max(max(fd, sd), td);
		level = -log2(dist + (1.0/65536))/* + 0.5*/;
		if(level < 0)
			level = 0;
	}

	for(int j = 0; j < yNumber; j++)
	{
		for(int i = 0; i <= xNumber; i++)
		{
			vector3r w;

			if(i == xNumber)
			{
				if(j+1 == yNumber)
				{
					if(points.size() > 0) //if triangle is to small...
						break;
					real weight = 1.0 / 3.0;
					w = vector3r(weight, weight, weight);
				}
				else
					continue;
			}
			else
			{
				w = obtainWeights(a, b, f, i*sampleDistance, j*sampleDistance);
			}


			if(w.x >= 0 && w.y >= 0 && w.z >= 0)
			{
				vector3r vertex;
				vertex = va * w.x + vb * w.y + vc * w.z;
				vector2r coord;
				if(isCoords)
					coord = ca * w.x + cb * w.y + cc * w.z;
				vector3r normal;
				if(isNormals)
					normal = na * w.x + nb * w.y + nc * w.z;

				Point point;
				point.position = vertex;
				point.material.diffuse = color96(1, 1, 1);
				point.material.alpha = 1;
				point.material.specular = 0;
				if(material != 0)
				{
					color96 color;
					float parameter;
					if(material->getDiffuse(color))
						point.material.diffuse = color;
					if(material->getAlpha(parameter))
						point.material.alpha = parameter;
					if(material->getSpecular(color))
						point.material.specular = (color.r + color.g + color.b) / 3.0;
				}
				if(isNormals)
					point.material.normal = normal.normalized();
				else
					point.material.normal = (va - vb).cross(va - vc).normalized();

				if(isCoords)
				{
					//texture has greater priority than material parameters (???):
					if(diffuseTexture != 0)
						point.material.diffuse = diffuseTexture->sample(coord, level);
					if(alphaTexture != 0)
						point.material.alpha = (alphaTexture->sample(coord, level)).r; //improve it!
					if(specularTexture != 0)
					{
						color96 spec = specularTexture->sample(coord, level);
						point.material.specular = (spec.r + spec.g + spec.g) / 3.0;
					}
				}

				if(point.material.alpha > 0.0f) //add only visible points
					points.push_back(point);
			}
		}
	}
}

int Node::findLongestEdge(const OBJFileLoader::Face & face, const OBJFileLoader & mesh) const
{
	int shift = 0;
	real maxEdgeLength = -1.0;
	for(int i = 0; i < 3; i++)
	{
		real length = (mesh.getVertices()[face.vertex[i]] - 
			mesh.getVertices()[face.vertex[(i+1)%3]]).length();
		if(length > maxEdgeLength)
		{
			shift = i;
			maxEdgeLength = length;
		}
	}
	return shift;
}

vector3r Node::obtainWeights(real a, real b, real f, real x, real y) const
{
	real g = (b*(x-f)/y) + f;
	real wx = g/a;
	real wy = (b-y)/b;
	return vector3r((1-wy)*(1-wx), (1-wy)*wx, wy);
}

void Node::encodeMaterial(const VoxelMaterial & mat)
{
	material.diffuse.r = mat.diffuse.r * 255;
	material.diffuse.g = mat.diffuse.g * 255;
	material.diffuse.b = mat.diffuse.b * 255;

	/*material.alpha = mat.alpha * 255;

	material.normal.x = mat.normal.x * 127;
	material.normal.y = mat.normal.y * 127;
	material.normal.z = mat.normal.z * 127;

	material.specular = mat.specular * 255;*/
}

void Node::combinePoints(VoxelMaterial & outputMaterial, const std::vector<Point> points) const
{
	outputMaterial.diffuse = color96(0, 0, 0);
	outputMaterial.alpha = 0;
	outputMaterial.normal = vector3r(0, 0, 0);
	outputMaterial.specular = 0;

	real weight = 1.0 / points.size();
	for(int i = 0; i < points.size(); i++)
	{
		outputMaterial.diffuse = outputMaterial.diffuse + points[i].material.diffuse * weight;
		outputMaterial.alpha = outputMaterial.alpha + points[i].material.alpha * weight;
		outputMaterial.normal = outputMaterial.normal + points[i].material.normal * weight;
		outputMaterial.specular = outputMaterial.specular + points[i].material.specular * weight;
	}
}

int Node::countChildren(uint8 mask) const
{
	int number = 0;
	for(int i = 0; i < 8; i++)
		if(mask & (1 << i))
			number++;
	return number;
}
