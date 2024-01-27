#include "FrameBuffer.hpp"

#include <cfloat>
#include <cassert>

const int INITIAL_BUFFER_WEIGHT = 255;

FrameBuffer::FrameBuffer(int width, int height) : width(width), height(height), aa(true)
{
	colorBuffer = new colorRGB<uint32>[width * height];
	weightBuffer = new uint32[width * height];
	depthBuffer = new float[width * height];
	coverBuffer = new PixelCover[width * height];

	int size = max(width, height);
	levelNumber = log2(size-0.5f) + 1.0f;
	levelSize = new vector2i[levelNumber];
	hierarchicalBuffer = new uint8 * [levelNumber];
	int w = width; int h = height;
	for(int i = levelNumber-1; i >= 0; i--)
	{
		w = w/2 + w%2;
		h = h/2 + h%2;
		levelSize[i].x = w;
		levelSize[i].y = h;
		hierarchicalBuffer[i] = new uint8[w*h];
	}
}

FrameBuffer::~FrameBuffer()
{
	delete [] colorBuffer;
	delete [] weightBuffer;
	delete [] depthBuffer;
	delete [] coverBuffer;

	for(int i = 0; i < levelNumber; i++)
		delete [] hierarchicalBuffer[i];
	delete [] hierarchicalBuffer;
	delete [] levelSize;
}

bool FrameBuffer::setSector(vector2f position, vector2f size)
{
	this->position = position;
	this->size = size;
	pixelsOnUnit = width / size.x;
	squareSize = (size.x * (1 << levelNumber)) / width;
	if(static_cast<int>(pixelsOnUnit * size.y + 0.5f) != height)
		return false;
	return true;
}

void FrameBuffer::setClearColor(color24 color)
{
	clearColor = color;
}

void FrameBuffer::clear()
{
	if(aa)
	{
		for(int i = 0; i < width * height; i++)
		{
			colorBuffer[i] = colorRGB<uint32>(0, 0, 0);
			weightBuffer[i] = 0;
			depthBuffer[i] = FLT_MAX;
			coverBuffer[i].x = coverBuffer[i].y = 0;
		}
	}
	else
	{
		for(int i = 0; i < width * height; i++)
		{
			colorBuffer[i] = colorRGB<uint32>(0, 0, 0);
			weightBuffer[i] = 0;
			depthBuffer[i] = FLT_MAX;
		}
	}

	int w = width; int h = height;
	for(int i = levelNumber-1; i >= 0; i--)
	{
		for(int y = 0; y < levelSize[i].y; y++)
		{
			for(int x = 0; x < levelSize[i].x; x++)
			{
				uint8 value = 0;
				if((w & 0x1) && x+1 == levelSize[i].x)
					value |= 0xA;
				if((h & 0x1) && y+1 == levelSize[i].y)
					value |= 0xC;
				hierarchicalBuffer[i][x+y*levelSize[i].x] = value;
			}
		}
		w = levelSize[i].x;
		h = levelSize[i].y;
	}
}

bool FrameBuffer::checkRectVisibility(
	float left, float right, float bottom, float top) const
{
	if(left > position.x + size.x || right < position.x || 
		bottom > position.y + size.y || top < position.y)
		return false; //frustum culling

	float halfPixelSize = (0.5f / pixelsOnUnit);
	left = (left - position.x - halfPixelSize) / squareSize;
	right = (right - position.x + halfPixelSize) / squareSize;
	bottom = (bottom - position.y - halfPixelSize) / squareSize;
	top = (top - position.y + halfPixelSize) / squareSize;

	float squareResolutionInv = 1.0f / (1 << levelNumber) * 0.99999f;
	left = max(left, 0.0f);
	right = min(right, width * squareResolutionInv);
	bottom = max(bottom, 0.0f);
	top = min(top, height * squareResolutionInv);

	float rectSize = max(right - left, top - bottom);
	int level = 0;
	while(rectSize < 1.0f && level < levelNumber)
	{
		rectSize += rectSize;
		level++;
	}
	level = min(level, levelNumber-1);
	int res = (1 << level);
	if(level > 0)
		level--;
	else
		res += res;

	int bx = left * res; int ex = right * res;
	int by = bottom * res; int ey = top * res;

	for(int y = by; y <= ey; y++)
	{
		for(int x = bx; x <= ex; x++)
		{
			uint8 mask = 1 << ((x & 0x1) | ((y & 0x1) << 1));
			int index = (x >> 1) + ((y >> 1) * levelSize[level].x);
			if(!(hierarchicalBuffer[level][index] & mask))
				return true;
		}
	}

	return false;
}

