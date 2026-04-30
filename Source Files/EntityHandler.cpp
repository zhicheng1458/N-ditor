#include "EntityHandler.h"
#include <iostream>

EntityHandler::EntityHandler(float tileSize, const Palette& p)
{
	entityShader = new ShaderProgram("./Resources/Shaders/Entity.glsl", ShaderProgram::eGeometry);
	connectorShader = new ShaderProgram("./Resources/Shaders/EntityConnector.glsl", ShaderProgram::eGeometry);
	this->tileSize = tileSize;

	hintEntity1 = new EntityData();
	hintEntity2 = new EntityData();

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
	delete entityShader;
	delete connectorShader;
	delete hintEntity1;
	delete hintEntity2;

	for (int i = 0; i < entityDatas.size(); i++)
	{
		delete entityDatas[i];
		entityDatas[i] = nullptr;
	}
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
	update();
}

void EntityHandler::stopHighlight()
{
	resolveHighlight(nullptr, lastHighlightedEntity);
	lastHighlightedEntity = nullptr;
}

void EntityHandler::showHintEntity()
{
	drawHintEntity = true;
}

void EntityHandler::hideHintEntity()
{
	drawHintEntity = false;
}

//It is strongly recommended to utilize the move method to move the hint entity around
//as it is applicated to area selection as well.
void EntityHandler::setHintEntity(entityType type, int cursorXCoord, int cursorYCoord, EntityRotation rotation)
{
	clearVolatileBuffers();
	hintEntity1->type = type;
	hintEntity1->entityCoordx = cursorXCoord;
	hintEntity1->entityCoordy = cursorYCoord;
	hintEntity1->rotation = rotation;
	volatileEntityDynamicDatas.push_back(hintEntity1);
	setVolatileBuffers();
}

void EntityHandler::setHintEntityRotation(EntityRotation rotation)
{
	if (hintEntity1Placed)
	{
		hintEntity2->rotation = rotation;
	}
	else
	{
		hintEntity1->rotation = rotation;
	}
	setVolatileBuffers();
}

void EntityHandler::placeEntity()
{
	if (hintEntity1->type == NONE) { return; } //Prevent placing a ghost entity.

	clearVolatileBuffers();
	
	if (hintEntity1->type != EXIT &&
		hintEntity1->type != EXIT_SWITCH &&
		hintEntity1->type != LOCKED_DOOR &&
		hintEntity1->type != LOCKED_DOOR_SWITCH &&
		hintEntity1->type != TRAP_DOOR &&
		hintEntity1->type != TRAO_DOOR_SWITCH)
	{
		this->addStaticEntity(*hintEntity1);
		volatileEntityDynamicDatas.push_back(hintEntity1);
		setVolatileBuffers();
		return;
	}

	if (hintEntity1Placed == false)
	{
		volatileEntityStaticData.push_back(hintEntity1);
		hintEntity2->entityCoordx = hintEntity1->entityCoordx;
		hintEntity2->entityCoordy = hintEntity1->entityCoordy;
		hintEntity2->rotation = hintEntity1->rotation;
		volatileEntityDynamicDatas.push_back(hintEntity2);
		EntityConnector temp;
		temp.e1 = hintEntity1;
		temp.e2 = hintEntity2;
		temp.highlight = 0;
		volatileEntityConnections.push_back(temp);

		switch (hintEntity1->type)
		{
		case EXIT:
			hintEntity2->type = EXIT_SWITCH; break;
		case EXIT_SWITCH:
			hintEntity2->type = EXIT; break;
		case LOCKED_DOOR:
			hintEntity2->type = LOCKED_DOOR_SWITCH; break;
		case LOCKED_DOOR_SWITCH:
			hintEntity2->type = LOCKED_DOOR; break;
		case TRAP_DOOR:
			hintEntity2->type = TRAO_DOOR_SWITCH; break;
		case TRAO_DOOR_SWITCH:
			hintEntity2->type = TRAP_DOOR; break;
		default:
			break;
		}

		setVolatileBuffers();
		hintEntity1Placed = true;
	}
	else
	{
		this->addStaticEntity(*hintEntity1, *hintEntity2);
		hintEntity1->entityCoordx = hintEntity2->entityCoordx;
		hintEntity1->entityCoordy = hintEntity2->entityCoordy;
		hintEntity1->rotation = hintEntity2->rotation;
		switch (hintEntity2->type)
		{
		case EXIT:
			hintEntity1->type = EXIT_SWITCH; break;
		case EXIT_SWITCH:
			hintEntity1->type = EXIT; break;
		case LOCKED_DOOR:
			hintEntity1->type = LOCKED_DOOR_SWITCH; break;
		case LOCKED_DOOR_SWITCH:
			hintEntity1->type = LOCKED_DOOR; break;
		case TRAP_DOOR:
			hintEntity1->type = TRAO_DOOR_SWITCH; break;
		case TRAO_DOOR_SWITCH:
			hintEntity1->type = TRAP_DOOR; break;
		default:
			break;
		}
		volatileEntityDynamicDatas.push_back(hintEntity1);
		setVolatileBuffers();
		hintEntity1Placed = false;
	}
}

