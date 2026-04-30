#include "EditorOverlay.h"

EditorOverlay::EditorOverlay(float screenWidth, float screenHeight, float tileSize)
{
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;
	this->tileSize = tileSize;
	
	trayScrollMaxOffset = tileSize * 28.0f * 2.0f; //There are exactly 29 entities/28 gaps, button layout are 2 tileSize apart.

	screenProjMtx = glm::ortho(-screenWidth * 0.5f, screenWidth * 0.5f, -screenHeight * 0.5f, screenHeight * 0.5f);

	trayShader = new ShaderProgram("./Resources/Shaders/Tray.glsl", ShaderProgram::eRender);
	entitySelectorShader = new ShaderProgram("./Resources/Shaders/EntitySelector.glsl", ShaderProgram::eRender);
	entitySpriteSheet = new Texture();
	entitySpriteSheet->loadTexture("./Resources/Textures/EntitiesSpriteSheetV2.png");
	glUseProgram(trayShader->getProgramID());
	glUniform1f(glGetUniformLocation(trayShader->getProgramID(), "screenWidth"), screenWidth);
	glUniform1f(glGetUniformLocation(trayShader->getProgramID(), "screenHeight"), screenHeight);
	glUniformMatrix4fv(glGetUniformLocation(trayShader->getProgramID(), "projMtx"), 1, false, (float*)&screenProjMtx);
	glUseProgram(0);
	glUseProgram(entitySelectorShader->getProgramID());
	glUniform1f(glGetUniformLocation(entitySelectorShader->getProgramID(), "screenWidth"), screenWidth);
	glUniform1f(glGetUniformLocation(entitySelectorShader->getProgramID(), "screenHeight"), screenHeight);
	glUniform1f(glGetUniformLocation(entitySelectorShader->getProgramID(), "scrollValue"), currentScroll);
	glUniform1i(glGetUniformLocation(entitySelectorShader->getProgramID(), "entitySpriteSheet"), 0); //Made assumption that GLTEXTURE_0 is active for this texture;
	glUniform1i(glGetUniformLocation(entitySelectorShader->getProgramID(), "typeToHighlight"), MINE);
	glUseProgram(0);

	glGenBuffers(1, &trayVertexBuffer);
	glGenBuffers(1, &trayIndexBuffer);
	glGenBuffers(1, &entitySelectorVertexBuffer);
	glGenBuffers(1, &entitySelectorIndexBuffer);

	this->buildOverlay();
}

EditorOverlay::~EditorOverlay()
{
	glFinish();
	delete trayShader;
	delete entitySelectorShader;
	delete entitySpriteSheet;
	glDeleteBuffers(1, &trayVertexBuffer);
	glDeleteBuffers(1, &trayIndexBuffer);
	glDeleteBuffers(1, &entitySelectorVertexBuffer);
	glDeleteBuffers(1, &entitySelectorIndexBuffer);
}

void EditorOverlay::showTray()
{
	drawTray = true;
}

void EditorOverlay::hideTray()
{
	drawTray = false;
}

void EditorOverlay::scrollTray(float offset)
{
	currentScroll = glm::clamp(currentScroll + 100.0f * ((offset > 0.0f) ? 1.0f : -1.0f), trayScrollMinOffset, trayScrollMaxOffset);
	glUseProgram(entitySelectorShader->getProgramID());
	glUniform1f(glGetUniformLocation(entitySelectorShader->getProgramID(), "scrollValue"), currentScroll);
	glUseProgram(0);
}

bool EditorOverlay::wantControl(float mouseX, float mouseY)
{
	return drawTray && mouseY >= (screenHeight - traySize);
}

void EditorOverlay::processMouseLocation(float mouseX, float mouseY)
{
	if (!drawTray) { return; }

	glm::vec2 buttonScreenOffset = glm::vec2(screenWidth * 0.5f, screenHeight - traySize * 0.5f);
	if (mouseY < buttonScreenOffset.y - tileSize * 0.5f || mouseY > buttonScreenOffset.y + tileSize * 0.5f) { return; }

	float normalizedMouseX = mouseX - buttonScreenOffset.x + tileSize * 0.5f + currentScroll;
	if (normalizedMouseX < 0.0f || normalizedMouseX > tileSize * 28.0f * 2.0f + tileSize) { return; }

	int selected = static_cast<int>(normalizedMouseX / tileSize);
	if (selected % 2 == 1)
	{
		/*
		entityType currentType = NONE;
		if (lastSelectedEntityType != currentType)
		{
			glUseProgram(entitySelectorShader->getProgramID());
			glUniform1i(glGetUniformLocation(entitySelectorShader->getProgramID(), "typeToHighlight"), 99);
			glUseProgram(0);
		}
		lastSelectedEntityType = currentType;
		*/
	}
	else
	{
		entityType currentType = static_cast<entityType>(selected / 2);
		if (lastSelectedEntityType != currentType)
		{
			glUseProgram(entitySelectorShader->getProgramID());
			glUniform1i(glGetUniformLocation(entitySelectorShader->getProgramID(), "typeToHighlight"), selected / 2);
			glUseProgram(0);
		}
		lastSelectedEntityType = currentType;
	}
}

