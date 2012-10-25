#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <Windows.h>
#include <time.h>

#include <d2d1.h>

#define SAFE_RELEASE(P) if(P){P->Release() ; P = NULL ;}
#define SAFE_DELETE(P) if(P){delete P; P = NULL;}
#define SAFE_DELETE_ARRAY(P) if(P){delete[] P; P = NULL;}

// Generate a random float nuber between min and max
static float randomFloat(float min, float max)
{
	// Make sure srand only run once.
	static bool hasRun = false;
	if(!hasRun)
	{
		srand((unsigned int)time(0));
		hasRun = true;
	}

	float random = ((float)rand()) / (float)RAND_MAX;
	float range = max - min;
	return (random * range) + min;
}

// Generate random integer between start and end(inclusive)
static int randomInt(int start, int end)
{
	static bool hasRun = false;
	if(!hasRun)
	{
		srand((unsigned int)time(0));
		hasRun = true;
	}

	int k = rand() % (end - start + 1) + start;

	return k;
}

// Generate random lower case letter('a' - 'z')
static char randomLowerCaseLetterObject()
{
	int offset = randomInt(0, 25);
	char letter = 'a' + offset;

	return letter;
}

// Generate random upper case letter('A' - 'Z')
static char randomUpperCaseLetterObject()
{
	int offset = randomInt(0, 25);
	char letter = 'A' + offset;

	return letter;
}

// Generate a random string,
// @strBuffer, the buffer to hold the string
// @strLength, the length of the string
static void randomString(wchar_t* strBuffer, int strLength)
{
	for(int i = 0; i < strLength; ++i)
	{
		int offset = randomInt(0, 25);
		strBuffer[i] = 'A' + offset;
	}

	strBuffer[strLength] = '\0';
}

// Generate a random RGB color
static D2D1_COLOR_F randomColor()
{
	D2D1_COLOR_F color;

	color.a = 1.0f;
	color.r = randomFloat(0.0f, 1.0f);
	color.g = randomFloat(0.0f, 1.0f);
	color.b = randomFloat(0.0f, 1.0f);
	
	return color;
}

#endif // end __UTILITIES_H__