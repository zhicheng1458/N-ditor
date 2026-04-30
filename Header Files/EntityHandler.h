#pragma once
#include "Core.h"
#include "Shader.h"
#include "Texture.h"
#include "Palette.h"
#include "AABBTree.h"

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

/*
struct EntityDataAdaptor
{
	std::vector<EntityData> entityDatas;

	// Must return the number of data poins
	size_t kdtree_get_point_count() const
	{
		return entityDatas.size();
	}
	// Must return the dim'th component of the idx'th point in the class:
	float kdtree_get_pt(const size_t idx, int dim) const
	{
		if (dim == 0) { return (float)entityDatas[idx].entityCoordx; }
		else if (dim == 1) { return (float)entityDatas[idx].entityCoordy; }
		else { return 0.0f; }
	}
	// Optional bounding-box computation: return false to default to a standard bbox computation loop.
	// Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
	// Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
	template <class BBOX>
	bool kdtree_get_bbox(BBOX& bb) const
	{
		return false;
	}
};
*/

class EntityHandler
{
public:
	EntityHandler(float tileSize, const Palette& p);
	~EntityHandler();

	void usePalette(const Palette& p);

	void deleteClosestEntity(const glm::vec2 entityCoord, const float radius);
	void highlightClosestEntity(const glm::vec2 entityCoord, const float radius);
	void moveHighlightedEntity(int cursorX, int cursorY);
	void setHighlightedEntityRotation(EntityRotation rotation);
	void stopHighlight();

	//Return a pointer to the entity within the vector, this allows the editor to easily change the value
	void showHintEntity();
	void hideHintEntity();
	//This will always reset previous hint, such as door and door switches. Coord supplied as entity coordinate.
	void setHintEntity(entityType type, int cursorXCoord, int cursorYCoord, EntityRotation rotation); 
	void setHintEntityRotation(EntityRotation rotation);
	void placeEntity();
	void moveHint(int cursorX, int cursorY); //cursorX and cursorY are supplied as entity coordinate at the mouse location.

	void update();
	void draw(glm::mat4 viewProjMtx);

private:

	const char* SPRITESHEET_PATH = "./Resources/Textures/EntitiesSpriteSheetV2.png";
	const Palette* palette = nullptr;
	float tileSize;

	ShaderProgram* entityShader;
	ShaderProgram* connectorShader;

	std::vector<EntityData*> entityDatas;
	std::vector<EntityConnector> entityConnections;

	uint entityDataBuffer;
	uint entityConnectorBuffer;
	bool allowModify = false;

	EntityData* lastHighlightedEntity = nullptr; //For tracking highlight

	//Specifically for when needing to draw the entity indicator on your cursor
	//The level editor must manage the hint entity itself, this class only helps to
	//draw the hint
	bool drawHintEntity = false;
	bool hintEntity1Placed = false;
	//For displaying on cursor when selecting entity. 
	EntityData* hintEntity1 = nullptr; //Request the pointer from entityHandler. Process request on pressing space
	EntityData* hintEntity2 = nullptr; //Request the pointer from entityHandler. Process request on first click with door type entity.
	std::vector<EntityData*> volatileEntityStaticData; //For those selected outside of the region
	std::vector<EntityData*> volatileEntityDynamicDatas; //For those selected within the region
	std::vector<EntityConnector> volatileEntityConnections;
	uint volatileEntityDataBuffer;
	uint volatileEntityConnectorBuffer;

	void drawEntities(glm::mat4 viewProjMtx);
	void drawConnectors(glm::mat4 viewProjMtx, const std::vector<EntityConnector>& connectionDatas, const uint connectorBuffer);
	void drawHint(glm::mat4 viewProjMtx);

	bool isSame(EntityData data1, EntityData data2);
	bool isLessThan(EntityData data1, EntityData data2);

	void addStaticEntity(EntityData data);
	void addStaticEntity(EntityData data, EntityData pair);
	void deleteEntity(EntityData* data);
	EntityData* findClosestEntity(const glm::vec2 entityCoord, const float radius);
	void resolveHighlight(EntityData* entityToHighlight, EntityData* entityToUnHighlight);

	void setStaticBuffers();
	void clearStaticBuffers();
	void setVolatileBuffers();
	void clearVolatileBuffers();

};