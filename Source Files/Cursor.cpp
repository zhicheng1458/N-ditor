#include "Cursor.h"

Cursor::Cursor(float tileSize, const Palette& p)
{
	this->tileSize = tileSize;
	lineThickness = this->tileSize / 20.0f;
	palette = &p;

	cursorShader = new ShaderProgram("./Resources/Shaders/Model.glsl", ShaderProgram::eRender);
	entityModeCursor = new Model();
	tileModeCursor = new Model();
	selectionRegion = new Model();

	glGenBuffers(1, &hintEntityDataBuffer);
	glGenBuffers(1, &hintEntityConnectorBuffer);

	entityShader = new ShaderProgram("./Resources/Shaders/Entity.glsl", ShaderProgram::eGeometry);
	connectorShader = new ShaderProgram("./Resources/Shaders/EntityConnector.glsl", ShaderProgram::eGeometry);
	const glm::mat4 rotationMtxDeg45 = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glUseProgram(entityShader->getProgramID());
	glUniformMatrix4fv(glGetUniformLocation(entityShader->getProgramID(), "rotationMtx45Deg"), 1, false, (float*)&rotationMtxDeg45);
	glUniform1i(glGetUniformLocation(entityShader->getProgramID(), "entitySpriteSheet"), 0); //Made assumption that GLTEXTURE_0 is active for this texture;
	glUniform1f(glGetUniformLocation(entityShader->getProgramID(), "tileSize"), tileSize); //For geometry shader
	glUseProgram(0);
	glUseProgram(connectorShader->getProgramID());
	glUniform1f(glGetUniformLocation(connectorShader->getProgramID(), "tileSize"), tileSize);
	glUseProgram(0);

	init();
}

Cursor::~Cursor()
{
	glFinish();
	glDeleteBuffers(1, &hintEntityDataBuffer);
	glDeleteBuffers(1, &hintEntityConnectorBuffer);

	delete cursorShader;
	delete entityShader;
	delete connectorShader;
	delete entityModeCursor;
	delete tileModeCursor;
	delete selectionRegion;
}

void Cursor::init()
{
	entityModeCursor->clearBuffers();
	tileModeCursor->clearBuffers();
	selectionRegion->clearBuffers();

	glm::vec4 lineColor = glm::vec4(palette->editorColors.cursorColor, 1.0f);
	float segmentWidth = this->tileSize / 8.0f;
	float cornerOffset = this->tileSize * 0.5f + lineThickness * 1.0f;

	entityModeCursor->addLine(glm::vec2(-segmentWidth, 0.0f), lineColor, glm::vec2(segmentWidth, 0.0f), lineColor, lineThickness);
	entityModeCursor->addLine(glm::vec2(0.0f, -segmentWidth), lineColor, glm::vec2(0.0f, segmentWidth), lineColor, lineThickness);
	entityModeCursor->setBuffers();

	glm::vec2 tl = glm::vec2(-cornerOffset, cornerOffset);
	glm::vec2 tr = glm::vec2(cornerOffset, cornerOffset);
	glm::vec2 bl = glm::vec2(-cornerOffset, -cornerOffset);
	glm::vec2 br = glm::vec2(cornerOffset, -cornerOffset);
	tileModeCursor->addLine(tl - glm::vec2(lineThickness * 0.5f, 0.0f), lineColor, tl + glm::vec2(segmentWidth, 0.0f), lineColor, lineThickness);
	tileModeCursor->addLine(tl + glm::vec2(0.0f, lineThickness * 0.5f), lineColor, tl + glm::vec2(0.0f, -segmentWidth), lineColor, lineThickness);
	tileModeCursor->addLine(tr + glm::vec2(lineThickness * 0.5f, 0.0f), lineColor, tr + glm::vec2(-segmentWidth, 0.0f), lineColor, lineThickness);
	tileModeCursor->addLine(tr + glm::vec2(0.0f, lineThickness * 0.5f), lineColor, tr + glm::vec2(0.0f, -segmentWidth), lineColor, lineThickness);
	tileModeCursor->addLine(bl - glm::vec2(lineThickness * 0.5f, 0.0f), lineColor, bl + glm::vec2(segmentWidth, 0.0f), lineColor, lineThickness);
	tileModeCursor->addLine(bl - glm::vec2(0.0f, lineThickness * 0.5f), lineColor, bl + glm::vec2(0.0f, segmentWidth), lineColor, lineThickness);
	tileModeCursor->addLine(br + glm::vec2(lineThickness * 0.5f, 0.0f), lineColor, br + glm::vec2(-segmentWidth, 0.0f), lineColor, lineThickness);
	tileModeCursor->addLine(br - glm::vec2(0.0f, lineThickness * 0.5f), lineColor, br + glm::vec2(0.0f, segmentWidth), lineColor, lineThickness);
	tileModeCursor->setBuffers();

	selectionRegion->setBuffers();

}

