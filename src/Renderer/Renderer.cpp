#include "Renderer.hpp"

static const float LOD_THRESHOLD = 1.41421356f;
static const float CULLING_THRESHOLD = 1.41421356f * 3.0f; //average for 4x4 (recommended minimum)

static const uint8 CHILDREN_NUMBER[256] = { //returns number of children
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
	4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};

static const uint8 WALL_MASK[8] = {21, 22, 25, 26, 37, 38, 41, 42};

static const float SQRT3 = 1.73205081f;

static const float SPECULAR_EXPONENT = 24;

static const float NEAR_PLANE = 0.01f;

Renderer::Renderer(int width, int height) : 
	sceneData(0), camera(0), frameBuffer(width, height), illumination(true)
{
}

Renderer::~Renderer()
{
}

void Renderer::setSceneData(uint8 * data)
{
	sceneData = data;
}

void Renderer::setCamera(const Camera * camera)
{
	this->camera = camera;
}

bool Renderer::setFrameBuffer(vector2f position, vector2f size)
{
	return frameBuffer.setSector(position, size);
}

void Renderer::setClearColor(color24 color)
{
	frameBuffer.setClearColor(color);
}

void Renderer::setLightSource(DirectionalLightSource light)
{
	lightSource = light;
}

bool Renderer::render(color24 * colorBuffer, bool switchRB, int spaceNumber)
{
	if(sceneData == 0 || camera == 0)
		return false;

	calculateFrustumNormals();

	frameBuffer.clear();

	vector3f position(0.0f, 0.0f, 0.0f);
	float size = 2.0f;
	const uint8 * frame = sceneData + *reinterpret_cast<const Pointer *>(sceneData);
	Voxel voxel = *reinterpret_cast<const Voxel *>(sceneData + POINTER_SIZE);

	if(obtainNodeScale(position, size) < LOD_THRESHOLD)
		drawVoxel(position, size, illuminate(voxel.material, position));
	else
		if(voxel.mask == 0)
			splitVoxel(position, size * 0.5f, illuminate(voxel.material, position));
		else
			draw(position, size * 0.5f, frame, voxel.mask);

	frameBuffer.finish(colorBuffer, switchRB, spaceNumber);
	return true;
}

void Renderer::calculateFrustumNormals()
{
	vector2f position, size;
	frameBuffer.getSector(position, size);

	vector3f lb = camera->getX() * position.x + camera->getY() * position.y + camera->getZ();
	vector3f rt = lb + camera->getX() * size.x + camera->getY() * size.y;

	fL = lb.cross(camera->getY()).normalized();
	fR = rt.cross(camera->getY() * -1.0f).normalized();
	fB = lb.cross(camera->getX() * -1.0f).normalized();
	fT = rt.cross(camera->getX()).normalized();
}

void Renderer::draw(vector3f position, float size, const uint8 * frame, uint8 mask)
{
	const Voxel * voxels = reinterpret_cast<const Voxel *>(frame + POINTER_SIZE);
	const uint8 * children = sceneData + *reinterpret_cast<const Pointer *>(frame);
	uint8 voxelShifts[8]; //voxels[voxelShift[i]] = i-th voxel
	uint16 childShifts[8]; //children + childShift[i] = pointer to frame of i-th child

	int voxelShift = 0;
	int childShift = 0;
	for(int i = 0; i < 8; i++)
	{
		if(mask & (1 << i))
		{
			voxelShifts[i] = voxelShift;
			childShifts[i] = childShift;
			childShift += CHILDREN_NUMBER[voxels[voxelShift].mask] * 
				VOXEL_SIZE + POINTER_SIZE;
			voxelShift++;
		}
	}

	float scale = obtainNodeScale(position, size);
	uint8 nearest = obtainNearestNode(position);

	float halfSize = size * 0.5f; //halfSize - half size of child
	position.x -= halfSize;
	position.y -= halfSize;
	position.z -= halfSize;

	for(int i = 0; i < 8; i++)
	{
		uint8 index = i ^ nearest;
		if(mask & (1 << index))
		{
			vector3f childPosition = position;
			if(index & 1)
				childPosition.x += size;
			if(index & 2)
				childPosition.y += size;
			if(index & 4)
				childPosition.z += size;

			const Voxel & v = voxels[voxelShifts[index]];
			if(scale < LOD_THRESHOLD)
				drawVoxel(childPosition, size, illuminate(v.material, childPosition));
			else
			{
				if(scale < CULLING_THRESHOLD || checkNodeVisibility(childPosition, size))
				{
					if(v.mask == 0)
						splitVoxel(childPosition, halfSize, illuminate(v.material, childPosition));
					else
						draw(childPosition, halfSize, children + childShifts[index], v.mask);
				}
			}
		}
	}
}

