#pragma once
#include "Core.h"
#include "Shader.h"
#include "AABBTree.h"
#include "Palette.h"

//MAKE SURE THE SHADER IS USING EACH TYPE TO ITS MEANING AS LISTED HERE
enum TileType
{
	EMPTY = -1,
	FULL = 0,
	SLOPE_45DEG = 1,
	SMALLSLOPE_RIGHT_60DEG = 2,
	SMALLSLOPE_LEFT_60DEG = 3,
	CURVE_IN = 4,
	HALF = 5,
	LARGESLOPE_RIGHT_60DEG = 6,
	LARGESLOPE_LEFT_60DEG = 7,
	CURVE_OUT = 8
};

//In actual editor, the tile is rotated CLOCKWISE;
enum TileRotation
{
	TILE_DEGREE_0 = 0,
	TILE_DEGREE_90 = 1,
	TILE_DEGREE_180 = 2,
	TILE_DEGREE_270 = 3
};

struct TileData
{
	int type;
	int tileCoordx;
	int tileCoordy; //each tile takes up 1 unit, so top left tile is (0, 0);
	int rotation;
	glm::vec3 color;
};

class TileHandler
{
public:
	TileHandler(float tileSize, const Palette& p);
	~TileHandler();

	void usePalette(const Palette& p);

	void addTile(TileData data);
	void deleteTile(TileData data);

	//void update(); //Only matters once you have animation
	void draw(glm::mat4 viewProjMtx);

private:

	const Palette* palette;

	bool isSameCoord(TileData data1, TileData data2);

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
	//uint indexBuffer;
	bool allowModify = false;

	void setBuffers();
	void clearBuffers();

	float tileSize;
};