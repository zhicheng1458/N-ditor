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
	glGenBuffers(1, &entityConnectorBuffer);
	glGenBuffers(1, &volatileEntityDataBuffer);
	glGenBuffers(1, &volatileEntityConnectorBuffer);

	/*
	for (int i = 0; i < 29; i++)
	{
		EntityData testData;
		testData.entityCoordx = (i+1)*4+6;
		testData.entityCoordy = 10+i;
		testData.type = i;
		testData.rotation = i % 8;
		testData.color = glm::vec3(1.0f);
		testData.highlight = 0;
		testData.pair = nullptr;

		this->addStaticEntity(testData);
	}

	EntityConnector exitConnector;
	EntityConnector lockedDoorConnector;
	EntityConnector closedDoorConnector;
	exitConnector.e1 = entityDatas[3];
	exitConnector.e2 = entityDatas[4];
	exitConnector.highlight = 0;
	entityDatas[3]->pair = entityDatas[4];
	entityDatas[4]->pair = entityDatas[3];
	lockedDoorConnector.e1 = entityDatas[6];
	lockedDoorConnector.e2 = entityDatas[7];
	lockedDoorConnector.highlight = 0;
	entityDatas[6]->pair = entityDatas[7];
	entityDatas[7]->pair = entityDatas[6];
	closedDoorConnector.e1 = entityDatas[8];
	closedDoorConnector.e2 = entityDatas[9];
	closedDoorConnector.highlight = 0;
	entityDatas[8]->pair = entityDatas[9];
	entityDatas[9]->pair = entityDatas[8];
	entityConnections.push_back(exitConnector);
	entityConnections.push_back(lockedDoorConnector);
	entityConnections.push_back(closedDoorConnector);
	*/

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
	clearVolatileBuffers();

	glDeleteBuffers(1, &entityDataBuffer);
	glDeleteBuffers(1, &entityConnectorBuffer);
	glDeleteBuffers(1, &volatileEntityDataBuffer);
	glDeleteBuffers(1, &volatileEntityConnectorBuffer);
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

void EntityHandler::deleteClosestEntity(const glm::vec2 entityCoord, const float radius)
{
	EntityData* closestEntity = findClosestEntity(entityCoord, radius);
	if (lastHighlightedEntity == closestEntity)
	{
		stopHighlight();
		deleteEntity(closestEntity);
		closestEntity = nullptr;
		highlightClosestEntity(entityCoord, radius);
	}
	else
	{
		deleteEntity(closestEntity);
		closestEntity = nullptr;
	}
}

void EntityHandler::highlightClosestEntity(const glm::vec2 entityCoord, const float radius)
{
	EntityData* closestEntity = findClosestEntity(entityCoord, radius);
	resolveHighlight(closestEntity, lastHighlightedEntity);
	lastHighlightedEntity = closestEntity;
}

void EntityHandler::moveHighlightedEntity(int cursorX, int cursorY)
{
	if (lastHighlightedEntity == nullptr) { return; };
	if (lastHighlightedEntity->entityCoordx == cursorX && lastHighlightedEntity->entityCoordy == cursorY) { return; }
	lastHighlightedEntity->entityCoordx = cursorX;
	lastHighlightedEntity->entityCoordy = cursorY;
	update();
}

void EntityHandler::setHighlightedEntityRotation(EntityRotation rotation)
{
	if (lastHighlightedEntity == nullptr) { return; };
	if (lastHighlightedEntity->rotation == rotation) { return; }
	lastHighlightedEntity->rotation = rotation;
	//TODO: Perform sanity check
	Entity::sanitizeImpossibleValue(lastHighlightedEntity);
	update();
}

void EntityHandler::stopHighlight()
{
	resolveHighlight(nullptr, lastHighlightedEntity);
	lastHighlightedEntity = nullptr;
}

bool EntityHandler::isSame(EntityData data1, EntityData data2)
{
	return false;
}

bool EntityHandler::isLessThan(EntityData data1, EntityData data2)
{
	return false;
}

void EntityHandler::addStaticEntity(EntityData data)
{
	//TODO: Perform sanity check

	clearStaticBuffers();
	EntityData* entity = new EntityData(data);
	entity->highlight = 0;
	Entity::sanitizeImpossibleValue(entity);
	entityDatas.push_back(entity);
	setStaticBuffers();
}