void Cursor::update(float mouseModelSpaceXCoord, float mouseModelSpaceYCoord)
{
	cursorLocation = glm::vec2(mouseModelSpaceXCoord, mouseModelSpaceYCoord);

	if (cursorType == ENTITY_PLACEMENT_CURSOR)
	{
		glm::ivec2 entityLocation = UtilityFunctions::clampToNearestPlaceableEntityCoord(cursorLocation, tileSize);
		currentHintEntity->entityCoordx = entityLocation.x;
		currentHintEntity->entityCoordy = entityLocation.y;
		setHintBuffer();
	}
}

void Cursor::draw(glm::mat4 viewProjMtx)
{
	glm::vec2 modelCoord = glm::vec2(0.0f);
	glm::mat4 modelMtx = glm::mat4(1.0f);
	glm::ivec2 closestCoord = glm::vec2(0.0f);

	switch (cursorType)
	{
	case ENTITY_CURSOR:
		closestCoord = UtilityFunctions::clampToNearestPlaceableEntityCoord(cursorLocation, tileSize);
		modelCoord = UtilityFunctions::convertEntityCoordToModelCoord(closestCoord, tileSize);
		modelMtx = glm::translate(glm::mat4(1.0f), glm::vec3(modelCoord.x, modelCoord.y, 0.0f));
		entityModeCursor->draw(modelMtx, viewProjMtx, cursorShader->getProgramID());
		break;
	case TILE_CURSOR:
		closestCoord = UtilityFunctions::clampToNearestTileCoord(cursorLocation, tileSize);
		modelCoord = UtilityFunctions::convertTileCoordToModelCoord(closestCoord, tileSize);
		modelMtx = glm::translate(glm::mat4(1.0f), glm::vec3(modelCoord.x, modelCoord.y, 0.0f));
		tileModeCursor->draw(modelMtx, viewProjMtx, cursorShader->getProgramID());
		break;
	case ENTITY_PLACEMENT_CURSOR:
		drawHintEntity(viewProjMtx);
		if (currentHintEntity == &hintEntities[1])
		{
			drawHintEntityConnector(viewProjMtx);
		}
		break;
	case REGION_SELECT_CURSOR:
		if (followMouse)
		{
			closestCoord = UtilityFunctions::clampToNearestTileCoord(cursorLocation, tileSize);
		}
		else
		{
			closestCoord = UtilityFunctions::clampToNearestTileCoord(selectionRegionDrawLocation, tileSize);
		}
		modelCoord = UtilityFunctions::convertTileCoordToModelCoord(closestCoord, tileSize);
		modelMtx = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f * (float)rotation), glm::vec3(0.0f, 0.0f, 1.0f));
		modelMtx = glm::translate(glm::mat4(1.0f), glm::vec3(modelCoord.x, modelCoord.y, 0.0f)) * modelMtx;
		selectionRegion->draw(modelMtx, viewProjMtx, cursorShader->getProgramID());
		break;
	default:
		break;
	}
}

