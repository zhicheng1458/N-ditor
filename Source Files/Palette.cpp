#include "Palette.h"

Palette::Palette()
{
	buildPalette(1600, 900);
}

Palette::~Palette()
{
	glFinish();
	if (entityTexture)
	{
		glDeleteTextures(1, &entityTexture);
	}
}

bool Palette::buildPalette(int windowWidth, int windowHeight)
{
	glFinish();
	if (entityTexture)
	{
		glDeleteTextures(1, &entityTexture);
	}

	std::vector<glm::vec3> colors = {
	entityColors.ninjaColor,							entityColors.mineExteriorColor,				entityColors.mineInteriorColor,					entityColors.goldInteriorColor,
	entityColors.goldExteriorColor,						entityColors.goldShineColor,				entityColors.exitDoorClosedInteriorColor,		entityColors.exitDoorClosedBorderColor,
	entityColors.exitSwitchClosedBorderColor,			entityColors.exitSwitchCenterColor,			entityColors.exitSwitchClosedBackgroundColor,	entityColors.regularDoorColor,
	entityColors.lockedDoorBarColor,					entityColors.lockedDoorCenterColor,			entityColors.lockedDoorSwitchClosedCenterColor,	entityColors.lockedDoorSwitchClosedBorderColor,
	entityColors.lockedDoorSwitchClosedBackgroundColor,	entityColors.trapDoorBarColor,				entityColors.trapDoorCenterColor,				entityColors.trapDoorSwitchClosedBorderColor,
	entityColors.trapDoorSwitchClosedBackgroundColor,	entityColors.bouncePadBaseColor,			entityColors.bouncePadSpringColor,				entityColors.oneWayLongColor,
	entityColors.oneWayShortColor,						entityColors.chaingunDroneBackgroundColor,	entityColors.chaingunDroneBorderColor,			entityColors.laserDroneBorderColor,
	entityColors.regularDroneBackgroundColor,			entityColors.regularDroneBorderColor,		entityColors.chaseDroneBackgroundColor,			entityColors.chaseDroneBorderColor,
	entityColors.floorGuardMainColor,					entityColors.floorGuardEyeColor,			entityColors.bounceBlockInteriorColor,			entityColors.bounceBlockBorderColor,
	entityColors.rocketBorderColor,						entityColors.rocketCenterColor,				entityColors.gaussBorderColor,					entityColors.gaussInteriorColor,
	entityColors.thwumpBorderColor,						entityColors.thwumpInteriorColor,			entityColors.thwumpRayColor,					entityColors.toggleUntouchedColor,
	entityColors.evilNinjaColor,						entityColors.laserTurretColor,				entityColors.boosterColor,						entityColors.deathballOuterColor,
	entityColors.deathballMiddleColor,					entityColors.deathballInnerColor,			entityColors.microDroneBackgroundColor,			entityColors.microDroneBorderColor,
	entityColors.eyebatColor,							entityColors.shoveThwumpExteriorColor,		entityColors.shoveThwumpRayColor,				entityColors.shoveThwumpInteriorColor };

	/*
	int tempTextureWidth = 0;
	int tempTextureHeight = 0;
	int tempNumColorChannel = 0;
	glGenTextures(1, &entityTexture);
	glBindTexture(GL_TEXTURE_2D, entityTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, textureWidth, textureHeight);
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("./Resources/Textures/EntitiesSpriteSheetV2.png", &tempTextureWidth, &tempTextureHeight, &tempNumColorChannel, 0);
	stbi_set_flip_vertically_on_load(false);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	*/

	//1. Generate a texture that holds all the component
	//2. Generate a second texture as the source, leave as empty for now
	//3. Generate a framebuffer for drawing
	//4. Build the vertex data
	//5. Draw the vertex data using a shader

	uint componentTextureWidth = 1000;
	uint componentTextureHeight = 1000;
	uint componentSizeOnTexture = 50;
	uint componentTexture = 0;
	uint tempFrameBuffer = 0;
	uint dataBuffer = 0;

	//Step 1
	glGenTextures(1, &componentTexture);
	glBindTexture(GL_TEXTURE_2D, componentTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, componentTextureWidth, componentTextureHeight);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (int i = 0; i < files.size(); i++)
	{
		bool flag = addSpriteToTexture(files[i], colors[i], i, componentTextureWidth, componentTextureHeight, componentSizeOnTexture);
		if (!flag)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDeleteTextures(1, &componentTexture);
			return false;
		}
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Step 2
	glGenTextures(1, &entityTexture);
	glBindTexture(GL_TEXTURE_2D, entityTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, textureWidth, textureHeight);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Step 3
	glGenFramebuffers(1, &tempFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, tempFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, entityTexture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Step 4
	std::vector<TextureMappingInfo> data;
	for (int i = 0; i < spriteTextureLocation.size(); i++)
	{
		addEntityBuildingData(spriteTextureLocation[i], i,
							  textureWidth, textureHeight, maximumWidthPerEntity,
							  componentTextureWidth, componentTextureHeight, componentSizeOnTexture,
							  data);
	}

	glGenBuffers(1, &dataBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, dataBuffer);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(TextureMappingInfo), &data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Step 5
	//Unfortunately the texture size may not be the same as the viewport size,
	//so we need to temporarily tell the viewport to be equal size to the texture
	glViewport(0, 0, textureWidth, textureHeight);
	ShaderProgram* entityBuilder = new ShaderProgram("./Resources/Shaders/EntityTextureBuilder.glsl", ShaderProgram::eRender);
	glUseProgram(entityBuilder->getProgramID());
	glUniform1i(glGetUniformLocation(entityBuilder->getProgramID(), "entityComponentTexture"), 0); //Made assumption that GLTEXTURE_0 is active for the component texture;
	
	glBindFramebuffer(GL_FRAMEBUFFER, tempFrameBuffer);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, componentTexture);
	glBindBuffer(GL_ARRAY_BUFFER, dataBuffer);
	uint vtxLoc = 0;
	glEnableVertexAttribArray(vtxLoc);
	glVertexAttribPointer(vtxLoc, 2, GL_FLOAT, GL_FALSE, sizeof(TextureMappingInfo), (void*)(0));
	uint txtLoc = 1;
	glEnableVertexAttribArray(txtLoc);
	glVertexAttribPointer(txtLoc, 2, GL_FLOAT, GL_FALSE, sizeof(TextureMappingInfo), (void*)(2 * sizeof(float)));
	glDrawArrays(GL_TRIANGLES, 0, data.size());

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glUseProgram(0);

	//Cleanup
	glViewport(0, 0, windowWidth, windowHeight); //Return viewport to original size
	glFinish();
	glDeleteTextures(1, &componentTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &tempFrameBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &dataBuffer);
	delete entityBuilder;

	return true;
}

