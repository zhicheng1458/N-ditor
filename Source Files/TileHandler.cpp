#include "TileHandler.h"

TileHandler::TileHandler(float tileSize, const Palette& p)
{
	tileShader = new ShaderProgram("./Resources/Shaders/Tile.glsl", ShaderProgram::eGeometry);
	this->tileSize = tileSize;
	palette = &p;

	glGenBuffers(1, &tileDataBuffer);

	tileTree = new AABBTree();
}

TileHandler::~TileHandler()
{
	delete tileShader;

	for (int i = 0; i < tileDatas.size(); i++)
	{
		delete tileDatas[i];
		tileDatas[i] = nullptr;
	}
	glDeleteBuffers(1, &tileDataBuffer);

	delete tileTree;
}

void TileHandler::usePalette(const Palette& p)
{
	palette = &p;

	clearBuffers();
	for (int i = 0; i < tileDatas.size(); i++)
	{
		tileDatas[i]->color = palette->backgroundColors.tileColor / 255.0f;
	}
	setBuffers();
}

void TileHandler::addTile(TileData data)
{
	// Old no tree implementation
	clearBuffers();
	bool found = false;
	for (int i = 0; i < tileDatas.size(); i++)
	{
		if (tileDatas[i]->tileCoordx == data.tileCoordx && tileDatas[i]->tileCoordy == data.tileCoordy)
		{
			tileDatas[i]->rotation = data.rotation;
			tileDatas[i]->type = data.type;
			found = true;
			break;
		}
	}
	if (!found)
	{
		data.color = palette->backgroundColors.tileColor / 255.0f;
		TileData* tile = new TileData(data);

		tileDatas.push_back(tile);
	}
	setBuffers();

	//Tree implementation for adding tiles
	/*
	int tileIndex = tileTree->find(glm::vec2(data.tileCoordx * tileSize + 0.01f, -data.tileCoordy * tileSize - 0.01f)); //Small offset to ensure search is for the tile;
	if (tileIndex < 0)
	{
		tileDatas.push_back(data);
		setBuffers();

		glm::vec2 lowerBound = glm::vec2(data.tileCoordx * tileSize, -data.tileCoordy * tileSize - tileSize);
		glm::vec2 upperBound = glm::vec2(data.tileCoordx * tileSize + tileSize, -data.tileCoordy * tileSize);
		AABB tileAABB(lowerBound, upperBound);
		AABBNode * newNode = new AABBNode(tileAABB);
		newNode->objectIndex = (int)tileDatas.size() - 1;
		tileTree->insertLeaf(*newNode);
		tileTree->update();
	}
	else
	{
		//It can also be empty tile, but it is still better than calling
		//remove and rebuild tree because that is very expensive on both
		//drawing the tiles and the tree building itself
		clearBuffers();
		tileDatas[tileIndex].type = data.type;
		tileDatas[tileIndex].rotation = data.rotation;
		setBuffers();
	}
	*/
}

void TileHandler::deleteTile(TileData data)
{
	clearBuffers();
	for (int i = 0; i < tileDatas.size(); i++)
	{
		if (isSameCoord(*tileDatas[i], data))
		{
			delete tileDatas[i];
			tileDatas[i] = nullptr;
			tileDatas.erase(tileDatas.begin() + i);
			break;
		}
	}
	setBuffers();
}

void TileHandler::draw(glm::mat4 viewProjMtx)
{
	//Custom shader is needed specifically for tiles

	if (allowModify)
	{
		fprintf(stderr, "Drawing may not be proceed as buffer is not set. Ending draw function \n");
		return;
	}

	// Set up shader
	glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);
	glUseProgram(tileShader->getProgramID());

	//glUniformMatrix4fv(glGetUniformLocation(tileShader->getProgramID(), "modelMtx"), 1, false, (float*)&modelMtx); //Irrelevant because each point has identity as model mtx
	glUniformMatrix4fv(glGetUniformLocation(tileShader->getProgramID(), "viewProjMtx"), 1, false, (float*)&viewProjMtx);

	//For geometry shader
	glUniform1f(glGetUniformLocation(tileShader->getProgramID(), "tileSize"), tileSize);

	// Set up state
	glBindBuffer(GL_ARRAY_BUFFER, tileDataBuffer);

	uint tileTypeLoc = 0;
	glEnableVertexAttribArray(tileTypeLoc);
	glVertexAttribIPointer(tileTypeLoc, 1, GL_INT, sizeof(TileData), (void*)(0 * sizeof(int)));

	uint tileCoordLoc = 1;
	glEnableVertexAttribArray(tileCoordLoc);
	glVertexAttribIPointer(tileCoordLoc, 2, GL_INT, sizeof(TileData), (void*)(1 * sizeof(int)));

	uint tileRotationLoc = 2;
	glEnableVertexAttribArray(tileRotationLoc);
	glVertexAttribIPointer(tileRotationLoc, 1, GL_INT, sizeof(TileData), (void*)(3 * sizeof(int)));

	uint tileColorLoc = 3;
	glEnableVertexAttribArray(tileColorLoc);
	glVertexAttribPointer(tileColorLoc, 3, GL_FLOAT, GL_FALSE, sizeof(TileData), (void*)(4 * sizeof(int))); //Careful of off-set

	glDrawArrays(GL_POINTS, 0, (GLsizei)tileDatas.size()); //Use GL_POINTS because every "point" is 1 TileData.

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLenum error = 0;
	if ((error = glGetError()) != GL_NO_ERROR)
	{
		printf("%i \n", (int)error);
	}

	glEnable(GL_CULL_FACE);

	glUseProgram(0);

	//tileTree->draw(viewProjMtx); //Use for debug tree
}

void TileHandler::setBuffers()
{
	if (tileDatas.size() > 0)
	{
		std::vector<TileData> dataBuffer;
		dataBuffer.reserve(tileDatas.size());

		for (int i = 0; i < tileDatas.size(); i++)
		{
			dataBuffer.push_back(*tileDatas[i]);
		}

		// Store vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, tileDataBuffer);
		glBufferData(GL_ARRAY_BUFFER, dataBuffer.size() * sizeof(TileData), &dataBuffer[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	allowModify = false;
}

void TileHandler::clearBuffers()
{
	allowModify = true; //What this really do is stopping the draw routine so u can modify the buffer
}

bool TileHandler::isSameCoord(TileData data1, TileData data2)
{
	return data1.tileCoordx == data2.tileCoordx && data1.tileCoordy == data2.tileCoordy;
}