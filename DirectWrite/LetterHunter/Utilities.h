#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <Windows.h>
#include <time.h>

#define SAFE_RELEASE(P) if(P){P->Release() ; P = NULL ;}
#define SAFE_DELETE(P) if(P){delete P; P = NULL;}
#define SAFE_DELETE_ARRAY(P) if(P){delete[] P; P = NULL;}

// Generate a random float nuber between min and max
static float randomFloat(float min, float max)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float range = max - min;
	return (random * range) + min;
}

// Generate random integer between start and end(inclusive)
static int RandomInt(int start, int end)
{
	srand((unsigned int)time(0));
	int k = rand() % (end - start + 1) + start;

	return k;
}

// Generate random lower case letter('a' - 'z')
static char RandomLowerCaseLetter()
{
	srand((unsigned int)time(0));

	int offset = RandomInt(0, 25);
	char letter = 'a' + offset;

	return letter;
}

// Generate random upper case letter('A' - 'Z')
static char RandomUpperCaseLetter()
{
	srand((unsigned int)time(0));

	int offset = RandomInt(0, 25);
	char letter = 'A' + offset;

	return letter;
}

// Generate a random RGB color
static DWORD RandomColor()
{
	srand((unsigned int)time(0));

	DWORD a = 0xff000000;
	int r = RandomInt(0, 255) << 16;
	int g = RandomInt(0, 255) << 8;
	int b = RandomInt(0, 255) ;

	DWORD color = a | r | g | b;
	return color;
}

#endif // end __UTILITIES_H__