void EntityHandler::addStaticEntity(EntityData data, EntityData pair)
{
	//TODO: Perform sanity check

	clearStaticBuffers();
	EntityData* entity = new EntityData(data);
	EntityData* pairEntity = new EntityData(pair);
	entity->highlight = 0;
	pairEntity->highlight = 0;
	entity->pair = pairEntity;
	pairEntity->pair = entity;
	Entity::sanitizeImpossibleValue(entity);
	Entity::sanitizeImpossibleValue(pairEntity);
	entityDatas.push_back(entity);
	entityDatas.push_back(pairEntity);
	EntityConnector connector;
	connector.e1 = entity;
	connector.e2 = pairEntity;
	connector.highlight = 0;
	entityConnections.push_back(connector);
	setStaticBuffers();
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

	for (int i = 0; i < entityConnections.size(); i++)
	{
		if (!(entityConnections[i].e1->entityCoordx < minX || entityConnections[i].e1->entityCoordx > maxX ||
			  entityConnections[i].e1->entityCoordy < minY || entityConnections[i].e1->entityCoordy > maxY) &&
			!(entityConnections[i].e2->entityCoordx < minX || entityConnections[i].e2->entityCoordx > maxX ||
			  entityConnections[i].e2->entityCoordy < minY || entityConnections[i].e2->entityCoordy > maxY))
		{
			/* TODO: This is essentially just add entity but on a different vector */
			EntityData* e1 = new EntityData(*(entityConnections[i].e1));
			EntityData* e2 = new EntityData(*(entityConnections[i].e2));
			e1->highlight = 1; //Staged entities are highlighted
			e2->highlight = 1;
			e1->pair = e2;
			e2->pair = e1;
			Entity::sanitizeImpossibleValue(e1);
			Entity::sanitizeImpossibleValue(e2);
			volatileEntityDynamicDatas.push_back(e1);
			volatileEntityDynamicDatas.push_back(e2);
			EntityConnector connector;
			connector.e1 = e1;
			connector.e2 = e2;
			volatileEntityConnections.push_back(connector);
		}
	}

	for (int i = 0; i < entityDatas.size(); i++)
	{
		if (entityDatas[i]->pair == nullptr)
		{
			if (!(entityDatas[i]->entityCoordx < minX || entityDatas[i]->entityCoordx > maxX ||
				  entityDatas[i]->entityCoordy < minY || entityDatas[i]->entityCoordy > maxY))
			{
				/* TODO: Same here, same as adding a new entity but on the volatile vector */
				EntityData * e = new EntityData(*entityDatas[i]);
				e->highlight = 1;
				Entity::sanitizeImpossibleValue(e);
				volatileEntityDynamicDatas.push_back(e);
			}
		}
	}

	setVolatileBuffers();
}

void EntityHandler::unstageSelected()
{
	clearVolatileBuffers();
}

void EntityHandler::pasteSelected()
{
	//Add all the pair entity utilize the connector data first
	for (int i = 0; i < volatileEntityConnections.size(); i++)
	{
		addStaticEntity(*volatileEntityConnections[i].e1, *volatileEntityConnections[i].e2);
	}

	//If an entity has a pair, that means it has been added already.
	for (int i = 0; i < volatileEntityDynamicDatas.size(); i++)
	{
		if (volatileEntityDynamicDatas[i]->pair == nullptr)
		{
			addStaticEntity(*volatileEntityDynamicDatas[i]);
		}
	}

	for (int i = 0; i < volatileEntityStaticData.size(); i++)
	{
		if (volatileEntityStaticData[i]->pair == nullptr)
		{
			addStaticEntity(*volatileEntityStaticData[i]);
		}
	}
}

