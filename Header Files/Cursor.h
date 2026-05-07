#pragma once
#include "Core.h"
#include "UtilityFunctions.h"

#include "Model.h"

#include "Palette.h"
#include "Entity.h"

enum CursorType
{
	TILE_CURSOR = 0,
	ENTITY_CURSOR = 1,
	ENTITY_PLACEMENT_CURSOR = 2,
	REGION_SELECT_CURSOR = 3
};

enum NumEntityToPlace
{
	NO_ENTITY = 0,
	SINGLE_ENTITY = 1,
	PAIR_ENTITY = 2
};

class Cursor
{
public:
	Cursor(float tileSize, const Palette& p);
	~Cursor();

	void update(float mouseModelSpaceXCoord, float mouseModelSpaceYCoord);
	void draw(glm::mat4 viewProjMtx);

	void buildSelectionRegionBuffer(glm::vec2 mouseModelSpaceStartCoord, glm::vec2 mouseModelSpaceEndCoord, bool isEdgeInclusive, bool isMouseLeftHeld);
	void clearSelectionRegionBuffer();
	void rotateSelectionRegionClockwise();
	void rotateSelectionRegionCounterClockwise();
	void setFollowMouse(bool toFollow);

	void setCursorType(CursorType t);

	NumEntityToPlace placeEntity();
	EntityData getFirstEntityData();
	EntityData getSecondEntityData();
	void setHintEntityType(entityType type);
	void setHintEntityRotation(EntityRotation rotation);
	void resetHintEntity(); //For when the cursor mode was changed mid placing entity pair

private:

	const Palette* palette;

	float tileSize;
	float lineThickness;

	CursorType cursorType = TILE_CURSOR;
	glm::vec2 cursorLocation = glm::vec2(0.0f);

	ShaderProgram* cursorShader;
	ShaderProgram* entityShader;
	ShaderProgram* connectorShader;
	Model * entityModeCursor;
	Model * tileModeCursor;
	Model * selectionRegion;
	glm::vec2 selectionRegionDrawLocation = glm::vec2(0.0f);
	int rotation = 0;
	bool followMouse = false;

	std::vector<EntityData> hintEntities = {EntityData(), EntityData()};
	EntityData* currentHintEntity = &hintEntities[0];
	EntityConnector hintConnector;
	EntityData entityToPlace1; //Use to remember the data for EntityHandler to place the entity
	EntityData entityToPlace2; //Use to remember the data for EntityHandler to place the entity
	EntityRotation expectedRotation = ENTITY_DEGREE_0;
	uint hintEntityDataBuffer;
	uint hintEntityConnectorBuffer;

	void init(); //Build the cursor

	void swapHintPointer();

	void setHintBuffer();
	void clearHintBuffer();
	void drawHintEntity(glm::mat4 viewProjMtx);
	void drawHintEntityConnector(glm::mat4 viewProjMtx);
};
