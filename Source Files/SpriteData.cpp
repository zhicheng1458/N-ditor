#include "SpriteData.h"

SpriteData::SpriteData(const char* filepath, glm::vec3 c)
{
	color = c;

	stbi_set_flip_vertically_on_load(true);
	data = stbi_load(filepath, &width, &height, &numColorChannel, 4);
	stbi_set_flip_vertically_on_load(false);
	if (data)
	{
		for (int i = 0; i < width * height; i++)
		{
			if (data[i * 4] == (unsigned char)0)
			{
				//Replace all black pixel with color specified in constructor
				data[i * 4] = (unsigned char)(c.x);
				data[i * 4 + 1] = (unsigned char)(c.y);
				data[i * 4 + 2] = (unsigned char)(c.z);
				//Alpha automatically supplied to be 1 due to stbi creating an alpha channel with value set.
				//data[i * 4 + 3] = (unsigned char)(1.0f * 255.0f);;
			}
			else
			{
				//Replace all white pixel with no color + set alpha channel to 0
				data[i * 4] = (unsigned char)(0);
				data[i * 4 + 1] = (unsigned char)(0);
				data[i * 4 + 2] = (unsigned char)(0);
				data[i * 4 + 3] = (unsigned char)(0);
			}
		}
		buildSuccess = true;
	}
	else
	{
		fprintf(stderr, "SpriteData: Failed to load file \n");
		buildSuccess = false;
	}
}

SpriteData::~SpriteData()
{
	if (data)
	{
		stbi_image_free(data);
		data = nullptr;
	}
}