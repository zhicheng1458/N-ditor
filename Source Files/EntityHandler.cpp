#include "EntityHandler.h"
#include <iostream>
#include "TileHandler.h"

EntityHandler::EntityHandler(float tileSize, const Palette& p)
{
	entityShader = new ShaderProgram("./Resources/Shaders/Entity.glsl", ShaderProgram::eGeometry);
	connectorShader = new ShaderProgram("./Resources/Shaders/EntityConnector.glsl", ShaderProgram::eGeometry);
	this->tileSize = tileSize;

	const glm::mat4 rotationMtxDeg45 = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glUseProgram(entityShader->getProgramID());
	glUniformMatrix4fv(glGetUniformLocation(entityShader->getProgramID(), "rotationMtx45Deg"), 1, false, (float*)&rotationMtxDeg45);
	glUniform1i(glGetUniformLocation(entityShader->getProgramID(), "entitySpriteSheet"), 0); //Made assumption that GLTEXTURE_0 is active for this texture;
	glUniform1f(glGetUniformLocation(entityShader->getProgramID(), "tileSize"), tileSize); //For geometry shader
	glUseProgram(0);
	glUseProgram(connectorShader->getProgramID());
	glUniform1f(glGetUniformLocation(connectorShader->getProgramID(), "tileSize"), tileSize);
	glUseProgram(0);

	glGenBuffers(1, &entityDataBuffer);
	glGenBuffers(1, &pairEntityDataBuffer);
	glGenBuffers(1, &volatileEntityDataBuffer);
	glGenBuffers(1, &volatileEntityConnectorBuffer);

	this->usePalette(p);
	this->setStaticBuffers();
}

EntityHandler::~EntityHandler()
{
	glFinish();
	delete entityShader;
	delete connectorShader;

	for (int i = 0; i < entityDatas.size(); i++)
	{
		delete entityDatas[i];
		entityDatas[i] = nullptr;
	}
	for (int i = 0; i < pairEntityDatas.size(); i++)
	{
		delete pairEntityDatas[i];
		pairEntityDatas[i] = nullptr;
	}
	clearVolatileBuffers();

	glDeleteBuffers(1, &entityDataBuffer);
	glDeleteBuffers(1, &pairEntityDataBuffer);
	glDeleteBuffers(1, &volatileEntityDataBuffer);
	glDeleteBuffers(1, &volatileEntityConnectorBuffer);
}

void EntityHandler::clearData()
{
	for (int i = 0; i < entityDatas.size(); i++)
	{
		delete entityDatas[i];
		entityDatas[i] = nullptr;
	}
	for (int i = 0; i < pairEntityDatas.size(); i++)
	{
		delete pairEntityDatas[i];
		pairEntityDatas[i] = nullptr;
	}
	entityDatas.clear();
	pairEntityDatas.clear();

	clearStaticBuffers();
	setStaticBuffers();
	clearVolatileBuffers();
	setVolatileBuffers();
}

void EntityHandler::usePalette(const Palette& p)
{
	palette = &p;

	glUseProgram(entityShader->getProgramID());
	glUniform1i(glGetUniformLocation(entityShader->getProgramID(), "textureWidth"), palette->textureWidth);
	glUniform1i(glGetUniformLocation(entityShader->getProgramID(), "textureHeight"), palette->textureHeight);
	glUniform1i(glGetUniformLocation(entityShader->getProgramID(), "widthPerEntity"), palette->maximumWidthPerEntity);
	glUseProgram(0);
}

//TODO: Fix searching algorithm since pair and non pair are separately stored now
void EntityHandler::deleteClosestEntity(const glm::vec2 entityCoord, const float radius, Modification& recorder)
{
	EntityData* closestEntity = findClosestEntity(entityCoord, radius);
	if (lastHighlightedEntity == closestEntity)
	{
		stopHighlight();
		deleteEntityByAddress(closestEntity, recorder);
		closestEntity = nullptr;
		highlightClosestEntity(entityCoord, radius);
	}
	else
	{
		deleteEntityByAddress(closestEntity, recorder);
		closestEntity = nullptr;
	}
}

void EntityHandler::highlightClosestEntity(const glm::vec2 entityCoord, const float radius)
{
	EntityData* closestEntity = findClosestEntity(entityCoord, radius);
	resolveHighlight(closestEntity, lastHighlightedEntity);
	lastHighlightedEntity = closestEntity;
}

bool EntityHandler::pickupHighlightedEntity(Modification& recorder)
{
	if (lastHighlightedEntity == nullptr)
	{
		return false;
	}
	for (int i = 0; i < entityDatas.size(); i++)
	{
		if (lastHighlightedEntity == entityDatas[i])
		{
			recorder.oldSingleEntity.push_back(*entityDatas[i]);
			return true;
		}
	}
	for (int i = 0; i < pairEntityDatas.size(); i++)
	{
		if (lastHighlightedEntity == &(pairEntityDatas[i]->e1) || lastHighlightedEntity == &(pairEntityDatas[i]->e2))
		{
			recorder.oldPairEntity.push_back(*pairEntityDatas[i]);
			return true;
		}
	}
	return false; //Somehow unable to find the highlighted entity. Possible confirmation of memory leak if this step is ran.
}

void EntityHandler::moveHighlightedEntity(int cursorX, int cursorY)
{
	if (lastHighlightedEntity == nullptr) { return; }
	if (lastHighlightedEntity->entityCoordx == cursorX && lastHighlightedEntity->entityCoordy == cursorY) { return; }
	lastHighlightedEntity->entityCoordx = cursorX;
	lastHighlightedEntity->entityCoordy = cursorY;
	update();
}

