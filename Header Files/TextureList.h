#pragma once
#include "Core.h"
#include "Texture.h"
#include <vector>

/**
 * TextureList is responsible for activating the correct texture for the
 * shader, but is not responsible for creating the texture itself.
 */

struct TextureInfo
{
	unsigned int textureID = 0;				//The id that openGL gave back to you when creating the texture
	unsigned int bindingLocation = -1;		//Which texture unit (GL_TEXTUREXXX) to bind the texture
	bool isMultisample = false;
};

class TextureList
{
	public:
		TextureList();

		void useProgram(const uint & programID);
		bool addTexture(const uint & textureID, unsigned int bindingLocation, bool isMultisampled);
		void bindTextures(); //Bind textures into proper textures unit
		void removeAllTexture(); //Remove all binded textures
		bool activateTextures(); //Tell shaders which texture is active
		bool deactivateTextures(); //Tell shaders which texture is inactive (doesn't actually unbind)

	private:
		const unsigned int MAX_COCURRENT_TEXTURES = 10; //Support up to 10 texture
		int textureLocation[10]; //GLSL truncate int to unsigned int so negative value becomes 0.
		int isActiveTexture[10]; //It should be bool type but glsl aren't that smart, combined textureID into it
		int inactiveTextureArray[10];
		unsigned int programID;

		std::vector<TextureInfo> textureInfoList;

		const unsigned int OBJECT_TEXTURE_LOC1 = 0;
		const unsigned int OBJECT_TEXTURE_LOC2 = 1;
		const unsigned int OBJECT_TEXTURE_LOC3 = 2;
		const unsigned int OBJECT_TEXTURE_LOC4 = 3;
		const unsigned int OBJECT_TEXTURE_LOC5 = 4;
		const unsigned int SHADOW_TEXTURE_LOC1 = 5;
};