void EntityHandler::deleteSelected()
{
	int minX = selectRegionMinBoundary.x;
	int maxX = selectRegionMaxBoundary.x;
	int minY = selectRegionMinBoundary.y;
	int maxY = selectRegionMaxBoundary.y;

	std::vector<EntityConnector>::iterator connectorItr = entityConnections.begin();
	while(connectorItr != entityConnections.end())
	{
		if (!(connectorItr->e1->entityCoordx < minX || connectorItr->e1->entityCoordx > maxX ||
			  connectorItr->e1->entityCoordy < minY || connectorItr->e1->entityCoordy > maxY) &&
			!(connectorItr->e2->entityCoordx < minX || connectorItr->e2->entityCoordx > maxX ||
			  connectorItr->e2->entityCoordy < minY || connectorItr->e2->entityCoordy > maxY))
		{
			connectorItr = entityConnections.erase(connectorItr);
		}
		else
		{
			connectorItr++;
		}
	}

	std::vector<EntityData*> toDelete;
	std::vector<EntityData*>::iterator entityItr = entityDatas.begin();

	while (entityItr != entityDatas.end())
	{
		if ((*entityItr)->pair == nullptr)
		{
			if (!((*entityItr)->entityCoordx < minX || (*entityItr)->entityCoordx > maxX ||
				  (*entityItr)->entityCoordy < minY || (*entityItr)->entityCoordy > maxY))
			{
				toDelete.push_back(*entityItr);
				entityItr = entityDatas.erase(entityItr);
			}
			else
			{
				entityItr++;
			}
		}
		else
		{
			if (!((*entityItr)->entityCoordx < minX || (*entityItr)->entityCoordx > maxX ||
				  (*entityItr)->entityCoordy < minY || (*entityItr)->entityCoordy > maxY) &&
				!((*entityItr)->pair->entityCoordx < minX || (*entityItr)->pair->entityCoordx > maxX ||
				  (*entityItr)->pair->entityCoordy < minY || (*entityItr)->pair->entityCoordy > maxY))
			{
				toDelete.push_back(*entityItr);
				entityItr = entityDatas.erase(entityItr);
			}
			else
			{
				entityItr++;
			}
		}
	}

	for (int i = 0; i < toDelete.size(); i++)
	{
		delete toDelete[i];
	}

	setStaticBuffers();
}

void EntityHandler::copySelected()
{
	stageSelected();
}

void EntityHandler::cutSelected()
{
	stageSelected();
	deleteSelected();
}

void EntityHandler::flipSelectedHorizontally()
{
	for (int i = 0; i < volatileEntityDynamicDatas.size(); i++)
	{
		flipEntityHorizontally(volatileEntityDynamicDatas[i], selectRegionMinBoundary.x, selectRegionMaxBoundary.x);
	}
	setVolatileBuffers();
}

void EntityHandler::flipSelectedVertically()
{
	for (int i = 0; i < volatileEntityDynamicDatas.size(); i++)
	{
		flipEntityVertically(volatileEntityDynamicDatas[i], selectRegionMinBoundary.y, selectRegionMaxBoundary.y);
	}
	setVolatileBuffers();
}