void EntityHandler::moveHint(int cursorX, int cursorY)
{
	if (volatileEntityDynamicDatas.size() == 0) { return; }

	int dx = cursorX - volatileEntityDynamicDatas[0]->entityCoordx;
	int dy = cursorY - volatileEntityDynamicDatas[0]->entityCoordy;
	volatileEntityDynamicDatas[0]->entityCoordx = cursorX;
	volatileEntityDynamicDatas[0]->entityCoordy = cursorY;

	for (int i = 1; i < volatileEntityDynamicDatas.size(); i++)
	{
		volatileEntityDynamicDatas[i]->entityCoordx += dx;
		volatileEntityDynamicDatas[i]->entityCoordy += dy;
	}
	setVolatileBuffers(); //TODO: use buffer sub data to speed this part up
}

void EntityHandler::update()
{
	clearStaticBuffers();
	setStaticBuffers();
}

void EntityHandler::draw(glm::mat4 viewProjMtx)
{
	drawEntities(viewProjMtx);
	drawConnectors(viewProjMtx, entityConnections, entityConnectorBuffer);
	if (drawHintEntity)
	{
		drawHint(viewProjMtx);
		drawConnectors(viewProjMtx, volatileEntityConnections, volatileEntityConnectorBuffer);
	}
}

void EntityHandler::drawEntities(glm::mat4 viewProjMtx)
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
	glBindBuffer(GL_ARRAY_BUFFER, entityDataBuffer);

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

	glDrawArrays(GL_POINTS, 0, (GLsizei)entityDatas.size()); //Use GL_POINTS because every "point" is 1 EntityData.

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLenum error = 0;
	if ((error = glGetError()) != GL_NO_ERROR)
	{
		printf("%i \n", (int)error);
	}

	glEnable(GL_CULL_FACE);

	glUseProgram(0);
}

void EntityHandler::drawConnectors(glm::mat4 viewProjMtx, const std::vector<EntityConnector>& connectionDatas, const uint connectorBuffer)
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

	glDrawArrays(GL_POINTS, 0, (GLsizei)connectionDatas.size()); //Use GL_POINTS because every "point" is 1 EntityData.

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLenum error = 0;
	if ((error = glGetError()) != GL_NO_ERROR)
	{
		printf("%i \n", (int)error);
	}

	glEnable(GL_CULL_FACE);

	glUseProgram(0);
}

void EntityHandler::drawHint(glm::mat4 viewProjMtx)
{
	// Set up shader
	glDisable(GL_CULL_FACE);
	glUseProgram(entityShader->getProgramID());

	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, entitySpriteSheet->getTextureID());
	glBindTexture(GL_TEXTURE_2D, palette->getEntityTextureID());

	glUniformMatrix4fv(glGetUniformLocation(entityShader->getProgramID(), "viewProjMtx"), 1, false, (float*)&viewProjMtx);

	// Set up state
	glBindBuffer(GL_ARRAY_BUFFER, volatileEntityDataBuffer);

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

	glDrawArrays(GL_POINTS, 0, (GLsizei)volatileEntityStaticData.size() + volatileEntityDynamicDatas.size()); //Use GL_POINTS because every "point" is 1 EntityData.

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLenum error = 0;
	if ((error = glGetError()) != GL_NO_ERROR)
	{
		printf("%i \n", (int)error);
	}

	glEnable(GL_CULL_FACE);

	glUseProgram(0);
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
	clearStaticBuffers();
	EntityData* entity = new EntityData(data);
	entityDatas.push_back(entity);
	setStaticBuffers();
}

void EntityHandler::addStaticEntity(EntityData data, EntityData pair)
{
	clearStaticBuffers();
	EntityData* entity = new EntityData(data);
	EntityData* pairEntity = new EntityData(pair);
	entity->pair = pairEntity;
	pairEntity->pair = entity;
	entityDatas.push_back(entity);
	entityDatas.push_back(pairEntity);
	EntityConnector connector;
	connector.e1 = entity;
	connector.e2 = pairEntity;
	connector.highlight = 0;
	entityConnections.push_back(connector);
	setStaticBuffers();
}

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
	volatileEntityStaticData.clear();
	volatileEntityDynamicDatas.clear();
	volatileEntityConnections.clear();
}
