#pragma once

#include "Core.h"
#include "UtilityFunctions.h"

#include "Model.h"
#include "Shader.h"
#include "Camera.h"

#include "Palette.h"
#include "TileHandler.h"
#include "EntityHandler.h"
#include "EditorOverlay.h"
#include "Cursor.h"

enum editingMode
{
	TILE_EDITING_MODE = 0,
	ENTITY_EDITING_MODE = 1,
	ENTITY_PLACEMENT_MODE = 2,
	REGION_EDITING_MODE = 3
};

class LevelEditor
{
public:
	LevelEditor();
	LevelEditor(float screenWidth, float screenHeight);
	~LevelEditor();

	void loadPalette(const char * paletteFolderPath); //TODO

	void update();
	void drawSolidLayer();
	void drawTransparentLayer();
	void drawOverlay();

	Camera * getCamera() { return viewpoint; }

	void keyboard(GLFWwindow * window, int key, int scancode, int action, int mods);
	void mouseButton(GLFWwindow * window, int button, int action, int mods);
	void mouseMotion(GLFWwindow * window, double xpos, double ypos);
	void scrolling(GLFWwindow * window, double xoffset, double yoffset);
	void resize(GLFWwindow* window, int width, int height);

	void setTileTypeKeyModifier(int key);
	TileType getTileTypeKeyModifier();
	void setTileRotationKeyModifier(int key);
	TileRotation getTileRotationKeyModifier();

	const editingMode & getEditingMode();
	const float getGridSpacing();

	//Colors (Use palette for these color)
	//glm::vec3 backgroundColor = glm::vec3(140.0f / 255.0f, 148.0f / 255.0f, 136.0f / 255.0f);
	//glm::vec3 tileColor = glm::vec3(79.0f / 255.0f, 86.0f / 255.0f, 87.0f / 255.0f);
	//glm::vec3 gridColor = glm::vec3(1.0f);

private:

	float screenWidth = 1600.0f;
	float screenHeight = 900.0f;
	glm::mat4 modelMtx; //The whole editor as a singular model

	enum GridLevel
	{
		LOW = 1,
		MEDIUM = 2,
		HIGH = 4
	};

	int currentGridLevel = (int)HIGH;

	//Camera
	Camera * viewpoint;

	//Grid
	Model * grid;
	ShaderProgram * gridShader;
	const float gridlineLength = 5000.0f;
	const float gridSpacing = 40.0f; //This also dictate the size of the tile
	const float gridlineThickness = 1.0f;

	Model * fineGrid;
	ShaderProgram * fineGridShader;
	const float fineGridSpacing = gridSpacing / (float)currentGridLevel;
	const float fineGridlineThickness = gridlineThickness * 0.5f;
	bool drawFineGrid = false;

	//Region Selector (playable region + editable region)
	glm::mat4 levelRegionModelMtx;
	Model * levelRegion;
	ShaderProgram * levelRegionShader;
	const float playableRegionLineWidth = gridSpacing * 42.0f; //The playable region is 42 tiles wide
	const float playableRegionLineHeight = gridSpacing * 27.0f; //The playable region is 27 tiles high
	const float playableRegionLineThickness = 4.0f;
	const glm::vec4 playableRegionLineColor = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
	const float entityPlaceableRegionLineWidth = gridSpacing * 0.25f * 255.0f; //The placeable region for entities is 64 tiles - 1 quartertile wide
	const float entityPlaceableRegionLineHeight = gridSpacing * 0.25f * 255.0f; //The placeable region for entities is 42 tiles - 1 quartertile high
	const float entityPlaceableRegionLineThickness = 6.0f;
	const glm::vec4 entityPlaceableRegionLineColor = glm::vec4(0.0f, 0.2f, 0.2f, 1.0f);

	//Palette
	Palette palette = Palette();

	//Tiles
	TileHandler tiles = TileHandler(gridSpacing, palette);

	//Entities
	EntityHandler entities = EntityHandler(gridSpacing, palette);

	//Overlay
	EditorOverlay overlay = EditorOverlay(screenWidth, screenHeight, gridSpacing);

	//Cursor
	Cursor mouse = Cursor(gridSpacing, palette);

	//Toggle for tile-editing mode vs entity-editing mode;
	editingMode currentEditingMode = TILE_EDITING_MODE;

	glm::ivec2 oldTileCoordinate = glm::vec2(0);
	glm::ivec2 oldEntityCoordinate = glm::vec2(0);
	TileType currentTileType = SLOPE_45DEG; //Default tile type is 45 deg tile
	TileRotation currentTileRotation = TILE_DEGREE_0;
	EntityRotation currentEntityRotation = ENTITY_DEGREE_0;
	const float MAX_HIGHLIGHT_SEARCH_RADIUS = 5.0f;

	//Keyboard and Mouse
	bool leftDown, middleDown, rightDown;
	float MouseX, MouseY; //For purely tracking mouse location
	double mouseLeftButtonPressedX, mouseLeftButtonPressedY, mouseLeftButtonReleasedX, mouseLeftButtonReleasedY; //For tracking leftclicking
	double mouseLeftButtonPressedStartTime, mouseLeftButtonPressedEndTime;
	bool keyStates[GLFW_KEY_LAST + 1];

	void init();
};