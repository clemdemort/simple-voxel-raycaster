#ifndef CustomNoise_H
#define CustomNoise_H

#include <GLFW/glfw3.h>
#include <iostream>
//made by clemdemort
//------------------
// 
//			WARNING:
// If you plan to use this please make sure you destroy your noise when you dont need it anymore
// as this will result in a memory leak!
// 
//
// 
//this is my custom noise generation classes!
class Noise1D
{
public:
	int l{};
	int* noise = new int[1];
	int IDX(int X)
	{
		return X % (l - 1);
	}
	void generate(int length,int min, int max, float seed) //not using a constructor since one might want to regenerate their noise many times
	{
		delete[] noise;
		l = length;
		noise = new int[l];
		srand(seed);
		for (int x = 0; x < length - 1; x++)
				noise[IDX(x)] = ((rand() % (max - min)) + min);
	}
	float Get(float GetX)
	{
		//gets the interger value for each "corner"
		int fX = GetX;
		int cX = GetX + 1;
		//calculates the noise value of each "corner"
		float corner1 = noise[IDX(fX)];
		float corner2 = noise[IDX(cX)];
		//calculates the distance to the X side
		float DistToX1 = (GetX - fX);
		float DistToX2 = (cX - GetX);
		//linearly interpolates in the X direction
		float result = (1 - DistToX1) * corner1 + (1 - DistToX2) * corner2;

		return result;
	}
	void destroy() { delete[] noise; }
};


class Noise2D
{
public:
	int w{};
	int h{};
	int* noise = new int[1];
	int IDX(int X, int Y)
	{
		return (((Y % (h - 1)) * w) + X % (w - 1));
	}
	void generate(int width, int height, int min, int max, float seed)
	{
		delete[] noise;
		w = width;
		h = height;
		noise = new int[w * h];
		srand(seed);
		for (int x = 0; x < width - 1; x++)
			for (int y = 0; y < height - 1; y++)
				noise[IDX(x, y)] = ((rand() % (max - min)) + min);
	}
	float Get(float GetX, float GetY)
	{
		//gets the interger value for each "corner"
		int fX = GetX;
		int fY = GetY;
		int cX = GetX + 1;
		int cY = GetY + 1;
		//calculates the noise value of each "corner"
		float corner1 = noise[IDX(fX, fY)];
		float corner2 = noise[IDX(fX, cY)];
		float corner3 = noise[IDX(cX, fY)];
		float corner4 = noise[IDX(cX, cY)];
		//calculates the distance to the X side
		float DistToX1 = (GetX - fX);
		float DistToX2 = (cX - GetX);
		//linearly interpolates in the X direction
		float resX1 = (1 - DistToX1) * corner1 + (1 - DistToX2) * corner3;
		float resX2 = (1 - DistToX1) * corner2 + (1 - DistToX2) * corner4;
		//calculates the distance to the Y side
		float DistToY1 = (GetY - fY);
		float DistToY2 = (cY - GetY);
		//linearly interpolates in the Y direction
		float result = (1 - DistToY1) * resX1 + (1 - DistToY2) * resX2;

		return result;
	}
	void destroy() { delete[] noise; }
};

class Noise3D
{
public:
	int w{};
	int h{};
	int d{};
	int* noise = new int[1];
	int IDX(int X, int Y, int Z)
	{
		return ((Z % (d - 1)) * h * w) + ((Y % (h - 1)) * w) + X % (w - 1);
	}
	void generate(int width, int height, int depth, int min, int max, float seed)
	{
		delete[] noise;
		w = width;
		h = height;
		d = depth;
		noise = new int[w * h * d];
		srand(seed);
		for (int x = 0; x < width - 1; x++)
			for (int y = 0; y < height - 1; y++)
				for (int z = 0; z < depth - 1; z++)
					noise[IDX(x, y, z)] = ((rand() % (max - min)) + min);
	}
	float Get(float GetX, float GetY, float GetZ)
	{
		//gets the interger value for each "corner"
		int fX = GetX;
		int fY = GetY;
		int fZ = GetZ;
		int cX = GetX + 1;
		int cY = GetY + 1;
		int cZ = GetZ + 1;
		//calculates the noise value of each "corner"
		float corner1 = noise[IDX(fX, fY,fZ)];
		float corner2 = noise[IDX(fX, fY,cZ)];
		float corner3 = noise[IDX(fX, cY,fZ)];
		float corner4 = noise[IDX(fX, cY,cZ)];
		float corner5 = noise[IDX(cX, fY,fZ)];
		float corner6 = noise[IDX(cX, fY,cZ)];
		float corner7 = noise[IDX(cX, cY,fZ)];
		float corner8 = noise[IDX(cX, cY,cZ)];
		//calculates the distance to the X side
		float DistToX1 = (GetX - fX);
		float DistToX2 = (cX - GetX);
		//linearly interpolates in the X direction
		float resX1 = (1 - DistToX1) * corner1 + (1 - DistToX2) * corner5; //fff and cff
		float resX2 = (1 - DistToX1) * corner2 + (1 - DistToX2) * corner6; //ffc and cfc
		float resX3 = (1 - DistToX1) * corner3 + (1 - DistToX2) * corner7; //fcf and ccf
		float resX4 = (1 - DistToX1) * corner4 + (1 - DistToX2) * corner8; //fcc and ccc
		//calculates the distance to the Y side
		float DistToY1 = (GetY - fY);
		float DistToY2 = (cY - GetY);
		//linearly interpolates in the Y direction
		float resY1 = (1 - DistToY1) * resX1 + (1 - DistToY2) * resX3; //ff and cf
		float resY2 = (1 - DistToY1) * resX2 + (1 - DistToY2) * resX4; //fc and cc
		//calculates the distance to the Z side
		float DistToZ1 = (GetZ - fZ);
		float DistToZ2 = (cZ - GetZ);
		//linearly interpolates in the Y direction
		float result = (1 - DistToZ1) * resY1 + (1 - DistToZ2) * resY2;
		
		return result;
	}
	void destroy() { delete[] noise; }
};
#endif