bool EntityHandler::placedownHighlightedEntity(Modification& recorder)
{
	if (lastHighlightedEntity == nullptr) { return false; }

	for (int i = 0; i < entityDatas.size(); i++)
	{
		if (lastHighlightedEntity == entityDatas[i])
		{
			if (!recorder.oldSingleEntity.empty() && !Entity::isSame(*lastHighlightedEntity, recorder.oldSingleEntity[0]))
			{
				recorder.newSingleEntity.push_back(*entityDatas[i]);
			}
			else
			{
				recorder.oldSingleEntity.clear(); //No movement was made
			}
			return true;
		}
	}
	for (int i = 0; i < pairEntityDatas.size(); i++)
	{
		if (lastHighlightedEntity == &(pairEntityDatas[i]->e1) || lastHighlightedEntity == &(pairEntityDatas[i]->e2))
		{
			if (!recorder.oldPairEntity.empty() &&
				!(Entity::isSame(*lastHighlightedEntity, recorder.oldPairEntity[0].e1) ||
				  Entity::isSame(*lastHighlightedEntity, recorder.oldPairEntity[0].e2))) //Should be *mostly* fine since the pair entity usually have different type
			{
				recorder.newPairEntity.push_back(*pairEntityDatas[i]);
			}
			else
			{
				recorder.oldPairEntity.clear(); //No movement was made
			}
			return true;
		}
	}
	return false; //Somehow unable to find the highlighted entity. Possible confirmation of memory leak if this step is ran.
}

void EntityHandler::setHighlightedEntityRotation(EntityRotation rotation, Modification& recorder)
{
	if (lastHighlightedEntity == nullptr) { return; }
	if (lastHighlightedEntity->rotation == rotation) { return; }
	for (int i = 0; i < entityDatas.size(); i++)
	{
		if (lastHighlightedEntity == entityDatas[i])
		{
			recorder.oldSingleEntity.push_back(*entityDatas[i]);
			lastHighlightedEntity->rotation = rotation;
			Entity::sanitizeImpossibleValue(*lastHighlightedEntity);
			recorder.newSingleEntity.push_back(*entityDatas[i]);
			update();
			return;
		}
	}
	for (int i = 0; i < pairEntityDatas.size(); i++)
	{
		if (lastHighlightedEntity == &(pairEntityDatas[i]->e1) || lastHighlightedEntity == &(pairEntityDatas[i]->e2))
		{
			recorder.oldPairEntity.push_back(*pairEntityDatas[i]);
			lastHighlightedEntity->rotation = rotation;
			Entity::sanitizeImpossibleValue(*lastHighlightedEntity);
			recorder.newPairEntity.push_back(*pairEntityDatas[i]);
			update();
			return;
		}
	}
	update();
}

void EntityHandler::setHighlightedEntityMode(EntityMode mode, Modification& recorder)
{
	if (lastHighlightedEntity == nullptr) { return; }
	if (lastHighlightedEntity->mode == mode) { return; }
	for (int i = 0; i < entityDatas.size(); i++)
	{
		if (lastHighlightedEntity == entityDatas[i])
		{
			recorder.oldSingleEntity.push_back(*entityDatas[i]);
			lastHighlightedEntity->mode = mode;
			Entity::sanitizeImpossibleValue(*lastHighlightedEntity);
			recorder.newSingleEntity.push_back(*entityDatas[i]);
			update();
			return;
		}
	}
	for (int i = 0; i < pairEntityDatas.size(); i++)
	{
		if (lastHighlightedEntity == &(pairEntityDatas[i]->e1) || lastHighlightedEntity == &(pairEntityDatas[i]->e2))
		{
			recorder.oldPairEntity.push_back(*pairEntityDatas[i]);
			lastHighlightedEntity->mode = mode;
			Entity::sanitizeImpossibleValue(*lastHighlightedEntity);
			recorder.newPairEntity.push_back(*pairEntityDatas[i]);
			update();
			return;
		}
	}
	update();
}

void EntityHandler::stopHighlight()
{
	resolveHighlight(nullptr, lastHighlightedEntity);
	lastHighlightedEntity = nullptr;
}

bool EntityHandler::addStaticEntity(EntityData data, Modification& recorder)
{
	EntityData* entity = new EntityData(data);
	entity->highlight = 0;
	Entity::sanitizeImpossibleValue(*entity);

	//Check if the type is allowed to be the same
	bool dupeAllowed = false;
	for (int j = 0; j < dupeAllowedList.size(); j++)
	{
		if (entity->type == dupeAllowedList[j])
		{
			dupeAllowed = true;
			break;
		}
	}

	if (!dupeAllowed)
	{
		//TODO: Extremely inefficient duplicate removal that need to be changed
		for (int i = 0; i < entityDatas.size(); i++)
		{
			if (Entity::isSame(*entity, *entityDatas[i]))
			{
				//Check if the type is allowed to be the same
				delete entity;
				return false;
			}
		}
	}

	//Mine <-> Toggle inversion, also very inefficient right now
	if (entity->type == MINE)
	{
		for (int i = 0; i < entityDatas.size(); i++)
		{
			if (entityDatas[i]->type == TOGGLE_MINE &&
				entity->entityCoordx == entityDatas[i]->entityCoordx &&
				entity->entityCoordy == entityDatas[i]->entityCoordy)
			{
				delete entity;
				recorder.oldSingleEntity.push_back(*entityDatas[i]);
				entityDatas[i]->type = MINE;
				recorder.newSingleEntity.push_back(*entityDatas[i]);
				clearStaticBuffers();
				setStaticBuffers();
				return true;
			}
		}
	}
	else if (entity->type == TOGGLE_MINE)
	{
		for (int i = 0; i < entityDatas.size(); i++)
		{
			if (entityDatas[i]->type == MINE &&
				entity->entityCoordx == entityDatas[i]->entityCoordx &&
				entity->entityCoordy == entityDatas[i]->entityCoordy)
			{
				delete entity;
				recorder.oldSingleEntity.push_back(*entityDatas[i]);
				entityDatas[i]->type = TOGGLE_MINE;
				recorder.newSingleEntity.push_back(*entityDatas[i]);
				clearStaticBuffers();
				setStaticBuffers();
				return true;
			}
		}
	}

	entityDatas.push_back(entity);
	recorder.newSingleEntity.push_back(*entity);
	clearStaticBuffers();
	setStaticBuffers();
	return true;
}