void Cursor::buildSelectionRegionBuffer(glm::vec2 mouseModelSpaceStartCoord, glm::vec2 mouseModelSpaceEndCoord,
									 bool isEdgeInclusive, bool isMouseLeftHeld)
{
	//1. Build the border based on (0,0) as end location first, use absolute value as width and height
	//2. Flip the x and y based on positive/negative difference on end to start
	//3. Shift down and right half a tile
	//4. Move to end tile coord location
	selectionRegion->clearBuffers();

	//When you are building the selection region, it is not allowed to follow the mouse.
	followMouse = false;

	glm::vec4 lineColor = glm::vec4(palette->editorColors.cursorColor / 255.0f, 1.0f);
	if (!isMouseLeftHeld)
	{
		lineColor = glm::vec4(1.0f - lineColor.x, 1.0f - lineColor.y, 1.0f - lineColor.z, 1.0f);
	}

	glm::ivec2 startTileCoord = UtilityFunctions::clampToNearestTileCoord(mouseModelSpaceStartCoord, tileSize);
	glm::ivec2 endTileCoord = UtilityFunctions::clampToNearestTileCoord(mouseModelSpaceEndCoord, tileSize);
	float width = (std::abs(endTileCoord.x - startTileCoord.x)) * tileSize;
	float height = (std::abs(endTileCoord.y - startTileCoord.y)) * tileSize;

	float cornerOffset = this->tileSize * 0.5f;
	if (isEdgeInclusive)
	{
		cornerOffset += lineThickness * 1.0f;
	}
	else
	{
		cornerOffset -= lineThickness * 1.0f;
	}

	glm::vec2 tl = glm::vec2(-cornerOffset - width, cornerOffset + height);
	glm::vec2 tr = glm::vec2(cornerOffset, cornerOffset + height);
	glm::vec2 bl = glm::vec2(-cornerOffset - width, -cornerOffset);
	glm::vec2 br = glm::vec2(cornerOffset, -cornerOffset);

	//This should be calculated in the draw function instead
	//glm::vec2 modelOffset = glm::vec2(endTileCoord.x, -endTileCoord.y) * tileSize + glm::vec2(tileSize * 0.5f, -tileSize * 0.5f);
	selectionRegionDrawLocation = mouseModelSpaceEndCoord;
	rotation = 0;

	glm::vec2 flip = glm::vec2(1.0f, 1.0f);
	if (endTileCoord.x < startTileCoord.x) { flip.x = -flip.x; }
	if (endTileCoord.y < startTileCoord.y) { flip.y = -flip.y; }

	selectionRegion->addLine((tl - glm::vec2(lineThickness * 0.5f, 0.0f)) * flip, lineColor,
							 (tr + glm::vec2(lineThickness * 0.5f, 0.0f)) * flip, lineColor, lineThickness);
	selectionRegion->addLine((bl - glm::vec2(lineThickness * 0.5f, 0.0f)) * flip, lineColor,
							 (br + glm::vec2(lineThickness * 0.5f, 0.0f)) * flip, lineColor, lineThickness);
	selectionRegion->addLine((tl + glm::vec2(0.0f, lineThickness * 0.5f)) * flip, lineColor,
							 (bl - glm::vec2(0.0f, lineThickness * 0.5f)) * flip, lineColor, lineThickness);
	selectionRegion->addLine((tr + glm::vec2(0.0f, lineThickness * 0.5f)) * flip, lineColor,
							 (br - glm::vec2(0.0f, lineThickness * 0.5f)) * flip, lineColor, lineThickness);

	selectionRegion->setBuffers();
}

void Cursor::clearSelectionRegionBuffer()
{
	selectionRegion->clearBuffers();
}

void Cursor::rotateSelectionRegionClockwise()
{
	rotation += 4;
	rotation++;
	rotation = rotation % 4;
}

void Cursor::rotateSelectionRegionCounterClockwise()
{
	rotation += 4;
	rotation--;
	rotation = rotation % 4;
}

void Cursor::setFollowMouse(bool toFollow)
{
	followMouse = toFollow;
}

void Cursor::setCursorType(CursorType t)
{
	cursorType = t;
}