void FrameBuffer::drawSample(vector2f coordinate, float depth, float size, color32 color)
{
	if(aa)
		drawSampleWithAA(coordinate, depth, size, color);
	else
		drawSampleWithoutAA(coordinate, depth, size, color);
}

void FrameBuffer::finish(color24 * destBuffer, bool switchRB, int spaceNumber)
{
	if(aa)
	{
		for(int y = 0; y < height; y++)
		{
			for(int x = 0; x < width; x++)
			{
				color24 color;
				int index = x + y * width;

				if(weightBuffer[index] != 0)
				{
					bool le = (x == 0 || weightBuffer[index - 1] != 0);
					bool re = (x + 1 == width || weightBuffer[index + 1] != 0);
					bool te = (y == 0 || weightBuffer[index - width] != 0);
					bool be = (y + 1 == height || weightBuffer[index + width] != 0);

					if(!(le && re && te && be))
					{
						uint32 weight = 255 * 255;

						if((le || re) && (te || be))
						{
							uint8 cx = (le && re ? 255 : coverBuffer[index].x);
							uint8 cy = (te && be ? 255 : coverBuffer[index].y);
							weight -= (uint32)cx * cy;
						}

						if(weight > 0)
						{
							colorBuffer[index] = colorBuffer[index] + 
								colorRGB<uint32>(clearColor.r, clearColor.g, clearColor.b) * weight;
							weightBuffer[index] += weight;
						}
					}

					uint32 weight = weightBuffer[index];
					colorRGB<uint32> & c = colorBuffer[index];
					if(!switchRB)
					{
						color.r = c.r / weight;
						color.g = c.g / weight;
						color.b = c.b / weight;
					}
					else
					{
						color.r = c.b / weight;
						color.g = c.g / weight;
						color.b = c.r / weight;
					}
				}
				else
					color = clearColor;
					
				*destBuffer = color;
				destBuffer++;
			}
			destBuffer += spaceNumber;
		}
	}
	else
	{
		for(int y = 0; y < height; y++)
		{
			for(int x = 0; x < width; x++)
			{
				color24 color;
				int index = x + y * width;
				uint32 weight = weightBuffer[index];
				if(weight != 0)
				{
					colorRGB<uint32> & c = colorBuffer[index];
					if(!switchRB)
					{
						color.r = c.r / weight;
						color.g = c.g / weight;
						color.b = c.b / weight;
					}
					else
					{
						color.r = c.b / weight;
						color.g = c.g / weight;
						color.b = c.r / weight;
					}
				}
				else
					color = clearColor;
				*destBuffer = color;
				destBuffer++;
			}
			destBuffer += spaceNumber;
		}
	}
}

void FrameBuffer::drawSampleWithoutAA(vector2f coordinate, float depth, float size, color32 color)
{
	coordinate = (coordinate - position) * (pixelsOnUnit * 256.0f);
	int mx = static_cast<int>(coordinate.x) - 128;
	int my = static_cast<int>(coordinate.y) - 128;
	int x = mx >> 8;
	int y = my >> 8;
	uint8 wx = mx;
	uint8 wy = my;

	colorRGB<uint32> c(color.r, color.g, color.b);
	uint32 weight;
	/*
	00 01
	10 11
	*/
	uint index00 = x + y * width;
	uint index01 = index00 + 1;
	uint index10 = index00 + width;
	uint index11 = index10 + 1;

	bool le = x >= 0;
	bool re = x+1 < width;
	bool te = y >= 0;
	bool be = y+1 < height;

	float ds = depth - size * 2.0f;
	bool c00 = false, c01 = false, c10 = false, c11 = false;

	if(le && re && te && be)
	{
		c00 = depthBuffer[index00] > ds;
		c01 = depthBuffer[index01] > ds;
		c10 = depthBuffer[index10] > ds;
		c11 = depthBuffer[index11] > ds;
	}
	else
	{
		if(x >= -1 && x < width && y >= -1 && y < height)
		{
			c00 = le && te && depthBuffer[index00] > ds;
			c01 = re && te && depthBuffer[index01] > ds;
			c10 = le && be && depthBuffer[index10] > ds;
			c11 = re && be && depthBuffer[index11] > ds;
		}
	}

	if(c00)
	{
		weight = (uint32)(255-wx)*(255-wy);
		weight++;
		colorBuffer[index00] = colorBuffer[index00] + c * weight;
		weightBuffer[index00] += weight;
		depthBuffer[index00] = depth;
		coverSquare(x, y, levelNumber);
	}
	if(c01)
	{
		weight = (uint32)(wx)*(255-wy);
		weight++;
		colorBuffer[index01] = colorBuffer[index01] + c * weight;
		weightBuffer[index01] += weight;
		depthBuffer[index01] = depth;
		coverSquare(x + 1, y, levelNumber);
	}
	if(c10)
	{
		weight = (uint32)(255-wx)*(wy);
		weight++;
		colorBuffer[index10] = colorBuffer[index10] + c * weight;
		weightBuffer[index10] += weight;
		depthBuffer[index10] = depth;
		coverSquare(x, y + 1, levelNumber);
	}
	if(c11)
	{
		weight = (uint32)(wx)*(wy);
		weight++;
		colorBuffer[index11] = colorBuffer[index11] + c * weight;
		weightBuffer[index11] += weight;
		depthBuffer[index11] = depth;
		coverSquare(x + 1, y + 1, levelNumber);
	}
}

