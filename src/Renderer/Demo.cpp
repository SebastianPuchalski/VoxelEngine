#include "Demo.hpp"

#include <iostream>
#include <fstream>

static const float MOUSE_SPEED = 0.0002f;
static const float CAMERA_SPEED = 0.25f;

const std::string CONFIGURATION_FILE_NAME = "Renderer.conf";

Demo::Demo() : sceneData(0), renderer(0), yaw(0), pitch(0), 
	left(false), right(false), front(false), back(false)
{
}

Demo::~Demo()
{
}

bool Demo::initialize(const std::string & sceneFileName)
{
	if(!settings.load(CONFIGURATION_FILE_NAME))
		return false;

	if(!loadScene(sceneFileName))
		return false;

	if(renderer != 0)
	{
		finalize();
		return false;
	}
	renderer = new MultithreadedRenderer(
		settings.width, 
		settings.height, 
		true, 
		settings.tileNumber, 
		settings.threadNumber);

	initializeCamera();

	renderer->setSceneData(sceneData);
	renderer->setCamera(&camera);
	renderer->setPerspective(settings.fieldOfView);
	renderer->setClearColor(settings.clearColor);
	renderer->setAA(settings.antialiasing);

	if(!Window::create(Window::Settings(
		settings.width, 
		settings.height, 
		settings.fullscreen, 
		settings.windowTile, 
		settings.multiplier)))
	{
		finalize();
		return false;
	}
	Window::setOnMouseMove(Window::onMouseMoveDelegate(this, &Demo::onMouseMove));
	Window::setOnKey(Window::onKeyDelegate(this, &Demo::onKey));

	return true;
}

void Demo::finalize()
{
	Window::destroy();

	if(renderer != 0)
	{
		delete renderer;
		renderer = 0;
	}

	if(sceneData != 0)
	{
		delete [] sceneData;
		sceneData = 0;
	}
}

bool Demo::update()
{
	updateCamera();
	color24 * data = renderer->render();
	if(data == 0)
	{
		std::cout << "Rendering error!\n";
		return false;
	}
	return Window::draw((Window::color *)data);
}

bool Demo::Settings::load(const std::string & fileName)
{
	std::fstream file(fileName);
	if(!file.is_open())
		return false;

	file >> width >> height;
	file >> tileNumber.x >> tileNumber.y;
	file >> threadNumber;

	int fs;
	file >> fs;
	fullscreen = (fs == 1);
	file >> windowTile;
	file >> multiplier;

	file >> startCameraPosition.x;
	file >> startCameraPosition.y;
	file >> startCameraPosition.z;
	file >> startCameraDirection.x;
	file >> startCameraDirection.y;
	file >> startCameraDirection.z;
	file >> fieldOfView;

	float r, g, b;
	file >> r >> g >> b;
	clearColor = color24(r * 255, g * 255, b * 255);
	int aa;
	file >> aa;
	antialiasing = (aa == 1);

	return !file.fail();
}

bool Demo::loadScene(const std::string & fileName)
{
	if(sceneData != 0)
		return false;

	std::ifstream file(fileName, std::ios::binary);
	if(!file.is_open())
		return false;

	SceneFileHeader header;
	if(!header.load(file))
		return false;

	sceneData = new uint8[header.getDataSize()];
	file.read(reinterpret_cast<char *>(sceneData), header.getDataSize());
	if(file.fail())
	{
		delete [] sceneData;
		return false;
	}

	return true;
}

void Demo::onMouseMove(int x, int y)
{
	yaw += x * MOUSE_SPEED;
	yaw = yaw - floor(yaw);
	pitch += (-y) * MOUSE_SPEED;
	pitch = max(0.0f, min(0.5f, pitch));
}

void Demo::onKey(int key, bool pressed)
{
	switch(key)
	{
	case 65:
		left = pressed;
		break;
	case 68:
		right = pressed;
		break;
	case 87:
		front = pressed;
		break;
	case 83:
		back = pressed;
		break;
	};

	if(key == 'F' && pressed)
	{
		settings.antialiasing = !settings.antialiasing;
		renderer->setAA(settings.antialiasing);
	}
}

void Demo::initializeCamera()
{
	camera.setPosition(settings.startCameraPosition);
	camera.setOrientation(settings.startCameraDirection);

	vector2f rotate(settings.startCameraDirection.x, 
		settings.startCameraDirection.z);
	rotate.normalize();
	yaw = acos(rotate.y) / (2.0f * PI);
	if(rotate.x < 0.0f)
		yaw = 1.0f - yaw;

	settings.startCameraDirection.normalize();
	pitch = 0.5f - acos(settings.startCameraDirection.y) / (2.0f * PI);

	updateCamera();
}

void Demo::updateCamera()
{
	float sinYaw = sin(yaw * 2.0f * PI);
	float cosYaw = cos(yaw * 2.0f * PI);
	float sinPitch = sin(pitch * 2.0f * PI);
	float cosPitch = cos(pitch * 2.0f * PI);
	vector3f x(cosYaw, 0.0f, -sinYaw);
	vector3f d(sinYaw * sinPitch, -cosPitch, cosYaw * sinPitch);
	camera.setX(x);
	camera.setY(d.cross(x));
	camera.setZ(d);

	timer.update();
	float shift = timer.getFrameTime() * CAMERA_SPEED;
	if(left)
		camera.setPosition(camera.getPosition() - camera.getX() * shift);
	if(right)
		camera.setPosition(camera.getPosition() + camera.getX() * shift);
	if(front)
		camera.setPosition(camera.getPosition() + camera.getZ() * shift);
	if(back)
		camera.setPosition(camera.getPosition() - camera.getZ() * shift);
}
