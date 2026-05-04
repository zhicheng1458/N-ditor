#pragma once
#include "Core.h"

//MAKE SURE THE SHADER IS USING EACH TYPE TO ITS MEANING AS LISTED HERE
enum entityType
{
	NINJA = 0,
	MINE = 1,
	GOLD = 2,
	EXIT = 3,
	EXIT_SWITCH = 4,
	REGULAR_DOOR = 5,
	LOCKED_DOOR = 6,
	LOCKED_DOOR_SWITCH = 7,
	TRAP_DOOR = 8,
	TRAO_DOOR_SWITCH = 9,
	BOUNCE_PAD = 10,
	ONE_WAY = 11,
	CHAINGUN = 12,
	LASER_DRONE = 13,
	REGULAR_DRONE = 14,
	CHASE_DRONE = 15,
	FLOOR_GUARD = 16,
	BOUNCE_BLOCK = 17,
	ROCKET = 18,
	GAUSS = 19,
	THWUMP = 20,
	TOGGLE_MINE = 21,
	EVIL_NINJA = 22,
	LASER_TURRET = 23,
	BOOST_PAD = 24,
	DEATHBALL = 25,
	MICRO_DRONE = 26,
	EYEBAT = 27,
	SHOVE_THWUMP = 28,
	NONE = 29
};

//In actual editor, the tile is rotated CLOCKWISE;
enum EntityRotation
{
	ENTITY_DEGREE_0 = 0,
	ENTITY_DEGREE_45 = 1,
	ENTITY_DEGREE_90 = 2,
	ENTITY_DEGREE_135 = 3,
	ENTITY_DEGREE_180 = 4,
	ENTITY_DEGREE_225 = 5,
	ENTITY_DEGREE_270 = 6,
	ENTITY_DEGREE_315 = 7,
};

struct EntityData
{
	int type = NONE;
	int entityCoordx = 0;
	int entityCoordy = 0;
	int rotation = ENTITY_DEGREE_0;
	glm::vec3 color = glm::vec3(0.0f);
	int highlight = 0; //0 for no highlight, 1 for highlight;
	EntityData* pair = nullptr;
};

struct EntityConnector
{
	EntityData* e1 = nullptr;
	EntityData* e2 = nullptr;
	int highlight = 0; //0 for no highlight, 1 for highlight;
};

//Shader can't dereference pointer so we need a struct that has all the value already
struct ConnectorShaderInfo
{
	glm::ivec2 entity1Coord;
	glm::ivec2 entity2Coord;
	int highlight; //0 for no highlight, 1 for highlight;
};