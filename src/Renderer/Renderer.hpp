/*
Module: Renderer
Author: Sebastian Puchalski
Date: 21.11.2012
*/

#ifndef RENDERER_HPP_
#define RENDERER_HPP_

#include "../common.hpp"
#include "../vector3.hpp"
#include "../SceneFile.hpp"
#include "Camera.hpp"
#include "FrameBuffer.hpp"
#include "LightSource.hpp"

class Renderer
{
public:
	Renderer(int width, int height);
	virtual ~Renderer();

	void setSceneData(uint8 * data);
	void setCamera(const Camera * camera);
	bool setFrameBuffer(vector2f position, vector2f size);
	void setClearColor(color24 color);
	void setAA(bool enabled){ frameBuffer.setAA(enabled); }
	void setLightSource(DirectionalLightSource light);
	void setIllumination(bool enabled){ illumination = enabled; }

	bool render(color24 * colorBuffer, bool switchRB = false, int spaceNumber = 0);

private:
	const uint8 * sceneData;
	const Camera * camera;
	FrameBuffer frameBuffer;
	vector3f fL, fR, fB, fT; //frustum normals
	DirectionalLightSource lightSource;
	bool illumination;

	void calculateFrustumNormals(); //normals are used to frustum culling

	void draw(vector3f position, float size, const uint8 * frame, uint8 mask); //drawing of node
	void splitVoxel(vector3f position, float size, color32 color, 
		uint8 nearest = 0, uint8 wallMask = 0);

	float obtainNodeScale(vector3f position, float size) const; //level of detail
	uint8 obtainNearestNode(vector3f position) const; //sorting

	bool checkNodeVisibility(vector3f position, float size) const; //frustum and occlusion culling
	void drawVoxel(vector3f position, float size, color32 color); //drawing of voxel
	color32 illuminate(const Voxel::Material & material, vector3f position); //calculate illumination
};

#endif //RENDERER_HPP_
