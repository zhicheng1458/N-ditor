#include "TextureList.h"

TextureList::TextureList()
{
	removeAllTexture();
	for (unsigned int i = 0; i < MAX_COCURRENT_TEXTURES; i++)
	{
		inactiveTextureArray[i] = -1;
	}

	textureInfoList.reserve(MAX_COCURRENT_TEXTURES);
}

void TextureList::useProgram(const uint & programID)
{
	this->programID = programID;
}

bool TextureList::addTexture(const uint & textureID, unsigned int bindingLocation, bool isMultisampled)
{
	if (programID)
	{
		TextureInfo newTextureInfo;
		newTextureInfo.textureID = textureID;
		if (bindingLocation > MAX_COCURRENT_TEXTURES)
		{
			fprintf(stderr, "binding location exceeded maximum number of supportable textures. Cannot use texture. \n");
			return false;
		}
		else
		{
			newTextureInfo.bindingLocation = bindingLocation;
		}
		newTextureInfo.isMultisample = isMultisampled;
		textureInfoList.push_back(newTextureInfo);

		textureLocation[newTextureInfo.bindingLocation] = newTextureInfo.bindingLocation;
		isActiveTexture[newTextureInfo.bindingLocation] = 1;


		return true;
	}
	else
	{
		fprintf(stderr, "Program ID does not exist. Cannot use texture \n");
		return false;
	}
}

void TextureList::bindTextures()
{
	if (programID)
	{
		glUseProgram(programID);
		/*
		for (unsigned int i = 0; i < MAX_COCURRENT_TEXTURES; i++)
		{
			glActiveTexture(GL_TEXTURE0 + textureLocation[i]);
			glBindTexture(GL_TEXTURE_2D, isActiveTexture[i]);
		}
		*/
		for (unsigned int i = 0; i < textureInfoList.size(); i++)
		{
			if (textureInfoList[i].bindingLocation == (unsigned int)-1) {continue; }

			glActiveTexture(GL_TEXTURE0 + textureInfoList[i].bindingLocation);
			if (textureInfoList[i].isMultisample)
			{
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureInfoList[i].textureID);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, textureInfoList[i].textureID);
			}
		}
		glActiveTexture(GL_TEXTURE0);
		glUniform1iv(glGetUniformLocation(programID, "textureBindingLocation"), MAX_COCURRENT_TEXTURES, textureLocation); // 0 -> GL_TEXTURE0
		glUseProgram(0);
	}
	else
	{
		fprintf(stderr, "Program ID does not exist. Cannot use texture \n");
	}
}

void TextureList::removeAllTexture()
{
	for (unsigned int i = 0; i < MAX_COCURRENT_TEXTURES; i++)
	{
		textureLocation[i] = MAX_COCURRENT_TEXTURES + 1;
		isActiveTexture[i] = -1;
	}
}

bool TextureList::activateTextures()
{
	if (programID)
	{
		glUseProgram(programID);
		glUniform1iv(glGetUniformLocation(programID, "isActiveTexture"), MAX_COCURRENT_TEXTURES, isActiveTexture);
		glUseProgram(0);
		return true;
	}
	else
	{
		fprintf(stderr, "Program ID does not exist. Cannot use texture \n");
		return false;
	}
}

bool TextureList::deactivateTextures()
{
	if (programID)
	{
		glUseProgram(programID);
		glUniform1iv(glGetUniformLocation(programID, "isActiveTexture"), MAX_COCURRENT_TEXTURES, inactiveTextureArray);
		glUseProgram(0);
		return true;
	}
	else
	{
		fprintf(stderr, "Program ID does not exist. Cannot use texture \n");
		return false;
	}
}