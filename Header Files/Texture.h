#pragma once
#include "Core.h"
#include "stb_image.h" //Use this line for any object that need to load an image

class Texture
{
	public:
		Texture();
		~Texture();

		bool loadTexture(const char * filePath);
		bool loadEmptyTexture(int width, int height);
		bool loadEmptyMultisampleTexture(int width, int height, unsigned int numSamples);
		bool loadShadowTexture(int width, int height);

		const uint & getTextureID() const;						//{ return texture; }
		const int getWidth();
		const int getHeight();

	private:
		uint texture;
		int width, height, numColorChannel;
		bool isMultisampled;
};