entityType EditorOverlay::getSelectedEntityType()
{
	if (lastSelectedEntityType != NONE)
	{
		return lastSelectedEntityType;
	}
	return lastSelectedEntityType;
}

void EditorOverlay::update()
{

}

void EditorOverlay::drawSolidObj()
{
	if (!drawTray) { return; }

	glDisable(GL_CULL_FACE);
	glUseProgram(trayShader->getProgramID());

	// Tray
	glBindBuffer(GL_ARRAY_BUFFER, trayVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, trayIndexBuffer);

	uint trayPosLoc = 0;
	glEnableVertexAttribArray(trayPosLoc);
	glVertexAttribPointer(trayPosLoc, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenBasedModelVtx), (void*)(0 * sizeof(float)));

	uint trayTextureLoc = 1;
	glEnableVertexAttribArray(trayTextureLoc);
	glVertexAttribPointer(trayTextureLoc, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenBasedModelVtx), (void*)(2 * sizeof(float)));

	uint trayColorLoc = 2;
	glEnableVertexAttribArray(trayColorLoc);
	glVertexAttribPointer(trayColorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(ScreenBasedModelVtx), (void*)(4 * sizeof(float)));

	glDrawElements(GL_TRIANGLES, (uint)trayIdx.size(), GL_UNSIGNED_INT, 0);

	// Clean up state
	glDisableVertexAttribArray(trayPosLoc);
	glDisableVertexAttribArray(trayTextureLoc);
	glDisableVertexAttribArray(trayColorLoc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnable(GL_CULL_FACE);
	glUseProgram(0);
}

void EditorOverlay::drawTransparentObj()
{
	if (!drawTray) { return; }

	glDisable(GL_CULL_FACE);
	glUseProgram(entitySelectorShader->getProgramID());

	// EntitySelector
	glBindBuffer(GL_ARRAY_BUFFER, entitySelectorVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entitySelectorIndexBuffer);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, entitySpriteSheet->getTextureID());

	uint entityPosLoc = 0;
	glEnableVertexAttribArray(entityPosLoc);
	glVertexAttribPointer(entityPosLoc, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenBasedModelVtx), (void*)(0 * sizeof(float)));

	uint entityTextureLoc = 1;
	glEnableVertexAttribArray(entityTextureLoc);
	glVertexAttribPointer(entityTextureLoc, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenBasedModelVtx), (void*)(2 * sizeof(float)));

	uint entityTypeLoc = 2;
	glEnableVertexAttribArray(entityTypeLoc);
	glVertexAttribIPointer(entityTypeLoc, 1, GL_INT, sizeof(ScreenBasedModelVtx), (void*)(8 * sizeof(float)));

	glDrawElements(GL_TRIANGLES, (uint)entitySelectorIdx.size(), GL_UNSIGNED_INT, 0);

	// Clean up state
	glDisableVertexAttribArray(entityPosLoc);
	glDisableVertexAttribArray(entityTextureLoc);
	glDisableVertexAttribArray(entityTypeLoc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnable(GL_CULL_FACE);
	glUseProgram(0);
}

void EditorOverlay::resize(float screenWidth, float screenHeight)
{
	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;

	glUseProgram(trayShader->getProgramID());
	glUniform1f(glGetUniformLocation(trayShader->getProgramID(), "screenWidth"), screenWidth);
	glUniform1f(glGetUniformLocation(trayShader->getProgramID(), "screenHeight"), screenHeight);
	glUseProgram(0);
	glUseProgram(entitySelectorShader->getProgramID());
	glUniform1f(glGetUniformLocation(entitySelectorShader->getProgramID(), "screenWidth"), screenWidth);
	glUniform1f(glGetUniformLocation(entitySelectorShader->getProgramID(), "screenHeight"), screenHeight);
	glUseProgram(0);

	trayVtx.clear();
	trayIdx.clear();
	entitySelectorVtx.clear();
	entitySelectorIdx.clear();
	this->buildOverlay();
}