const uint& Palette::getEntityTextureID() const
{
	return entityTexture;
}

glm::ivec2 Palette::getPlacementCoordinate(uint location, uint spriteWidth, uint spriteHeight, uint txtWidth, uint txtHeight, uint widthPerEntity)
{
	uint center = widthPerEntity / 2;
	uint row = location / (txtHeight / widthPerEntity);
	uint col = location % (txtWidth / widthPerEntity);

	uint x = col * widthPerEntity + center - spriteWidth / 2;
	uint y = row * widthPerEntity + center - spriteHeight / 2;

	return glm::ivec2(x, y);
}

bool Palette::addSpriteToTexture(const std::string& filepath, const glm::vec3& spriteColor, const uint& location,
								 uint txtWidth, uint txtHeight, uint widthPerEntity)
{
	std::string fullPath = entitySpriteLocation + filepath;
	SpriteData sprite = SpriteData(fullPath.c_str(), spriteColor);
	if (sprite.buildSuccess)
	{
		glm::ivec2 placementLoc = getPlacementCoordinate(location, (uint)sprite.width, (uint)sprite.height, txtWidth, txtHeight, widthPerEntity);
		glTexSubImage2D(GL_TEXTURE_2D, 0, placementLoc.x, placementLoc.y, sprite.width, sprite.height, GL_RGBA, GL_UNSIGNED_BYTE, sprite.data);
		return true;
	}
	else
	{
		return false;
	}
}

