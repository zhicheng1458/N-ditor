#pragma once
#include "Core.h"
#include "Shader.h"
#include "AABBTree.h"

#include "Tile.h"
#include "Palette.h"

#include "Action.h"

#include "UtilityFunctions.h"

class TileHandler
{
public:
	TileHandler(float tileSize, const Palette& p);
	~TileHandler();
	void clearData();

	void usePalette(const Palette& p);

	void addTile(TileData data, Modification& recorder);
	void addTile(int tileCoordX, int tileCoordY, TileRotation rotation, TileType type, Modification& recorder);
	void undo(const Modification& changes);
	void redo(const Modification& changes);
	//void deleteTile(TileData data, Modification& recorder);

	//void update(); //Only matters once you have animation
	void draw(glm::mat4 viewProjMtx);

	void setHintToFollowMouse(bool toFollow);
	void moveHint(glm::vec2 cursorInModelSpace); //cursorX and cursorY are supplied as entity coordinate at the mouse location.
	//corner1 should be when mouse is held, corner2 should be when mouse is released
	//pivot will be based on corner 2. Selected region is always inclusive
	void setSelectedRegion(glm::vec2 corner1, glm::vec2 corner2);
	void stageSelected();
	void unstageSelected();
	void pasteSelected(Modification& recorder);
	void fillSelected(Modification& recorder);
	void deleteSelected(Modification& recorder);
	void copySelected();
	void cutSelected(Modification& recorder);
	void flipSelectedHorizontally();
	void flipSelectedVertically();
	void rotateSelectedClockwise();
	void rotateSelectedCounterClockwise();
	void invertSelected();

	const std::vector<TileData*> & getTileData() const;

private:

	const Palette* palette;

	float tileSize;

	//Tiles
	glm::mat4 modelMtx = glm::mat4(1.0f); //Tiles are ALWAYS counted from origin
	ShaderProgram * tileShader;
	std::vector<TileData *> tileDatas;
	//Solution for tree:
	//Instead of deleting the object in the tileDatas, keep it as emptytile data
	//For the AABB tree, every time an object is inserted to the tileDatas vector,
	//A corresponding node with its index in the tileDatas is added to the tree.
	//This way draw will be extremely fast while we can still utilize the tree
	//for insert and remove
	AABBTree * tileTree;

	uint tileDataBuffer;
	bool allowModify = false;

	bool followMouse = false;
	glm::ivec2 selectRegionMinBoundary = glm::ivec2(0);
	glm::ivec2 selectRegionMaxBoundary = glm::ivec2(0);
	glm::ivec2 pivotTileLocation = glm::ivec2(0); //Tile location of the pivot
	std::vector<TileData*> volatileTileData;
	uint volatileTileDataBuffer;

	void drawTiles(glm::mat4 viewProjMtx, const uint tileBuffer, GLsizei tileBufferSize);

	void setBuffers();
	void clearBuffers();
	void setVolatileBuffers();
	void clearVolatileBuffers();

	//Which 2 corners doesn't matter as long as they are diagonal corner of each other
	void deleteSelectedTileCoordBased(glm::ivec2 corner1, glm::ivec2 corner2, Modification& recorder);
	void flipTileHorizontally(TileData& tile, int xMinBoundary, int xMaxBoundary);
	void flipTileVertically(TileData& tile, int yMinBoundary, int yMaxBoundary);
	void rotateTileClockwise(TileData& tile, const glm::ivec2& pivot);
	void rotateTileCounterClockwise(TileData& tile, const glm::ivec2& pivot);
	void invertTile(TileData& tile);

	bool isSameCoord(TileData data, int tileCoordx, int tileCoordy);
};