bool EntityHandler::addStaticEntity(EntityData data, EntityData pair, Modification& recorder)
{
	PairEntityData * pairEntity = new PairEntityData();
	pairEntity->e1 = EntityData(data);
	pairEntity->e2 = EntityData(pair);
	pairEntity->highlight = 0;
	pairEntity->e1.highlight = 0;
	pairEntity->e2.highlight = 0;
	Entity::sanitizeImpossibleValue(pairEntity->e1);
	Entity::sanitizeImpossibleValue(pairEntity->e2);

	//TODO: Extremely inefficient duplicate removal that need to be changed
	for (int i = 0; i < pairEntityDatas.size(); i++)
	{
		if (Entity::isSame(*pairEntity, *pairEntityDatas[i]))
		{
			delete pairEntity;
			return false;
		}
	}

	pairEntityDatas.push_back(pairEntity);
	recorder.newPairEntity.push_back(*pairEntity);
	clearStaticBuffers();
	setStaticBuffers();
	return true;
}

bool EntityHandler::deleteSingleEntity(EntityData data, Modification& recorder)
{
	clearStaticBuffers();
	for (int i = 0; i < entityDatas.size(); i++)
	{
		if (Entity::isSame(data, *entityDatas[i]))
		{
			recorder.oldSingleEntity.push_back(*entityDatas[i]);
			delete entityDatas[i];
			entityDatas.erase(entityDatas.begin() + i);
			setStaticBuffers();
			return true;
		}
	}
	setStaticBuffers(); //Nothing was deleted
	return false;
}

bool EntityHandler::deletePairEntity(PairEntityData data, Modification& recorder)
{
	clearStaticBuffers();
	for (int i = 0; i < pairEntityDatas.size(); i++)
	{
		if (Entity::isSame(data, *pairEntityDatas[i]))
		{
			recorder.oldPairEntity.push_back(*pairEntityDatas[i]);
			delete pairEntityDatas[i];
			pairEntityDatas.erase(pairEntityDatas.begin() + i);
			setStaticBuffers();
			return true;
		}
	}
	setStaticBuffers(); //Nothing was deleted
	return false;
}

void EntityHandler::undo(const Modification& changes)
{
	Modification placeholder;
	//Delete the new items
	for (int i = 0; i < changes.newSingleEntity.size(); i++)
	{
		deleteSingleEntity(changes.newSingleEntity[i], placeholder);
	}
	for (int i = 0; i < changes.newPairEntity.size(); i++)
	{
		deletePairEntity(changes.newPairEntity[i], placeholder);
	}
	//Add the old items
	for (int i = 0; i < changes.oldSingleEntity.size(); i++)
	{
		addStaticEntity(changes.oldSingleEntity[i], placeholder);
	}
	for (int i = 0; i < changes.oldPairEntity.size(); i++)
	{
		addStaticEntity(changes.oldPairEntity[i].e1, changes.oldPairEntity[i].e2, placeholder);
	}
}

void EntityHandler::redo(const Modification& changes)
{
	Modification placeholder;
	//Delete the old items
	for (int i = 0; i < changes.oldSingleEntity.size(); i++)
	{
		deleteSingleEntity(changes.oldSingleEntity[i], placeholder);
	}
	for (int i = 0; i < changes.oldPairEntity.size(); i++)
	{
		deletePairEntity(changes.oldPairEntity[i], placeholder);
	}
	//Add the new items
	for (int i = 0; i < changes.newSingleEntity.size(); i++)
	{
		addStaticEntity(changes.newSingleEntity[i], placeholder);
	}
	for (int i = 0; i < changes.newPairEntity.size(); i++)
	{
		addStaticEntity(changes.newPairEntity[i].e1, changes.newPairEntity[i].e2, placeholder);
	}
}

void EntityHandler::setHintToFollowMouse(bool toFollow)
{
	followMouse = toFollow;
}

void EntityHandler::moveHint(glm::vec2 cursorInModelSpace) //Basically equivalent to update();
{
	if (!followMouse) { return; }

	//if (volatileEntityDynamicDatas.size() == 0) { return; }

	//TODO: alternative option: use the normal entity placing coord to allow finer movement
	glm::ivec2 newPivotLocation = UtilityFunctions::clampToNearestPivotEntityCoord(cursorInModelSpace, tileSize);
	glm::ivec2 diff = newPivotLocation - pivotEntityLocation;
	if (diff.x == 0 && diff.y == 0) { return; } //No change

	for (int i = 0; i < volatileEntityDynamicDatas.size(); i++)
	{
		volatileEntityDynamicDatas[i]->entityCoordx += diff.x;
		volatileEntityDynamicDatas[i]->entityCoordy += diff.y;
	}

	for (int i = 0; i < volatilePairEntityDatas.size(); i++)
	{
		volatilePairEntityDatas[i]->e1.entityCoordx += diff.x;
		volatilePairEntityDatas[i]->e1.entityCoordy += diff.y;
		volatilePairEntityDatas[i]->e2.entityCoordx += diff.x;
		volatilePairEntityDatas[i]->e2.entityCoordy += diff.y;
	}

	selectRegionMinBoundary = selectRegionMinBoundary + diff;
	selectRegionMaxBoundary = selectRegionMaxBoundary + diff;

	pivotEntityLocation = newPivotLocation;
	setVolatileBuffers(); //TODO: use buffer sub data to speed this part up
}

void EntityHandler::setSelectedRegion(glm::vec2 corner1, glm::vec2 corner2, bool isInclusive)
{
	glm::ivec2 corner1TileCoord = UtilityFunctions::clampToNearestTileCoord(corner1, tileSize);
	glm::ivec2 corner2TileCoord = UtilityFunctions::clampToNearestTileCoord(corner2, tileSize);
	int minX = std::min(corner1TileCoord.x, corner2TileCoord.x) * 4 + 4;
	int maxX = std::max(corner1TileCoord.x, corner2TileCoord.x) * 4 + 8;
	int minY = std::min(corner1TileCoord.y, corner2TileCoord.y) * 4 + 4;
	int maxY = std::max(corner1TileCoord.y, corner2TileCoord.y) * 4 + 8;

	if (!isInclusive)
	{
		minX++;
		maxX--;
		minY++;
		maxY--;
	}

	selectRegionMinBoundary = glm::ivec2(minX, minY);
	selectRegionMaxBoundary = glm::ivec2(maxX, maxY);
	
	pivotEntityLocation = UtilityFunctions::clampToNearestPivotEntityCoord(corner2, tileSize);
}

