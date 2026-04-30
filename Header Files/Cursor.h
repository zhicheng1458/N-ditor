#pragma once
#include "Core.h"
#include "UtilityFunctions.h"

#include "Model.h"

#include "Palette.h"

enum CursorType
{
	TILE_CURSOR = 0,
	ENTITY_CURSOR = 1,
	ENTITY_PLACEMENT_CURSOR = 2,
	REGION_SELECT_CURSOR = 3
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

private:
	//Maybe consider taking over the hint entity here?

	const Palette* palette;

	float tileSize;
	float lineThickness;

	CursorType cursorType = TILE_CURSOR;
	glm::vec2 cursorLocation = glm::vec2(0.0f);

	ShaderProgram* cursorShader;
	Model * entityModeCursor;
	Model * tileModeCursor;
	Model * selectionRegion;
	glm::vec2 selectionRegionDrawLocation = glm::vec2(0.0f);
	int rotation = 0;
	bool followMouse = false;

	void init(); //Build the cursor
};