void Renderer::splitVoxel(vector3f position, float size, color32 color, 
	uint8 nearest, uint8 wallMask)
{
	float scale = obtainNodeScale(position, size);
	if(wallMask == 0)
	{
		nearest = obtainNearestNode(position);
		wallMask = WALL_MASK[nearest];
	}

	float halfSize = size * 0.5f;
	position.x -= halfSize;
	position.y -= halfSize;
	position.z -= halfSize;

	for(int i = 0; i < 8; i++)
	{
		uint8 index = i ^ nearest;
		uint8 childWallMask = WALL_MASK[index] & wallMask;
		if(childWallMask != 0)
		{
			vector3f childPosition = position;
			if(index & 1)
				childPosition.x += size;
			if(index & 2)
				childPosition.y += size;
			if(index & 4)
				childPosition.z += size;

			if(scale < LOD_THRESHOLD)
				drawVoxel(childPosition, size, color);
			else
				if(scale < CULLING_THRESHOLD || checkNodeVisibility(childPosition, size))
					splitVoxel(childPosition, halfSize, color, nearest, childWallMask);
		}
	}
}

float Renderer::obtainNodeScale(vector3f position, float size) const //return node size / pixel size
{
	float distance = (position - camera->getPosition()).dot(camera->getZ());
	return size / abs(distance) * frameBuffer.getPixelsOnUnit();
}

uint8 Renderer::obtainNearestNode(vector3f position) const //return index of nearest node
{
	vector3f direction = camera->getPosition() - position;
	uint8 result = 0;
	result |= (direction.x >= 0.0f);
	result |= (direction.y >= 0.0f) << 1;
	result |= (direction.z >= 0.0f) << 2;
	return result;
}

bool Renderer::checkNodeVisibility(vector3f position, float size) const
{
	position = position - camera->getPosition();
	float distance = position.dot(camera->getZ()) - NEAR_PLANE; //distance to near plane
	float radius = size * SQRT3 * 0.5f;
	if(distance < -radius) return false;
	if(position.dot(fL) > radius || 
		position.dot(fR) > radius || 
		position.dot(fB) > radius || 
		position.dot(fT) > radius)
		return false; //frustum culling
	if(distance < radius) return true;

	float left = FLOAT_MAX;
	float right = FLOAT_MIN;
	float bottom = FLOAT_MAX;
	float top = FLOAT_MIN;

	float halfSize = size * 0.5f;
	position.x -= halfSize;
	position.y -= halfSize;
	position.z -= halfSize;

	for(int i = 0; i < 8; i++)
	{
		vector3f corner = position;
		if(i & 0x1)
			corner.x += size;
		if(i & 0x2)
			corner.y += size;
		if(i & 0x4)
			corner.z += size;

		vector2f coordinate;
		float inv_z = 1.0f / corner.dot(camera->getZ());
		coordinate.x = corner.dot(camera->getX()) * inv_z;
		coordinate.y = corner.dot(camera->getY()) * inv_z;

		left = min(left, coordinate.x);
		right = max(right, coordinate.x);
		bottom = min(bottom, coordinate.y);
		top = max(top, coordinate.y);
	}
	return frameBuffer.checkRectVisibility(left, right, bottom, top);
}

void Renderer::drawVoxel(vector3f position, float size, color32 color)
{
	position = position - camera->getPosition();
	float distance = position.dot(camera->getZ());
	if(distance > 0)
	{
		float inv_distance = 1.0f / distance;
		vector2r coordinate;
		coordinate.x = position.dot(camera->getX()) * inv_distance;
		coordinate.y = position.dot(camera->getY()) * inv_distance;
		frameBuffer.drawSample(coordinate, distance, size, color);
	}
}

color32 Renderer::illuminate(const Voxel::Material & material, vector3f position)
	//material should be given as reference or pointer in order to avoid copying
{
	return color32(material.diffuse.r, material.diffuse.g, material.diffuse.g, 255);

	/*if(!illumination)
		return color32(material.diffuse.r, material.diffuse.g, material.diffuse.g, material.alpha);

	color96 diffuse(material.diffuse.r / 255.0f, material.diffuse.g / 255.0f, material.diffuse.b / 255.0f);
	float alpha = material.alpha / 255.0f;
	vector3f normal(material.normal.x / 127.0f, material.normal.y / 127.0f, material.normal.z / 127.0f);
	float specular = material.specular / 255.0f;

	color96 color(0, 0, 0);
	//ambient:
	color = color + diffuse * lightSource.ambient;
	//diffuse:
	color = color + diffuse * lightSource.diffuse * max(0.0f, normal.dot(lightSource.direction) * -1.0f);
	//specular:
	if(material.specular > 0)
	{
		vector3f incidence = (position - camera->getPosition()).normalized();
		vector3f reflection = incidence - normal * normal.dot(incidence) * 2.0f;
		float coeff = max(0.0f, reflection.dot(lightSource.direction) * -1.0f);
		if(coeff > 0)
		{
			coeff *= coeff; //coeff^2
			coeff *= coeff; //coeff^4
			coeff *= coeff; //coeff^8
			coeff *= coeff; //coeff^16
			color = color + lightSource.specular * specular * coeff;
		}
	}

	//color.r = 1 - exp(-color.r);
	//color.g = 1 - exp(-color.g);
	//color.b = 1 - exp(-color.b);
	color.r = min(color.r, 1.0f);
	color.g = min(color.g, 1.0f);
	color.b = min(color.b, 1.0f);

	return color32(color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, alpha * 255.0f);*/
}