void EntityHandler::stageSelected()
{
	if (volatileEntityStaticData.size() > 0 || volatileEntityDynamicDatas.size() > 0)
	{
		unstageSelected();
	}

	int minX = selectRegionMinBoundary.x;
	int minY = selectRegionMinBoundary.y;
	int maxX = selectRegionMaxBoundary.x;
	int maxY = selectRegionMaxBoundary.y;

	for (int i = 0; i < pairEntityDatas.size(); i++)
	{
		if (!(pairEntityDatas[i]->e1.entityCoordx < minX || pairEntityDatas[i]->e1.entityCoordx > maxX ||
			  pairEntityDatas[i]->e1.entityCoordy < minY || pairEntityDatas[i]->e1.entityCoordy > maxY) &&
			!(pairEntityDatas[i]->e2.entityCoordx < minX || pairEntityDatas[i]->e2.entityCoordx > maxX ||
			  pairEntityDatas[i]->e2.entityCoordy < minY || pairEntityDatas[i]->e2.entityCoordy > maxY))
		{
			/* TODO: This is essentially just add entity but on a different vector */
			PairEntityData* pairEntity = new PairEntityData();
			pairEntity->e1 = EntityData(pairEntityDatas[i]->e1);
			pairEntity->e2 = EntityData(pairEntityDatas[i]->e2);
			pairEntity->e1.highlight = 1; //Staged entities are highlighted
			pairEntity->e2.highlight = 1;
			pairEntity->highlight = 1;
			Entity::sanitizeImpossibleValue(pairEntity->e1);
			Entity::sanitizeImpossibleValue(pairEntity->e2);

			volatilePairEntityDatas.push_back(pairEntity);
		}
	}

	for (int i = 0; i < entityDatas.size(); i++)
	{
		if (!(entityDatas[i]->entityCoordx < minX || entityDatas[i]->entityCoordx > maxX ||
			  entityDatas[i]->entityCoordy < minY || entityDatas[i]->entityCoordy > maxY))
		{
			/* TODO: Same here, same as adding a new entity but on the volatile vector */
			EntityData * e = new EntityData(*entityDatas[i]);
			e->highlight = 1;
			Entity::sanitizeImpossibleValue(*e);
			volatileEntityDynamicDatas.push_back(e);
		}
	}

	setVolatileBuffers();
}

void EntityHandler::unstageSelected()
{
	clearVolatileBuffers();
}

void EntityHandler::pasteSelected(Modification& recorder)
{
	//Add all the pair entity utilize the connector data first
	for (int i = 0; i < volatilePairEntityDatas.size(); i++)
	{
		addStaticEntity(volatilePairEntityDatas[i]->e1, volatilePairEntityDatas[i]->e2, recorder);
	}

	for (int i = 0; i < volatileEntityDynamicDatas.size(); i++)
	{
		addStaticEntity(*volatileEntityDynamicDatas[i], recorder);
	}

	for (int i = 0; i < volatileEntityStaticData.size(); i++)
	{
		addStaticEntity(*volatileEntityStaticData[i], recorder);
	}
}

void EntityHandler::deleteSelected(Modification& recorder)
{
	int minX = selectRegionMinBoundary.x;
	int maxX = selectRegionMaxBoundary.x;
	int minY = selectRegionMinBoundary.y;
	int maxY = selectRegionMaxBoundary.y;

	std::vector<PairEntityData*>::iterator pairItr = pairEntityDatas.begin();
	while(pairItr != pairEntityDatas.end())
	{
		if (!((*pairItr)->e1.entityCoordx < minX || (*pairItr)->e1.entityCoordx > maxX ||
			  (*pairItr)->e1.entityCoordy < minY || (*pairItr)->e1.entityCoordy > maxY) &&
			!((*pairItr)->e2.entityCoordx < minX || (*pairItr)->e2.entityCoordx > maxX ||
			  (*pairItr)->e2.entityCoordy < minY || (*pairItr)->e2.entityCoordy > maxY))
		{
			recorder.oldPairEntity.push_back(**pairItr);
			delete *pairItr;
			pairItr = pairEntityDatas.erase(pairItr);
		}
		else
		{
			pairItr++;
		}
	}

	std::vector<EntityData*>::iterator entityItr = entityDatas.begin();
	while (entityItr != entityDatas.end())
	{
		if (!((*entityItr)->entityCoordx < minX || (*entityItr)->entityCoordx > maxX ||
			  (*entityItr)->entityCoordy < minY || (*entityItr)->entityCoordy > maxY))
		{
			recorder.oldSingleEntity.push_back(**entityItr);
			delete *entityItr;
			entityItr = entityDatas.erase(entityItr);
		}
		else
		{
			entityItr++;
		}
	}

	setStaticBuffers();
}

void EntityHandler::copySelected()
{
	stageSelected();
}

void EntityHandler::cutSelected(Modification& recorder)
{
	stageSelected();
	deleteSelected(recorder);
}

void EntityHandler::flipSelectedHorizontally()
{
	for (int i = 0; i < volatileEntityDynamicDatas.size(); i++)
	{
		flipEntityHorizontally(*volatileEntityDynamicDatas[i], selectRegionMinBoundary.x, selectRegionMaxBoundary.x);
	}
	for (int i = 0; i < volatilePairEntityDatas.size(); i++)
	{
		flipEntityHorizontally(volatilePairEntityDatas[i]->e1, selectRegionMinBoundary.x, selectRegionMaxBoundary.x);
		flipEntityHorizontally(volatilePairEntityDatas[i]->e2, selectRegionMinBoundary.x, selectRegionMaxBoundary.x);
	}
	setVolatileBuffers();
}