void FrameBuffer::drawSampleWithAA(vector2f coordinate, float depth, float size, color32 color)
{
	coordinate = (coordinate - position) * (pixelsOnUnit * 256.0f);
	int mx = static_cast<int>(coordinate.x) - 128;
	int my = static_cast<int>(coordinate.y) - 128;
	int x = mx >> 8;
	int y = my >> 8;
	uint8 wx = mx;
	uint8 wy = my;

	float ds = depth - size * 2.0f;

	colorRGB<uint32> c(color.r, color.g, color.b);
	uint32 index;
	uint8 weightX, weightY;
	uint32 weight;

	//     x=0 x=1
	// y=0 00  01
	// y=1 10  11

	for(uint8 i = 0; i < 4; i++)
	{
		int rx = x + (i & 1);
		int ry = y + (i >> 1);

		if(rx < 0 || rx >= width || ry < 0 || ry >= height)
			continue;

		index = rx + ry * width;

		if(depthBuffer[index] > ds) //depth pass
		{
			if(i & 1) //x
				weightX = wx;
			else
				weightX = 255 - wx;
			if(i & 2) //y
				weightY = wy;
			else
				weightY = 255 - wy;
			weight = (uint32)weightX * weightY;

			coverBuffer[index].x = max(coverBuffer[index].x, weightX);
			coverBuffer[index].y = max(coverBuffer[index].y, weightY);
			weight++;
			colorBuffer[index] = colorBuffer[index] + c * weight;
			weightBuffer[index] += weight;
			if(depthBuffer[index] == FLT_MAX)
				depthBuffer[index] = depth;
			coverSquare(rx, ry, levelNumber);
		}
		else //depth fail
		{
			PixelCover & cover = coverBuffer[index];

			uint8 mask = 0;
			mask |= (rx == 0 || weightBuffer[index - 1] != 0);
			mask |= (rx + 1 == width || weightBuffer[index + 1] != 0) << 1;
			mask |= (ry == 0 || weightBuffer[index - width] != 0) << 2;
			mask |= (ry + 1 == height || weightBuffer[index + width] != 0) << 3;

			if(mask != 0xF) //no discard
			{
				if(i & 1) //x
					weightX = wx;
				else
					weightX = 255 - wx;
				if(i & 2) //y
					weightY = wy;
				else
					weightY = 255 - wy;
				weight = (uint32)weightX * weightY;

				uint8 ix, iy; //intersection

				if((mask & 0x3) == 0x3)
					ix = weightX;
				else
				{
					if((i & 1) == (mask & 1)) //corresponding
						ix = min(cover.x, weightX);
					else //opposite
						ix = max((int32)cover.x + weightX - 255, 0);
				}

				if((mask & 0xB) == 0xB)
					iy = weightY;
				else
				{
					if((i >> 1) == ((mask >> 2) & 1)) //corresponding
						iy = min(cover.y, weightY);
					else //opposite
						iy = max((int32)cover.y + weightY - 255, 0);
				}

				weight -= (uint32)ix * iy;
				if(weight > 0)
				{
					colorBuffer[index] = colorBuffer[index] + c * weight;
					weightBuffer[index] += weight;
				}
			}
		}
	}
}

void FrameBuffer::coverSquare(int x, int y, int level) //iterative version
{
	while(true)
	{
		level--;
		uint8 mask = 1 << ((x & 0x1) | ((y & 0x1) << 1));
		x = x >> 1; y = y >> 1;
		uint8 & square = hierarchicalBuffer[level][x + y * levelSize[level].x];
		if((square & mask) || (square |= mask) != 0xF || level <= 0)
			return;
	}
}
