/*
Module: FrameBuffer
Author: Sebastian Puchalski
Date: 26.11.2012
*/

#ifndef FRAME_BUFFER_HPP_
#define FRAME_BUFFER_HPP_

#include "../vector2.hpp"
#include "../color.hpp"

struct PixelCover
{
	uint8 x;
	uint8 y;
};

class FrameBuffer
{
public:
	FrameBuffer(int width, int height); //width and height must be even
	~FrameBuffer();

	bool setSector(vector2f position, vector2f size);
	void setClearColor(color24 color);

	void setAA(bool enabled){ aa = enabled; }

	void clear();
	bool checkRectVisibility(
		float left, float right, float bottom, float top) const;
	void drawSample(vector2f coordinate, float depth, float size, color32 color); 
		//samples must be dense enough to cover all 2x2 pixel rectangles
	float getPixelsOnUnit() const { return pixelsOnUnit; }
	void finish(color24 * destBuffer, bool switchRB, int spaceNumber);

	void getSector(vector2f & position, vector2f & size) const 
		{ position = this->position; size = this->size; }

	int getWidth() const { return width; }
	int getHeight() const { return height; }

private:
	const int width;
	const int height;

	colorRGB<uint32> * colorBuffer;
	uint32 * weightBuffer;
	float * depthBuffer;
	PixelCover * coverBuffer;

	int levelNumber;
	vector2i * levelSize;
	uint8 ** hierarchicalBuffer;

	vector2f position; //position of rectangle
	vector2f size; //size of rectangle
	float pixelsOnUnit;
	float squareSize;
	color24 clearColor;

	bool aa;

	void drawSampleWithoutAA(vector2f coordinate, float depth, float size, color32 color);
	void drawSampleWithAA(vector2f coordinate, float depth, float size, color32 color);
	void coverSquare(int x, int y, int level);
};

#endif //FRAME_BUFFER_HPP_
