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
// also a mention never hurts ;)
// 
//this is my custom noise generation class!
class Noise
{
public:
	int w{};
	int h{};
	int *noise2D = new int[w * h];
	int IDX(int X,int Y)
	{
		return (((Y % (h-1)) * w) + X % (w-1));
	}
	void generate2d(int width, int height, int min, int max)
	{
		w = width;
		h = height;
		noise2D = new int[w * h];
		srand(glfwGetTime());
		for (int x = 0; x < width-1; x++)
			for (int y = 0; y < height-1; y++)
				noise2D[IDX(x,y)] = ((rand() % (max - min)) + min);
	}
	float Get2D(float GetX, float GetY)
	{
		//gets the interger value for each "corner"
		int fX = GetX;
		int fY = GetY;
		int cX = GetX+1;
		int cY = GetY+1;
		//calculates the noise value of each "corner"
		float corner1 = noise2D[IDX(fX,fY)];
		float corner2 = noise2D[IDX(fX,cY)];
		float corner3 = noise2D[IDX(cX,fY)];
		float corner4 = noise2D[IDX(cX,cY)];
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
	void destroy(){ delete[] noise2D; }
};


#endif // !Time_Sync_H