NumEntityToPlace Cursor::placeEntity()
{
	if (currentHintEntity->type == NONE)
	{
		return NO_ENTITY; //Prevent placing a ghost entity.
	}

	if (currentHintEntity->type != EXIT &&
		currentHintEntity->type != EXIT_SWITCH &&
		currentHintEntity->type != LOCKED_DOOR &&
		currentHintEntity->type != LOCKED_DOOR_SWITCH &&
		currentHintEntity->type != TRAP_DOOR &&
		currentHintEntity->type != TRAP_DOOR_SWITCH)
	{
		entityToPlace1 = hintEntities[0];
		entityToPlace1.rotation = expectedRotation;
		entityToPlace1.mode = expectedMode;
		return SINGLE_ENTITY;
	}

	if (currentHintEntity == &hintEntities[0])
	{

		//Copy location and rotation of hint entity 1 to hint entity 2
		hintEntities[1].entityCoordx = hintEntities[0].entityCoordx;
		hintEntities[1].entityCoordy = hintEntities[0].entityCoordy;
		hintEntities[1].rotation = expectedRotation;
		hintEntities[1].mode = expectedMode;

		//Find the correct type pair for the other entity
		switch (hintEntities[0].type)
		{
		case EXIT:
			hintEntities[1].type = EXIT_SWITCH; break;
		case EXIT_SWITCH:
			hintEntities[1].type = EXIT; break;
		case LOCKED_DOOR:
			hintEntities[1].type = LOCKED_DOOR_SWITCH; break;
		case LOCKED_DOOR_SWITCH:
			hintEntities[1].type = LOCKED_DOOR; break;
		case TRAP_DOOR:
			hintEntities[1].type = TRAP_DOOR_SWITCH; break;
		case TRAP_DOOR_SWITCH:
			hintEntities[1].type = TRAP_DOOR; break;
		default:
			break;
		}

		Entity::sanitizeImpossibleValue(hintEntities[1]);

		swapHintPointer();
		setHintBuffer();
		return NO_ENTITY; //Need more information as this is a pair entity
	}
	else
	{
		entityToPlace1 = hintEntities[0];
		entityToPlace2 = hintEntities[1];
		entityToPlace1.rotation = expectedRotation;
		entityToPlace2.rotation = expectedRotation;
		entityToPlace1.mode = expectedMode;
		entityToPlace2.mode = expectedMode;

		//Copy location and rotation of hint entity 2 to hint entity 1
		hintEntities[0].entityCoordx = hintEntities[1].entityCoordx;
		hintEntities[0].entityCoordy = hintEntities[1].entityCoordy;
		hintEntities[0].rotation = expectedRotation;
		hintEntities[0].mode = expectedMode;

		hintEntities[1].type = NONE; //Additionally, since the second hint entity is always being drawn, set the type to none to prevent drawing.

		//Find the correct type pair for the other entity
		switch (hintEntities[1].type)
		{
		case EXIT:
			hintEntities[0].type = EXIT_SWITCH; break;
		case EXIT_SWITCH:
			hintEntities[0].type = EXIT; break;
		case LOCKED_DOOR:
			hintEntities[0].type = LOCKED_DOOR_SWITCH; break;
		case LOCKED_DOOR_SWITCH:
			hintEntities[0].type = LOCKED_DOOR; break;
		case TRAP_DOOR:
			hintEntities[0].type = TRAP_DOOR_SWITCH; break;
		case TRAP_DOOR_SWITCH:
			hintEntities[0].type = TRAP_DOOR; break;
		default:
			break;
		}

		Entity::sanitizeImpossibleValue(hintEntities[0]);

		swapHintPointer();
		setHintBuffer();
		return PAIR_ENTITY;
	}
}

EntityData Cursor::getFirstEntityData()
{
	return entityToPlace1;
}

EntityData Cursor::getSecondEntityData()
{
	return entityToPlace2;
}

void Cursor::setHintEntityType(entityType type)
{
	currentHintEntity->type = type;
	Entity::sanitizeImpossibleValue(*currentHintEntity);
	setHintBuffer();
}

void Cursor::setHintEntityRotation(EntityRotation rotation)
{
	expectedRotation = rotation;
	currentHintEntity->rotation = expectedRotation;
	Entity::sanitizeImpossibleValue(*currentHintEntity);
	setHintBuffer();
}

void Cursor::setHintEntityMode(EntityMode mode)
{
	expectedMode = mode;
	currentHintEntity->mode = expectedMode;
	Entity::sanitizeImpossibleValue(*currentHintEntity);
	setHintBuffer();
}

void Cursor::resetHintEntity()
{
	hintEntities[0].type = NONE;
	hintEntities[1].type = NONE;
	currentHintEntity = &hintEntities[0];
	Entity::sanitizeImpossibleValue(*currentHintEntity);
}

void Cursor::swapHintPointer()
{
	if (currentHintEntity == &hintEntities[0])
	{
		currentHintEntity = &hintEntities[1];
	}
	else
	{
		currentHintEntity = &hintEntities[0];
	}
	currentHintEntity->rotation = expectedRotation;
	currentHintEntity->mode = expectedMode;
	Entity::sanitizeImpossibleValue(*currentHintEntity);
}

