/*
Module: Demo
Author: Sebastian Puchalski
Date: 15.12.2013
*/

#ifndef DEMO_HPP_
#define DEMO_HPP_

#include "MultithreadedRenderer.hpp"
#include "Window.hpp"
#include "Timer.hpp"

class Demo
{
public:
	Demo();
	virtual ~Demo();

	bool initialize(const std::string & sceneFileName);
	void finalize();

	bool update();

private:
	class Settings
	{
	public:
		int width, height;
		vector2i tileNumber;
		int threadNumber;

		bool fullscreen;
		std::string windowTile;
		int multiplier;

		vector3f startCameraPosition;
		vector3f startCameraDirection;
		float fieldOfView;

		color24 clearColor;
		bool antialiasing;

		bool load(const std::string & fileName);
	};

	Settings settings;

	uint8 * sceneData;
	MultithreadedRenderer * renderer;
	Camera camera;
	float yaw, pitch;
	bool left, right, front, back;
	Timer timer;

	bool loadScene(const std::string & fileName);

	void onMouseMove(int x, int y);
	void onKey(int key, bool pressed);

	void initializeCamera();
	void updateCamera();
};

#endif //DEMO_HPP_