void EntityHandler::flipSelectedVertically()
{
	for (int i = 0; i < volatileEntityDynamicDatas.size(); i++)
	{
		flipEntityVertically(*volatileEntityDynamicDatas[i], selectRegionMinBoundary.y, selectRegionMaxBoundary.y);
	}
	for (int i = 0; i < volatilePairEntityDatas.size(); i++)
	{
		flipEntityVertically(volatilePairEntityDatas[i]->e1, selectRegionMinBoundary.y, selectRegionMaxBoundary.y);
		flipEntityVertically(volatilePairEntityDatas[i]->e2, selectRegionMinBoundary.y, selectRegionMaxBoundary.y);
	}
	setVolatileBuffers();
}

void EntityHandler::rotateSelectedClockwise()
{
	for (int i = 0; i < volatileEntityDynamicDatas.size(); i++)
	{
		rotateEntityClockwise(*volatileEntityDynamicDatas[i], pivotEntityLocation);
	}
	for (int i = 0; i < volatilePairEntityDatas.size(); i++)
	{
		rotateEntityClockwise(volatilePairEntityDatas[i]->e1, pivotEntityLocation);
		rotateEntityClockwise(volatilePairEntityDatas[i]->e2, pivotEntityLocation);
	}

	//The two corner of the selected region also need to be rotated
	int dx = selectRegionMinBoundary.x - pivotEntityLocation.x;
	int dy = selectRegionMinBoundary.y - pivotEntityLocation.y;
	selectRegionMinBoundary.x = pivotEntityLocation.x + dy;
	selectRegionMinBoundary.y = pivotEntityLocation.y - dx;

	dx = selectRegionMaxBoundary.x - pivotEntityLocation.x;
	dy = selectRegionMaxBoundary.y - pivotEntityLocation.y;
	selectRegionMaxBoundary.x = pivotEntityLocation.x + dy;
	selectRegionMaxBoundary.y = pivotEntityLocation.y - dx;

	int minX = std::min(selectRegionMinBoundary.x, selectRegionMaxBoundary.x);
	int maxX = std::max(selectRegionMinBoundary.x, selectRegionMaxBoundary.x);
	int minY = std::min(selectRegionMinBoundary.y, selectRegionMaxBoundary.y);
	int maxY = std::max(selectRegionMinBoundary.y, selectRegionMaxBoundary.y);

	selectRegionMinBoundary = glm::ivec2(minX, minY);
	selectRegionMaxBoundary = glm::ivec2(maxX, maxY);

	setVolatileBuffers();
}

void EntityHandler::rotateSelectedCounterClockwise()
{
	for (int i = 0; i < volatileEntityDynamicDatas.size(); i++)
	{
		rotateEntityCounterClockwise(*volatileEntityDynamicDatas[i], pivotEntityLocation);
	}
	for (int i = 0; i < volatilePairEntityDatas.size(); i++)
	{
		rotateEntityCounterClockwise(volatilePairEntityDatas[i]->e1, pivotEntityLocation);
		rotateEntityCounterClockwise(volatilePairEntityDatas[i]->e2, pivotEntityLocation);
	}

	//The two corner of the selected region also need to be rotated
	int dx = selectRegionMinBoundary.x - pivotEntityLocation.x;
	int dy = selectRegionMinBoundary.y - pivotEntityLocation.y;
	selectRegionMinBoundary.x = pivotEntityLocation.x - dy;
	selectRegionMinBoundary.y = pivotEntityLocation.y + dx;

	dx = selectRegionMaxBoundary.x - pivotEntityLocation.x;
	dy = selectRegionMaxBoundary.y - pivotEntityLocation.y;
	selectRegionMaxBoundary.x = pivotEntityLocation.x - dy;
	selectRegionMaxBoundary.y = pivotEntityLocation.y + dx;

	int minX = std::min(selectRegionMinBoundary.x, selectRegionMaxBoundary.x);
	int maxX = std::max(selectRegionMinBoundary.x, selectRegionMaxBoundary.x);
	int minY = std::min(selectRegionMinBoundary.y, selectRegionMaxBoundary.y);
	int maxY = std::max(selectRegionMinBoundary.y, selectRegionMaxBoundary.y);

	selectRegionMinBoundary = glm::ivec2(minX, minY);
	selectRegionMaxBoundary = glm::ivec2(maxX, maxY);

	setVolatileBuffers();
}

/*
void EntityHandler::sortEntityListByType()
{
	std::sort(entityDatas.begin(), entityDatas.end(), [](EntityData* e1, EntityData* e2) {
		return (int)(e1->type) < (int)(e2->type);
	});
}
*/

const std::vector<EntityData*>& EntityHandler::getEntityData() const
{
	return entityDatas;
}

const std::vector<PairEntityData*>& EntityHandler::getPairEntityData() const
{
	return pairEntityDatas;
}

///////////////////////////////////////////////////////////////////////

void EntityHandler::update()
{
	clearStaticBuffers();
	setStaticBuffers();
}

void EntityHandler::draw(glm::mat4 viewProjMtx)
{
	drawEntities(viewProjMtx, entityDataBuffer, (GLsizei)(entityDatas.size() + pairEntityDatas.size() * 2));
	drawConnectors(viewProjMtx, pairEntityDataBuffer, (GLsizei)pairEntityDatas.size());
	drawEntities(viewProjMtx, volatileEntityDataBuffer,
				(GLsizei)(volatileEntityStaticData.size() + volatileEntityDynamicDatas.size() + volatilePairEntityDatas.size() * 2));
	drawConnectors(viewProjMtx, volatileEntityConnectorBuffer, (GLsizei)volatilePairEntityDatas.size());
}