void Cursor::setHintBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, hintEntityDataBuffer);
	glBufferData(GL_ARRAY_BUFFER, hintEntities.size() * sizeof(EntityData), &hintEntities[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	ConnectorShaderInfo temp;
	temp.entity1Coord = glm::ivec2(hintEntities[0].entityCoordx, hintEntities[0].entityCoordy);
	temp.entity2Coord = glm::ivec2(hintEntities[1].entityCoordx, hintEntities[1].entityCoordy);

	glBindBuffer(GL_ARRAY_BUFFER, hintEntityConnectorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ConnectorShaderInfo), &temp, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void Cursor::clearHintBuffer()
{
	//Hint entities doesn't actually get cleared and instead use a flag to determine if it is drawn or not
}

void Cursor::drawHintEntity(glm::mat4 viewProjMtx)
{

	// Set up shader
	glDisable(GL_CULL_FACE);
	glUseProgram(entityShader->getProgramID());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, palette->getEntityTextureID());

	glUniformMatrix4fv(glGetUniformLocation(entityShader->getProgramID(), "viewProjMtx"), 1, false, (float*)&viewProjMtx);

	// Set up state
	glBindBuffer(GL_ARRAY_BUFFER, hintEntityDataBuffer);

	uint entityTypeLoc = 0;
	glEnableVertexAttribArray(entityTypeLoc);
	glVertexAttribIPointer(entityTypeLoc, 1, GL_INT, sizeof(EntityData), (void*)(0 * sizeof(int)));

	uint entityCoordLoc = 1;
	glEnableVertexAttribArray(entityCoordLoc);
	glVertexAttribIPointer(entityCoordLoc, 2, GL_INT, sizeof(EntityData), (void*)(1 * sizeof(int)));

	uint entityRotationLoc = 2;
	glEnableVertexAttribArray(entityRotationLoc);
	glVertexAttribIPointer(entityRotationLoc, 1, GL_INT, sizeof(EntityData), (void*)(3 * sizeof(int)));

	uint entityModeLoc = 3;
	glEnableVertexAttribArray(entityModeLoc);
	glVertexAttribIPointer(entityModeLoc, 1, GL_INT, sizeof(EntityData), (void*)(4 * sizeof(int)));

	uint entityColorLoc = 4;
	glEnableVertexAttribArray(entityColorLoc);
	glVertexAttribPointer(entityColorLoc, 3, GL_FLOAT, GL_FALSE, sizeof(EntityData), (void*)(5 * sizeof(int)));

	uint entityHighlightLoc = 5;
	glEnableVertexAttribArray(entityHighlightLoc);
	glVertexAttribIPointer(entityHighlightLoc, 1, GL_INT, sizeof(EntityData), (void*)(5 * sizeof(int) + 3 * sizeof(float)));

	glDrawArrays(GL_POINTS, 0, hintEntities.size()); //Use GL_POINTS because every "point" is 1 EntityData.

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnable(GL_CULL_FACE);

	glUseProgram(0);
}

void Cursor::drawHintEntityConnector(glm::mat4 viewProjMtx)
{

	// Set up shader
	glDisable(GL_CULL_FACE);
	glUseProgram(connectorShader->getProgramID());

	glUniformMatrix4fv(glGetUniformLocation(connectorShader->getProgramID(), "viewProjMtx"), 1, false, (float*)&viewProjMtx);

	// Set up state
	glBindBuffer(GL_ARRAY_BUFFER, hintEntityConnectorBuffer);

	uint entity1CoordLoc = 0;
	glEnableVertexAttribArray(entity1CoordLoc);
	glVertexAttribIPointer(entity1CoordLoc, 2, GL_INT, sizeof(ConnectorShaderInfo), (void*)(0 * sizeof(int)));

	uint entity2CoordLoc = 1;
	glEnableVertexAttribArray(entity2CoordLoc);
	glVertexAttribIPointer(entity2CoordLoc, 2, GL_INT, sizeof(ConnectorShaderInfo), (void*)(2 * sizeof(int)));

	uint entityHighlightLoc = 2;
	glEnableVertexAttribArray(entityHighlightLoc);
	glVertexAttribIPointer(entityHighlightLoc, 1, GL_INT, sizeof(ConnectorShaderInfo), (void*)(4 * sizeof(int)));

	glDrawArrays(GL_POINTS, 0, 1); //There is only 1 connector

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnable(GL_CULL_FACE);

	glUseProgram(0);
}