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

	init();
}

Cursor::~Cursor()
{
	glFinish();
	delete cursorShader;
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
	rotation--;
	rotation = rotation % 4;
}

void Cursor::rotateSelectionRegionCounterClockwise()
{
	rotation += 4;
	rotation++;
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


