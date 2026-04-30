#pragma once
#include "Core.h"
#include "stb_image.h" //Use this line for any object that need to load an image

class SpriteData
{

public:
	bool buildSuccess;

	unsigned char* data = nullptr;
	int width;
	int height;
	int numColorChannel; //Doesn't actually help because I manually defined it to have 4 color channel
	glm::vec3 color;

	SpriteData(const char* filepath, glm::vec3 c);
	~SpriteData();
};