void EditorOverlay::buildOverlay()
{
	//Build the overlay in terms of screen coordinate

	//Tray
	glm::vec2 topLeft = glm::vec2(0, screenHeight - traySize);
	glm::vec2 topRight = glm::vec2(screenWidth, screenHeight - traySize);
	glm::vec2 bottomLeft = glm::vec2(0, screenHeight);
	glm::vec2 bottomRight = glm::vec2(screenWidth, screenHeight);
	glm::vec4 trayColor = glm::vec4(0.6f, 0.6f, 0.6f, 0.7f);
	glm::vec2 noTexture = glm::vec2(-1.0f, -1.0f);
	addTriangle(topRight, topLeft, bottomLeft, trayColor, trayColor, trayColor, noTexture, noTexture, noTexture, 0, trayVtx, trayIdx);
	addTriangle(topRight, bottomLeft, bottomRight, trayColor, trayColor, trayColor, noTexture, noTexture, noTexture, 0, trayVtx, trayIdx);

	bottomLeft = topLeft;
	bottomRight = topRight;
	topLeft.y -= trayBorderSize;
	topRight.y -= trayBorderSize;
	glm::vec4 trayBorderColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	addTriangle(topRight, topLeft, bottomLeft, trayBorderColor, trayBorderColor, trayBorderColor, noTexture, noTexture, noTexture, 0, trayVtx, trayIdx);
	addTriangle(topRight, bottomLeft, bottomRight, trayBorderColor, trayBorderColor, trayBorderColor, noTexture, noTexture, noTexture, 0, trayVtx, trayIdx);

	//Buttons for selecting entities
	glm::vec4 entityColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec2 buttonScreenOffset = glm::vec2(screenWidth * 0.5f, screenHeight - traySize * 0.5f);
	//glm::vec2 buttonScreenOffset = glm::vec2(0.0f, screenHeight - traySize * 0.5f);
	float halfTileSize = tileSize * 0.5f;
	topLeft = buttonScreenOffset + glm::vec2(-halfTileSize, -halfTileSize);
	topRight = buttonScreenOffset + glm::vec2(halfTileSize, -halfTileSize);
	bottomLeft = buttonScreenOffset + glm::vec2(-halfTileSize, halfTileSize);
	bottomRight = buttonScreenOffset + glm::vec2(halfTileSize, halfTileSize);
	int offset = 0;
	float entitySizeOnSpriteSheet = 1.0f / (float)numRows;
	glm::vec2 textureTLOffset = glm::vec2(0.0f, -entitySizeOnSpriteSheet); 
	glm::vec2 textureTROffset = glm::vec2(entitySizeOnSpriteSheet, -entitySizeOnSpriteSheet);
	glm::vec2 textureBLOffset = glm::vec2(0.0f, 0.0f);
	glm::vec2 textureBROffset = glm::vec2(entitySizeOnSpriteSheet, 0.0f);
	for (int i = 0; i < 29; i++)
	{
		glm::vec2 sampleCoord = glm::vec2((i % numCols) * entitySizeOnSpriteSheet,
								   1.0f - (i / numRows) * entitySizeOnSpriteSheet);

		glm::vec2 tl = topLeft + glm::vec2(tileSize * 2.0f * i, 0.0f);
		glm::vec2 tr = topRight + glm::vec2(tileSize * 2.0f * i, 0.0f);
		glm::vec2 bl = bottomLeft + glm::vec2(tileSize * 2.0f * i, 0.0f);
		glm::vec2 br = bottomRight + glm::vec2(tileSize * 2.0f * i, 0.0f);
		glm::vec2 sampleCoordTL = sampleCoord + textureTLOffset;
		glm::vec2 sampleCoordTR = sampleCoord + textureTROffset;
		glm::vec2 sampleCoordBL = sampleCoord + textureBLOffset;
		glm::vec2 sampleCoordBR = sampleCoord + textureBROffset;
		addTriangle(tr, tl, bl, entityColor, entityColor, entityColor, sampleCoordTR, sampleCoordTL, sampleCoordBL, i, entitySelectorVtx, entitySelectorIdx);
		addTriangle(tr, bl, br, entityColor, entityColor, entityColor, sampleCoordTR, sampleCoordBL, sampleCoordBR, i, entitySelectorVtx, entitySelectorIdx);
	}

	//Buffet building
	//Tray Buffers
	if (trayIdx.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, trayVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, trayVtx.size() * sizeof(ScreenBasedModelVtx), &trayVtx[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, trayIndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, trayIdx.size() * sizeof(uint), &trayIdx[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	//EntitySelectorBuffers
	if (entitySelectorIdx.size() > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, entitySelectorVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, entitySelectorVtx.size() * sizeof(ScreenBasedModelVtx), &entitySelectorVtx[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entitySelectorIndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, entitySelectorIdx.size() * sizeof(uint), &entitySelectorIdx[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}

void EditorOverlay::addTriangle(const glm::vec2& v1, const glm::vec2& v2, const glm::vec2& v3,
								const glm::vec4& c1, const glm::vec4& c2, const glm::vec4& c3,
								const glm::vec2& t1, const glm::vec2& t2, const glm::vec2& t3,
								int type, std::vector<ScreenBasedModelVtx>& vtx, std::vector<uint>& idx)
{
	uint vertexCount = (uint)vtx.size();

	vtx.push_back({ {v1.x, v1.y}, {t1.x, t1.y}, {c1.x, c1.y, c1.z, c1.a}, {type} });
	vtx.push_back({ {v2.x, v2.y}, {t2.x, t2.y}, {c2.x, c2.y, c2.z, c2.a}, {type} });
	vtx.push_back({ {v3.x, v3.y}, {t3.x, t3.y}, {c3.x, c3.y, c3.z, c3.a}, {type} });

	idx.push_back(vertexCount + (uint)0); idx.push_back(vertexCount + (uint)1); idx.push_back(vertexCount + (uint)2);
}
