#pragma once
#include "Core.h"
#include "Shader.h"
#include "Texture.h"
#include "Palette.h"
#include "AABBTree.h"

#include "Entity.h"
#include "UtilityFunctions.h"

#include <algorithm>

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
	void clearData();

	void usePalette(const Palette& p);

	void deleteClosestEntity(const glm::vec2 entityCoord, const float radius);
	void highlightClosestEntity(const glm::vec2 entityCoord, const float radius);
	void moveHighlightedEntity(int cursorX, int cursorY);
	void setHighlightedEntityRotation(EntityRotation rotation);
	void setHighlightedEntityMode(EntityMode mode);
	void stopHighlight();

	bool addStaticEntity(EntityData data);
	bool addStaticEntity(EntityData data, EntityData pair);

	void setHintToFollowMouse(bool toFollow);
	void moveHint(glm::vec2 cursorInModelSpace); //cursorX and cursorY are supplied as entity coordinate at the mouse location.

	//corner1 should be when mouse is held, corner2 should be when mouse is released
	//pivot will be based on corner 2
	//TODO: QOL: All entities within the region will be staged; pair entities outside
	//of the region will be stages as static entities.
	void setSelectedRegion(glm::vec2 corner1, glm::vec2 corner2, bool isInclusive);
	void stageSelected();
	void unstageSelected();
	void pasteSelected();
	void deleteSelected();
	void copySelected();
	void cutSelected();
	void flipSelectedHorizontally();
	void flipSelectedVertically();
	void rotateSelectedClockwise();
	void rotateSelectedCounterClockwise();

	void sortEntityListByType();
	const std::vector<EntityData*>& getEntityData() const;
	const std::vector<EntityConnector>& getEntityConnectorData() const;

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

	bool followMouse = false;
	glm::ivec2 selectRegionMinBoundary = glm::ivec2(0);
	glm::ivec2 selectRegionMaxBoundary = glm::ivec2(0);
	glm::ivec2 pivotEntityLocation = glm::ivec2(0); //Entity location of the pivot
	std::vector<EntityData*> volatileEntityStaticData; //For those selected outside of the region
	std::vector<EntityData*> volatileEntityDynamicDatas; //For those selected within the region
	std::vector<EntityConnector> volatileEntityConnections;
	uint volatileEntityDataBuffer;
	uint volatileEntityConnectorBuffer;

	std::vector<entityType> dupeAllowedList = {GOLD, EVIL_NINJA}; //TODO: allow user to directly modify this list

	void drawEntities(glm::mat4 viewProjMtx, const uint entityBuffer, GLsizei entityBufferSize);
	void drawConnectors(glm::mat4 viewProjMtx, const uint connectorBuffer, GLsizei connectorBufferSize);

	void deleteEntity(EntityData* data);
	EntityData* findClosestEntity(const glm::vec2 entityCoord, const float radius);
	void resolveHighlight(EntityData* entityToHighlight, EntityData* entityToUnHighlight);

	void setStaticBuffers();
	void clearStaticBuffers();
	void setVolatileBuffers();
	void clearVolatileBuffers();

	void flipEntityHorizontally(EntityData & entity, int xMinBoundary, int xMaxBoundary);
	void flipEntityVertically(EntityData & entity, int yMinBoundary, int yMaxBoundary);
	void rotateEntityClockwise(EntityData & entity, const glm::ivec2 & pivot);
	void rotateEntityCounterClockwise(EntityData & entity, const glm::ivec2 & pivot);

	//bool sortByType(EntityData * e1, EntityData * e2);
};