// TODO: Fix the attribute location after restructure
void EntityHandler::drawEntities(glm::mat4 viewProjMtx, const uint entityBuffer, GLsizei entityBufferSize)
{
	if (allowModify)
	{
		fprintf(stderr, "Drawing may not be proceed as buffer is not set. Ending draw function \n");
		return;
	}

	// Set up shader
	glDisable(GL_CULL_FACE);
	glUseProgram(entityShader->getProgramID());

	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, entitySpriteSheet->getTextureID());
	glBindTexture(GL_TEXTURE_2D, palette->getEntityTextureID());

	glUniformMatrix4fv(glGetUniformLocation(entityShader->getProgramID(), "viewProjMtx"), 1, false, (float*)&viewProjMtx);

	// Set up state
	glBindBuffer(GL_ARRAY_BUFFER, entityBuffer);

	uint entityTypeLoc = 0;
	glEnableVertexAttribArray(entityTypeLoc);
	glVertexAttribIPointer(entityTypeLoc, 1, GL_INT, sizeof(EntityData), (void*)(0 * sizeof(int)));

	uint entityCoordLoc = 1;
	glEnableVertexAttribArray(entityCoordLoc);
	glVertexAttribIPointer(entityCoordLoc, 2, GL_INT, sizeof(EntityData), (void*)(1 * sizeof(int)));

	uint entityRotationLoc = 2;
	glEnableVertexAttribArray(entityRotationLoc);
	glVertexAttribIPointer(entityRotationLoc, 1, GL_INT, sizeof(EntityData), (void*)(3 * sizeof(int)));

	uint entityModeLoc = 3;
	glEnableVertexAttribArray(entityModeLoc);
	glVertexAttribIPointer(entityModeLoc, 1, GL_INT, sizeof(EntityData), (void*)(4 * sizeof(int)));

	uint entityColorLoc = 4;
	glEnableVertexAttribArray(entityColorLoc);
	glVertexAttribPointer(entityColorLoc, 3, GL_FLOAT, GL_FALSE, sizeof(EntityData), (void*)(5 * sizeof(int)));

	uint entityHighlightLoc = 5;
	glEnableVertexAttribArray(entityHighlightLoc);
	glVertexAttribIPointer(entityHighlightLoc, 1, GL_INT, sizeof(EntityData), (void*)(5 * sizeof(int) + 3 * sizeof(float)));

	glDrawArrays(GL_POINTS, 0, entityBufferSize); //Use GL_POINTS because every "point" is 1 EntityData.

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLenum error = 0;
	if ((error = glGetError()) != GL_NO_ERROR)
	{
		printf("%i \n", (int)error);
	}

	glEnable(GL_CULL_FACE);

	glUseProgram(0);
}

// TODO: Fix the attribute location after restructure
void EntityHandler::drawConnectors(glm::mat4 viewProjMtx, const uint connectorBuffer, GLsizei connectorBufferSize)
{
	if (allowModify)
	{
		fprintf(stderr, "Drawing may not be proceed as buffer is not set. Ending draw function \n");
		return;
	}

	// Set up shader
	glDisable(GL_CULL_FACE);
	glUseProgram(connectorShader->getProgramID());

	glUniformMatrix4fv(glGetUniformLocation(connectorShader->getProgramID(), "viewProjMtx"), 1, false, (float*)&viewProjMtx);

	// Set up state
	glBindBuffer(GL_ARRAY_BUFFER, connectorBuffer);

	uint entity1CoordLoc = 0;
	glEnableVertexAttribArray(entity1CoordLoc);
	glVertexAttribIPointer(entity1CoordLoc, 2, GL_INT, sizeof(ConnectorShaderInfo), (void*)(0 * sizeof(int)));

	uint entity2CoordLoc = 1;
	glEnableVertexAttribArray(entity2CoordLoc);
	glVertexAttribIPointer(entity2CoordLoc, 2, GL_INT, sizeof(ConnectorShaderInfo), (void*)(2 * sizeof(int)));

	uint entityHighlightLoc = 2;
	glEnableVertexAttribArray(entityHighlightLoc);
	glVertexAttribIPointer(entityHighlightLoc, 1, GL_INT, sizeof(ConnectorShaderInfo), (void*)(4 * sizeof(int)));

	glDrawArrays(GL_POINTS, 0, connectorBufferSize); //Use GL_POINTS because every "point" is 1 EntityData.

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLenum error = 0;
	if ((error = glGetError()) != GL_NO_ERROR)
	{
		printf("%i \n", (int)error);
	}

	glEnable(GL_CULL_FACE);

	glUseProgram(0);
}

///////////////////////////////////////////////////////////////////////

void EntityHandler::deleteEntityByAddress(EntityData* p, Modification& recorder)
{
	clearStaticBuffers();
	for (int i = 0; i < entityDatas.size(); i++)
	{
		if (p == entityDatas[i])
		{
			recorder.oldSingleEntity.push_back(*entityDatas[i]);
			delete entityDatas[i];
			entityDatas.erase(entityDatas.begin() + i);
			setStaticBuffers();
			return;
		}
	}
	for (int i = 0; i < pairEntityDatas.size(); i++)
	{
		if (p == &(pairEntityDatas[i]->e1) || p == &(pairEntityDatas[i]->e2))
		{
			recorder.oldPairEntity.push_back(*pairEntityDatas[i]);
			delete pairEntityDatas[i];
			pairEntityDatas.erase(pairEntityDatas.begin() + i);
			setStaticBuffers();
			return;
		}
	}
	setStaticBuffers(); //Nothing was deleted
}