void Palette::addTriangle(const glm::vec2& v1, const glm::vec2& v2, const glm::vec2& v3,
						  const glm::vec2& t1, const glm::vec2& t2, const glm::vec2& t3,
						  std::vector<TextureMappingInfo>& data)
{
	data.push_back({ {v1.x, v1.y}, {t1.x, t1.y}});
	data.push_back({ {v2.x, v2.y}, {t2.x, t2.y}});
	data.push_back({ {v3.x, v3.y}, {t3.x, t3.y}});
}

void Palette::addSquare(const glm::vec2& topLeftVtx, const glm::vec2& bottomRightVtx,
						const glm::vec2& topLeftTxtLoc, const glm::vec2& bottomRightTxtLoc,
						std::vector<TextureMappingInfo>& data)
{
	glm::vec2 topRightVtx = glm::vec2(bottomRightVtx.x, topLeftVtx.y);
	glm::vec2 bottomLeftVtx = glm::vec2(topLeftVtx.x, bottomRightVtx.y);
	glm::vec2 topRightTxtLoc = glm::vec2(bottomRightTxtLoc.x, topLeftTxtLoc.y);
	glm::vec2 bottomLeftTxtLoc = glm::vec2(topLeftTxtLoc.x, bottomRightTxtLoc.y);

	addTriangle(topLeftVtx, bottomLeftVtx, topRightVtx,
				topLeftTxtLoc, bottomLeftTxtLoc, topRightTxtLoc,
				data);
	addTriangle(topRightVtx, bottomLeftVtx,	bottomRightVtx,
				topRightTxtLoc, bottomLeftTxtLoc, bottomRightTxtLoc,
				data);
}

void Palette::addEntityBuildingData(const uint entityLocOnDestinationTexture, const uint entityComponentLocOnSourceTexture,
									const uint destinationTextureWidth, const uint destinationTextureHeight, const uint destinationSizePerEntity,
									const uint sourceTextureWidth, const uint sourceTextureHeight, const uint sourceSizePerComponent,
									std::vector<TextureMappingInfo>& data)
{
	uint d_rows = destinationTextureHeight / destinationSizePerEntity;
	uint d_cols = destinationTextureWidth / destinationSizePerEntity;
	float d_stride = 1.0f / (float)d_rows;
	//Get bottom left corner
	float d_x = (float)(entityLocOnDestinationTexture % d_cols) * d_stride;
	float d_y = (float)(entityLocOnDestinationTexture / d_rows) * d_stride;
	glm::vec2 d_tl = glm::vec2(d_x, d_y + d_stride);
	glm::vec2 d_br = glm::vec2(d_x + d_stride, d_y);
	d_tl = glm::vec2(d_tl.x * 2.0f - 1.0f, d_tl.y * 2.0f - 1.0f); //Need to transform texture range [0, 1] to screen normalized range [-1, 1]
	d_br = glm::vec2(d_br.x * 2.0f - 1.0f, d_br.y * 2.0f - 1.0f);

	uint s_rows = sourceTextureHeight / sourceSizePerComponent;
	uint s_cols = sourceTextureWidth / sourceSizePerComponent;
	float s_stride = 1.0f / (float)s_rows;
	float s_x = (float)(entityComponentLocOnSourceTexture % s_cols) * s_stride;
	float s_y = (float)(entityComponentLocOnSourceTexture / s_rows) * s_stride;
	glm::vec2 s_tl = glm::vec2(s_x, s_y + s_stride);
	glm::vec2 s_br = glm::vec2(s_x + s_stride, s_y);

	addSquare(d_tl, d_br, s_tl, s_br, data);
}
