#include "Texture.h"

Texture::Texture()
{
	texture = 0;
	isMultisampled = false;
}

Texture::~Texture()
{
	if (texture)
	{
		glDeleteTextures(1, &texture);
	}
}

bool Texture::loadTexture(const char * filePath)
{
	if (texture)
	{
		glDeleteTextures(1, &texture);
	}

	if (filePath == nullptr)
	{
		fprintf(stderr, "No texture specified \n");
		return false;
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	unsigned char * data = stbi_load(filePath, &width, &height, &numColorChannel, 0);
	if (data)
	{
		if (width % 4 != 0 || height % 4 != 0)
		{
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //Default is 4, so it will only work for nicely aligned picture.
		}

		if (numColorChannel == 3)
		{
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, width, height);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else if (numColorChannel == 4)
		{
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, width, height);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			fprintf(stderr, "Failed to load texture: Unknown number of color channels \n");
			stbi_image_free(data);
			glBindTexture(GL_TEXTURE_2D, 0);
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		fprintf(stderr, "Failed to load texture \n");
		glBindTexture(GL_TEXTURE_2D, 0);
		return false;
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

bool Texture::loadEmptyTexture(int width, int height)
{
	if (texture)
	{
		glDeleteTextures(1, &texture);
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

bool Texture::loadEmptyMultisampleTexture(int width, int height, unsigned int numSamples)
{
	if (texture)
	{
		glDeleteTextures(1, &texture);
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, numSamples, GL_RGBA16F, width, height, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	isMultisampled = true;
	return true;
}

bool Texture::loadShadowTexture(int width, int height)
{
	if (texture)
	{
		glDeleteTextures(1, &texture);
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

const uint & Texture::getTextureID() const
{
	return texture;
}

const int Texture::getWidth()
{
	return width;
}

const int Texture::getHeight()
{
	return height;
}