//May return a pointer to nullptr to indicate no closest
EntityData* EntityHandler::findClosestEntity(const glm::vec2 entityCoord, const float radius)
{
	EntityData* returnptr = nullptr;
	float closest = radius * radius;
	bool found = false;
	for (int i = 0; i < entityDatas.size(); i++)
	{
		float xDistance = entityDatas[i]->entityCoordx - entityCoord.x;
		float yDistance = entityDatas[i]->entityCoordy - entityCoord.y;
		float distance = xDistance * xDistance + yDistance * yDistance;
		if (distance < closest)
		{
			returnptr = entityDatas[i];
			if (distance < 0.0001f)
			{
				found = true;
				break;
			}
			else
			{
				closest = distance;
			}
		}
	}

	if (found) { return returnptr; }

	//Add searching for pair data as well and compare which one is closest
	for (int i = 0; i < pairEntityDatas.size(); i++)
	{
		float e1xDistance = pairEntityDatas[i]->e1.entityCoordx - entityCoord.x;
		float e1yDistance = pairEntityDatas[i]->e1.entityCoordy - entityCoord.y;
		float distance = e1xDistance * e1xDistance + e1yDistance * e1yDistance;
		if (distance < closest)
		{
			returnptr = &pairEntityDatas[i]->e1;
			if (distance < 0.0001f)
			{
				break;
			}
			else
			{
				closest = distance;
			}
		}

		float e2xDistance = pairEntityDatas[i]->e2.entityCoordx - entityCoord.x;
		float e2yDistance = pairEntityDatas[i]->e2.entityCoordy - entityCoord.y;
		distance = e2xDistance * e2xDistance + e2yDistance * e2yDistance;
		if (distance < closest)
		{
			returnptr = &pairEntityDatas[i]->e2;
			if (distance < 0.0001f)
			{
				break;
			}
			else
			{
				closest = distance;
			}
		}
	}

	return returnptr;
}

//May pass nullptr to act as no-action
void EntityHandler::resolveHighlight(EntityData* entityToHighlight, EntityData* entityToUnHighlight)
{
	bool needUpdate = false;
	if (entityToUnHighlight != nullptr)
	{
		switch (entityToUnHighlight->type)
		{
			//Search in pair entities
			case EXIT: case EXIT_SWITCH: case LOCKED_DOOR: case LOCKED_DOOR_SWITCH: case TRAP_DOOR: case TRAP_DOOR_SWITCH:
			{
				for (int i = 0; i < pairEntityDatas.size(); i++)
				{
					if (entityToUnHighlight == &(pairEntityDatas[i]->e1) || entityToUnHighlight == &(pairEntityDatas[i]->e2))
					{
						//If the entity to highlight is the same as the one about to be unhighlighted (this can include its pair), then no highlighting change is needed.
						if (entityToHighlight == &(pairEntityDatas[i]->e1) || entityToHighlight == &(pairEntityDatas[i]->e2)) { return; }
						pairEntityDatas[i]->e1.highlight = 0;
						pairEntityDatas[i]->e2.highlight = 0;
						pairEntityDatas[i]->highlight = 0;
						break;
					}
				}
				break;
			}
			default: //Search in single entities
			{
				//If the entity to highlight is the same as the one about to be unhighlighted, then no highlighting change is needed.
				if (entityToUnHighlight == entityToHighlight) { return; }
				for (int i = 0; i < entityDatas.size(); i++)
				{
					if (entityToUnHighlight == entityDatas[i])
					{
						entityDatas[i]->highlight = 0;
						break;
					}
				}
				break;
			}
		}
		needUpdate = true;
	}

	if (entityToHighlight != nullptr)
	{
		switch (entityToHighlight->type)
		{
			//Search in pair entities
			case EXIT: case EXIT_SWITCH: case LOCKED_DOOR: case LOCKED_DOOR_SWITCH: case TRAP_DOOR: case TRAP_DOOR_SWITCH:
			{
				for (int i = 0; i < pairEntityDatas.size(); i++)
				{
					if (entityToHighlight == &(pairEntityDatas[i]->e1) || entityToHighlight == &(pairEntityDatas[i]->e2))
					{
						pairEntityDatas[i]->e1.highlight = 1;
						pairEntityDatas[i]->e2.highlight = 1;
						pairEntityDatas[i]->highlight = 1;
						break;
					}
				}
				break;
			}
			default: //Search in single entities
			{
				for (int i = 0; i < entityDatas.size(); i++)
				{
					if (entityToHighlight == entityDatas[i])
					{
						entityDatas[i]->highlight = 1;
						break;
					}
				}
				break;
			}
		}
		needUpdate = true;
	}

	if (needUpdate) { this->update(); }
}