void EntityHandler::rotateSelectedClockwise()
{
	for (int i = 0; i < volatileEntityDynamicDatas.size(); i++)
	{
		rotateEntityClockwise(volatileEntityDynamicDatas[i], pivotEntityLocation);
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
		rotateEntityCounterClockwise(volatileEntityDynamicDatas[i], pivotEntityLocation);
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

///////////////////////////////////////////////////////////////////////

void EntityHandler::update()
{
	clearStaticBuffers();
	setStaticBuffers();
}

void EntityHandler::draw(glm::mat4 viewProjMtx)
{
	drawEntities(viewProjMtx, entityDataBuffer, (GLsizei)entityDatas.size());
	drawConnectors(viewProjMtx, entityConnectorBuffer, (GLsizei)entityConnections.size());
	drawEntities(viewProjMtx, volatileEntityDataBuffer, (GLsizei)(volatileEntityStaticData.size() + volatileEntityDynamicDatas.size()));
	drawConnectors(viewProjMtx, volatileEntityConnectorBuffer, (GLsizei)volatileEntityConnections.size());
}

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

	uint entityColorLoc = 3;
	glEnableVertexAttribArray(entityColorLoc);
	glVertexAttribPointer(entityColorLoc, 3, GL_FLOAT, GL_FALSE, sizeof(EntityData), (void*)(4 * sizeof(int)));

	uint entityHighlightLoc = 4;
	glEnableVertexAttribArray(entityHighlightLoc);
	glVertexAttribIPointer(entityHighlightLoc, 1, GL_INT, sizeof(EntityData), (void*)(4 * sizeof(int) + 3 * sizeof(float)));

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

void EntityHandler::deleteEntity(EntityData* data)
{
	if (data == nullptr) { return; }
	clearStaticBuffers();
	EntityData* entityToDelete = nullptr;
	EntityData* pairToDelete = nullptr;
	for (int i = 0; i < entityDatas.size(); i++)
	{
		if (data == entityDatas[i])
		{
			entityToDelete = entityDatas[i];
			if (entityDatas[i]->pair != nullptr) { pairToDelete = entityDatas[i]->pair; }
			entityDatas.erase(entityDatas.begin() + i);
			break;
		}
	}

	if (pairToDelete != nullptr)
	{
		for (int i = 0; i < entityConnections.size(); i++)
		{
			if (pairToDelete == entityConnections[i].e1 || pairToDelete == entityConnections[i].e2)
			{
				entityConnections.erase(entityConnections.begin() + i);
				break;
			}
		}

		for (int i = 0; i < entityDatas.size(); i++)
		{
			if (pairToDelete == entityDatas[i])
			{
				entityDatas.erase(entityDatas.begin() + i);
				break;
			}
		}
	}

	if (entityToDelete != nullptr) { delete entityToDelete; }
	if (pairToDelete != nullptr) { delete pairToDelete; }
	setStaticBuffers();
}

//May return a pointer to nullptr to indicate no closest
EntityData* EntityHandler::findClosestEntity(const glm::vec2 entityCoord, const float radius)
{
	EntityData* returnptr = nullptr;
	float closest = radius * radius;
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
		//If the entity to highlight is the same as the one about to be unlighted (this can include its pair), then no highlighting change is needed.
		if (entityToUnHighlight == entityToHighlight || (entityToHighlight != nullptr && entityToUnHighlight == entityToHighlight->pair)) { return; }

		entityToUnHighlight->highlight = 0;
		switch (entityToUnHighlight->type)
		{
		case EXIT: case EXIT_SWITCH: case LOCKED_DOOR: case LOCKED_DOOR_SWITCH: case TRAP_DOOR: case TRAO_DOOR_SWITCH:
		{
			if (entityToUnHighlight->pair != nullptr) { entityToUnHighlight->pair->highlight = 0; }
			for (int i = 0; i < entityConnections.size(); i++)
			{
				if (entityToUnHighlight == entityConnections[i].e1 || entityToUnHighlight == entityConnections[i].e2) { entityConnections[i].highlight = 0; }
			}
			break;
		}
		default:
			break;
		}
		needUpdate = true;
	}

	if (entityToHighlight != nullptr)
	{
		entityToHighlight->highlight = 1;
		switch (entityToHighlight->type)
		{
		case EXIT: case EXIT_SWITCH: case LOCKED_DOOR: case LOCKED_DOOR_SWITCH: case TRAP_DOOR: case TRAO_DOOR_SWITCH:
		{
			if (entityToHighlight->pair != nullptr) { entityToHighlight->pair->highlight = 1; }
			for (int i = 0; i < entityConnections.size(); i++)
			{
				if (entityToHighlight == entityConnections[i].e1 || entityToHighlight == entityConnections[i].e2) { entityConnections[i].highlight = 1; }
			}
			break;
		}
		default:
			break;
		}

		needUpdate = true;
	}

	if (needUpdate) { this->update(); }
}

void EntityHandler::setStaticBuffers()
{
	if (entityDatas.size() > 0)
	{
		std::vector<EntityData> dataBuffer1;
		dataBuffer1.reserve(entityDatas.size());
		for (int i = 0; i < entityDatas.size(); i++)
		{
			dataBuffer1.push_back(*entityDatas[i]);
		}

		// Store vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, entityDataBuffer);
		glBufferData(GL_ARRAY_BUFFER, dataBuffer1.size() * sizeof(EntityData), &dataBuffer1[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (entityConnections.size() > 0)
	{
		std::vector<ConnectorShaderInfo> dataBuffer2;
		dataBuffer2.reserve(entityConnections.size());
		for (int i = 0; i < entityConnections.size(); i++)
		{
			ConnectorShaderInfo temp;
			temp.entity1Coord = glm::ivec2(entityConnections[i].e1->entityCoordx, entityConnections[i].e1->entityCoordy);
			temp.entity2Coord = glm::ivec2(entityConnections[i].e2->entityCoordx, entityConnections[i].e2->entityCoordy);
			temp.highlight = entityConnections[i].highlight;
			dataBuffer2.push_back(temp);
		}

		glBindBuffer(GL_ARRAY_BUFFER, entityConnectorBuffer);
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
	if (volatileEntityStaticData.size() > 0 || volatileEntityDynamicDatas.size() > 0)
	{
		std::vector<EntityData> dataBuffer1;
		dataBuffer1.reserve(volatileEntityStaticData.size() + volatileEntityDynamicDatas.size());
		for (int i = 0; i < volatileEntityStaticData.size(); i++)
		{
			dataBuffer1.push_back(*volatileEntityStaticData[i]);
		}
		for (int i = 0; i < volatileEntityDynamicDatas.size(); i++)
		{
			dataBuffer1.push_back(*volatileEntityDynamicDatas[i]);
		}

		// Store vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, volatileEntityDataBuffer);
		glBufferData(GL_ARRAY_BUFFER, dataBuffer1.size() * sizeof(EntityData), &dataBuffer1[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (volatileEntityConnections.size() > 0)
	{
		std::vector<ConnectorShaderInfo> dataBuffer2;
		dataBuffer2.reserve(volatileEntityConnections.size());
		for (int i = 0; i < volatileEntityConnections.size(); i++)
		{
			ConnectorShaderInfo temp;
			temp.entity1Coord = glm::ivec2(volatileEntityConnections[i].e1->entityCoordx, volatileEntityConnections[i].e1->entityCoordy);
			temp.entity2Coord = glm::ivec2(volatileEntityConnections[i].e2->entityCoordx, volatileEntityConnections[i].e2->entityCoordy);
			temp.highlight = volatileEntityConnections[i].highlight;
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

	volatileEntityStaticData.clear();
	volatileEntityDynamicDatas.clear();
	volatileEntityConnections.clear();
}

void EntityHandler::flipEntityHorizontally(EntityData* entity, int xMinBoundary, int xMaxBoundary)
{
	entity->entityCoordx = xMaxBoundary - (entity->entityCoordx - xMinBoundary);

	switch (entity->type)
	{
		//8-rotation style type: 0 <-> 4, 1 <-> 3, 5 <-> 7
		case LOCKED_DOOR: case TRAP_DOOR: case BOUNCE_PAD: case ONE_WAY: case LASER_TURRET: case BOOST_PAD:
		{
			int baseRotation = entity->rotation / 4;
			int remainder = entity->rotation % 4;
			entity->rotation = (baseRotation * 4 + 4 - remainder) % 8;
			break;
		}
		//4-rotation style type: 0 <-> 4
		case CHAINGUN: case LASER_DRONE: case REGULAR_DRONE: case CHASE_DRONE: case MICRO_DRONE: case THWUMP:
		{
			int baseRotation = entity->rotation / 4;
			int remainder = entity->rotation % 4;
			entity->rotation = (baseRotation * 4 + 4 - remainder) % 8;
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

void EntityHandler::flipEntityVertically(EntityData* entity, int yMinBoundary, int yMaxBoundary)
{
	entity->entityCoordy = yMaxBoundary - (entity->entityCoordy - yMinBoundary);

	switch (entity->type)
	{
		//8-rotation style type: 1 <-> 7, 2 <-> 6, 3 <-> 5
		case LOCKED_DOOR: case TRAP_DOOR: case BOUNCE_PAD: case ONE_WAY: case LASER_TURRET: case BOOST_PAD:
		{
			entity->rotation = (8 - entity->rotation) % 8;
			break;
		}
		//4-rotation style type: 2 <-> 6
		case CHAINGUN: case LASER_DRONE: case REGULAR_DRONE: case CHASE_DRONE: case MICRO_DRONE: case THWUMP:
		{
			entity->rotation = (8 - entity->rotation) % 8;
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

void EntityHandler::rotateEntityClockwise(EntityData* entity, const glm::ivec2 & pivot)
{
	int dx = entity->entityCoordx - pivot.x;
	int dy = entity->entityCoordy - pivot.y;

	entity->entityCoordx = pivot.x + dy;
	entity->entityCoordy = pivot.y - dx;
	entity->rotation = (entity->rotation + 8 + 2) % 8;

	//TODO: Perform sanity check
	Entity::sanitizeImpossibleValue(entity);
}

void EntityHandler::rotateEntityCounterClockwise(EntityData* entity, const glm::ivec2 & pivot)
{
	int dx = entity->entityCoordx - pivot.x;
	int dy = entity->entityCoordy - pivot.y;

	entity->entityCoordx = pivot.x - dy;
	entity->entityCoordy = pivot.y + dx;
	entity->rotation = (entity->rotation + 8 - 2) % 8;

	//TODO: Perform sanity check
	Entity::sanitizeImpossibleValue(entity);
}
