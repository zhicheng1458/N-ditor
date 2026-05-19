#pragma once

#include "Core.h"

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
	CURVE_OUT = 8,
	BORDER_TELEPORT = 9
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