void EntityHandler::setStaticBuffers()
{
	if (entityDatas.size() > 0 || pairEntityDatas.size() > 0)
	{
		std::vector<EntityData> dataBuffer1;
		dataBuffer1.reserve(entityDatas.size() + pairEntityDatas.size() * 2);
		for (int i = 0; i < entityDatas.size(); i++)
		{
			dataBuffer1.push_back(*entityDatas[i]);
		}
		for (int i = 0; i < pairEntityDatas.size(); i++)
		{
			dataBuffer1.push_back(pairEntityDatas[i]->e1);
			dataBuffer1.push_back(pairEntityDatas[i]->e2);
		}

		// Store vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, entityDataBuffer);
		glBufferData(GL_ARRAY_BUFFER, dataBuffer1.size() * sizeof(EntityData), &dataBuffer1[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (pairEntityDatas.size() > 0)
	{
		std::vector<ConnectorShaderInfo> dataBuffer2;
		dataBuffer2.reserve(pairEntityDatas.size());
		for (int i = 0; i < pairEntityDatas.size(); i++)
		{
			ConnectorShaderInfo temp;
			temp.entity1Coord = glm::ivec2(pairEntityDatas[i]->e1.entityCoordx, pairEntityDatas[i]->e1.entityCoordy);
			temp.entity2Coord = glm::ivec2(pairEntityDatas[i]->e2.entityCoordx, pairEntityDatas[i]->e2.entityCoordy);
			temp.highlight = pairEntityDatas[i]->highlight;
			dataBuffer2.push_back(temp);
		}

		glBindBuffer(GL_ARRAY_BUFFER, pairEntityDataBuffer);
		glBufferData(GL_ARRAY_BUFFER, dataBuffer2.size() * sizeof(ConnectorShaderInfo), &dataBuffer2[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	allowModify = false;
}

void EntityHandler::clearStaticBuffers()
{
	allowModify = true; //What this really do is stopping the draw routine so u can modify the buffer
}

void EntityHandler::setVolatileBuffers()
{
	if (volatileEntityStaticData.size() > 0 || volatileEntityDynamicDatas.size() > 0 || volatilePairEntityDatas.size() > 0)
	{
		std::vector<EntityData> dataBuffer1;
		dataBuffer1.reserve(volatileEntityStaticData.size() + volatileEntityDynamicDatas.size() + volatilePairEntityDatas.size() * 2);
		for (int i = 0; i < volatileEntityStaticData.size(); i++)
		{
			dataBuffer1.push_back(*volatileEntityStaticData[i]);
		}
		for (int i = 0; i < volatileEntityDynamicDatas.size(); i++)
		{
			dataBuffer1.push_back(*volatileEntityDynamicDatas[i]);
		}
		for (int i = 0; i < volatilePairEntityDatas.size(); i++)
		{
			dataBuffer1.push_back(volatilePairEntityDatas[i]->e1);
			dataBuffer1.push_back(volatilePairEntityDatas[i]->e2);
		}

		// Store vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, volatileEntityDataBuffer);
		glBufferData(GL_ARRAY_BUFFER, dataBuffer1.size() * sizeof(EntityData), &dataBuffer1[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (volatilePairEntityDatas.size() > 0)
	{
		std::vector<ConnectorShaderInfo> dataBuffer2;
		dataBuffer2.reserve(volatilePairEntityDatas.size());
		for (int i = 0; i < volatilePairEntityDatas.size(); i++)
		{
			ConnectorShaderInfo temp;
			temp.entity1Coord = glm::ivec2(volatilePairEntityDatas[i]->e1.entityCoordx, volatilePairEntityDatas[i]->e1.entityCoordy);
			temp.entity2Coord = glm::ivec2(volatilePairEntityDatas[i]->e2.entityCoordx, volatilePairEntityDatas[i]->e2.entityCoordy);
			temp.highlight = volatilePairEntityDatas[i]->highlight;
			dataBuffer2.push_back(temp);
		}

		glBindBuffer(GL_ARRAY_BUFFER, volatileEntityConnectorBuffer);
		glBufferData(GL_ARRAY_BUFFER, dataBuffer2.size() * sizeof(ConnectorShaderInfo), &dataBuffer2[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void EntityHandler::clearVolatileBuffers()
{
	//Unlike static buffer, calling clear means actually clearing all drawing data.
	glFinish();

	for (int i = 0; i < volatileEntityStaticData.size(); i++)
	{
		delete volatileEntityStaticData[i];
	}

	for (int i = 0; i < volatileEntityDynamicDatas.size(); i++)
	{
		delete volatileEntityDynamicDatas[i];
	}

	for (int i = 0; i < volatilePairEntityDatas.size(); i++)
	{
		delete volatilePairEntityDatas[i];
	}

	volatileEntityStaticData.clear();
	volatileEntityDynamicDatas.clear();
	volatilePairEntityDatas.clear();
}

void EntityHandler::flipEntityHorizontally(EntityData & entity, int xMinBoundary, int xMaxBoundary)
{
	entity.entityCoordx = xMaxBoundary - (entity.entityCoordx - xMinBoundary);

	switch (entity.type)
	{
		//8-rotation style type: 0 <-> 4, 1 <-> 3, 5 <-> 7
		case LOCKED_DOOR: case TRAP_DOOR: case BOUNCE_PAD: case ONE_WAY: case LASER_TURRET: case BOOST_PAD:
		{
			int baseRotation = entity.rotation / 4;
			int remainder = entity.rotation % 4;
			entity.rotation = (baseRotation * 4 + 4 - remainder) % 8;
			break;
		}
		//4-rotation style type: 0 <-> 4
		case CHAINGUN: case LASER_DRONE: case REGULAR_DRONE: case CHASE_DRONE: case MICRO_DRONE: case THWUMP:
		{
			int baseRotation = entity.rotation / 4;
			int remainder = entity.rotation % 4;
			entity.rotation = (baseRotation * 4 + 4 - remainder) % 8;
			break;
		}
		default:
		{
			break;
		}
	}

	//TODO: Perform sanity check
	Entity::sanitizeImpossibleValue(entity);
}

void EntityHandler::flipEntityVertically(EntityData & entity, int yMinBoundary, int yMaxBoundary)
{
	entity.entityCoordy = yMaxBoundary - (entity.entityCoordy - yMinBoundary);

	switch (entity.type)
	{
		//8-rotation style type: 1 <-> 7, 2 <-> 6, 3 <-> 5
		case LOCKED_DOOR: case TRAP_DOOR: case BOUNCE_PAD: case ONE_WAY: case LASER_TURRET: case BOOST_PAD:
		{
			entity.rotation = (8 - entity.rotation) % 8;
			break;
		}
		//4-rotation style type: 2 <-> 6
		case CHAINGUN: case LASER_DRONE: case REGULAR_DRONE: case CHASE_DRONE: case MICRO_DRONE: case THWUMP:
		{
			entity.rotation = (8 - entity.rotation) % 8;
			break;
		}
		default:
		{
			break;
		}
	}

	Entity::sanitizeImpossibleValue(entity);
}

void EntityHandler::rotateEntityClockwise(EntityData & entity, const glm::ivec2 & pivot)
{
	int dx = entity.entityCoordx - pivot.x;
	int dy = entity.entityCoordy - pivot.y;

	entity.entityCoordx = pivot.x + dy;
	entity.entityCoordy = pivot.y - dx;
	entity.rotation = (entity.rotation + 8 + 2) % 8;

	Entity::sanitizeImpossibleValue(entity);
}

void EntityHandler::rotateEntityCounterClockwise(EntityData & entity, const glm::ivec2 & pivot)
{
	int dx = entity.entityCoordx - pivot.x;
	int dy = entity.entityCoordy - pivot.y;

	entity.entityCoordx = pivot.x - dy;
	entity.entityCoordy = pivot.y + dx;
	entity.rotation = (entity.rotation + 8 - 2) % 8;

	Entity::sanitizeImpossibleValue(entity);
}

/*
bool EntityHandler::sortByType(EntityData * e1, EntityData * e2)
{
	return (int)(e1->type) < (int)(e2->type);
}
*/
