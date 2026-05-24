#include "TileHandler.h"

TileHandler::TileHandler(float tileSize, const Palette& p)
{
	tileShader = new ShaderProgram("./Resources/Shaders/Tile.glsl", ShaderProgram::eGeometry);
	this->tileSize = tileSize;
	palette = &p;

	glGenBuffers(1, &tileDataBuffer);
	glGenBuffers(1, &volatileTileDataBuffer);

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
	glDeleteBuffers(1, &volatileTileDataBuffer);

	delete tileTree;

}

void TileHandler::clearData()
{
	for (int i = 0; i < tileDatas.size(); i++)
	{
		delete tileDatas[i];
		tileDatas[i] = nullptr;
	}
	tileDatas.clear();

	clearBuffers();
	setBuffers();
	clearVolatileBuffers();
	setVolatileBuffers();
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

void TileHandler::addTile(TileData data, Modification& recorder)
{
	// Old no tree implementation
	clearBuffers();

	if (data.type == EMPTY) { data.rotation = TILE_DEGREE_0; } //Quick sanity check in case of empty data supplied

	bool found = false;
	for (int i = 0; i < tileDatas.size(); i++)
	{
		if (tileDatas[i]->tileCoordx == data.tileCoordx && tileDatas[i]->tileCoordy == data.tileCoordy)
		{
			//If the tile being added is the exact same, then the tile adding step will be skipped
			//(to prevent accidental misclick for save operation)
			if (tileDatas[i]->rotation != data.rotation || tileDatas[i]->type != data.type)
			{
				//Record the tile that gets changed
				recorder.oldTiles.push_back(*tileDatas[i]);

				tileDatas[i]->rotation = data.rotation;
				tileDatas[i]->type = data.type;

				recorder.newTiles.push_back(*tileDatas[i]);
			}
			found = true;
			break;
		}
	}
	if (!found)
	{
		//If it is attempting to add an empty tile to a location where tile doesn't exist yet, skip this step.
		//(to prevent accidental misclick for save operation)
		if (data.type != EMPTY)
		{
			TileData* tile = new TileData(data);
			tile->color = palette->backgroundColors.tileColor / 255.0f;

			tileDatas.push_back(tile);
			//Record the tile that gets changed. This is equivalent to changing an empty tile to a valid tile.
			data.type = EMPTY;
			recorder.oldTiles.push_back(data);
			recorder.newTiles.push_back(*tile);
		}
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

/*
void TileHandler::deleteTile(TileData data, Modification & recorder)
{
	clearBuffers();
	for (int i = 0; i < tileDatas.size(); i++)
	{
		if (isSameCoord(*tileDatas[i], data.tileCoordx, data.tileCoordy))
		{
			//Record the tile that gets changed
			if (tileDatas[i]->type != EMPTY)
			{
				recorder.tiles.push_back(*tileDatas[i]);
			}

			delete tileDatas[i];
			tileDatas[i] = nullptr;
			tileDatas.erase(tileDatas.begin() + i);
			break;
		}
	}
	setBuffers();
}
*/

void TileHandler::draw(glm::mat4 viewProjMtx)
{
	drawTiles(viewProjMtx, tileDataBuffer, (GLsizei)tileDatas.size());
	drawTiles(viewProjMtx, volatileTileDataBuffer, (GLsizei)volatileTileData.size());

	//tileTree->draw(viewProjMtx); //Use for debug tree
}

///////////////////////////////////////////////////////////////////////

void TileHandler::setHintToFollowMouse(bool toFollow)
{
	followMouse = toFollow;
}

void TileHandler::moveHint(glm::vec2 cursorInModelSpace) //Basically equivalent to update();
{
	if (!followMouse) { return; }

	//if (volatileTileData.size() == 0) { return; } //the selected region will move regardless of if there's any tile selected

	glm::ivec2 newPivotLocation = UtilityFunctions::clampToNearestTileCoord(cursorInModelSpace, tileSize);
	glm::ivec2 diff = newPivotLocation - pivotTileLocation;
	if (diff.x == 0 && diff.y == 0) { return; } //No change

	for (int i = 0; i < volatileTileData.size(); i++)
	{
		volatileTileData[i]->tileCoordx += diff.x;
		volatileTileData[i]->tileCoordy += diff.y;
	}

	selectRegionMinBoundary = selectRegionMinBoundary + diff;
	selectRegionMaxBoundary = selectRegionMaxBoundary + diff;

	pivotTileLocation = newPivotLocation;
	setVolatileBuffers(); //TODO: use buffer sub data to speed this part up
}

void TileHandler::setSelectedRegion(glm::vec2 corner1, glm::vec2 corner2)
{
	glm::ivec2 corner1TileCoord = UtilityFunctions::clampToNearestTileCoord(corner1, tileSize);
	glm::ivec2 corner2TileCoord = UtilityFunctions::clampToNearestTileCoord(corner2, tileSize);
	int minX = std::min(corner1TileCoord.x, corner2TileCoord.x);
	int maxX = std::max(corner1TileCoord.x, corner2TileCoord.x);
	int minY = std::min(corner1TileCoord.y, corner2TileCoord.y);
	int maxY = std::max(corner1TileCoord.y, corner2TileCoord.y);

	selectRegionMinBoundary = glm::ivec2(minX, minY);
	selectRegionMaxBoundary = glm::ivec2(maxX, maxY);

	pivotTileLocation = corner2TileCoord;
}

void TileHandler::stageSelected()
{
	if (volatileTileData.size() > 0)
	{
		unstageSelected();
	}

	glm::vec3 selectedTileColor = palette->editorColors.highlightColor / 255.0f;

	for (int i = 0; i < tileDatas.size(); i++)
	{
		if (!(tileDatas[i]->tileCoordx < selectRegionMinBoundary.x || tileDatas[i]->tileCoordx > selectRegionMaxBoundary.x ||
			  tileDatas[i]->tileCoordy < selectRegionMinBoundary.y || tileDatas[i]->tileCoordy > selectRegionMaxBoundary.y))
		{
			/* TODO: This is basically same as adding a new entity but on the volatile vector */
			TileData * t = new TileData(*tileDatas[i]);
			t->color = selectedTileColor;
			volatileTileData.push_back(t);
		}
	}

	//Fill all coordinate that didn't have a tile in it with an empty tile
	int row = (selectRegionMaxBoundary.y - selectRegionMinBoundary.y + 1); //also the height
	int col = (selectRegionMaxBoundary.x - selectRegionMinBoundary.x + 1); //also the width
	int size = row * col;

	if (volatileTileData.size() != size)
	{
		std::vector<bool> hasTile(size);
		for (int i = 0; i < hasTile.size(); i++)
		{
			hasTile[i] = false;
		}

		//Order goes for row first then column
		for (int i = 0; i < volatileTileData.size(); i++)
		{
			int index = (volatileTileData[i]->tileCoordy - selectRegionMinBoundary.y) * col + (volatileTileData[i]->tileCoordx - selectRegionMinBoundary.x);
			hasTile[index] = true;
		}

		for (int i = 0; i < hasTile.size(); i++)
		{
			if (!hasTile[i])
			{
				TileData* emptyTile = new TileData();
				emptyTile->tileCoordx = i % col + selectRegionMinBoundary.x;
				emptyTile->tileCoordy = i / col + selectRegionMinBoundary.y;
				emptyTile->type = EMPTY;
				emptyTile->color = selectedTileColor;

				volatileTileData.push_back(emptyTile);
			}
		}
	}

	setVolatileBuffers();
}

void TileHandler::unstageSelected()
{
	clearVolatileBuffers();
}

void TileHandler::pasteSelected(Modification& recorder)
{
	for (int i = 0; i < volatileTileData.size(); i++)
	{
		addTile(*volatileTileData[i], recorder);
	}
}

void TileHandler::fillSelected(Modification& recorder)
{
	//addtile method has sanity check to ensure no stacked tile.

	TileData t;
	t.rotation = TILE_DEGREE_0;
	for (int r = selectRegionMinBoundary.x; r <= selectRegionMaxBoundary.x; r++) //Nothing was staged :/
	{
		for (int c = selectRegionMinBoundary.y; c <= selectRegionMaxBoundary.y; c++)
		{
			t.tileCoordx = r;
			t.tileCoordy = c;
			t.type = FULL;

			addTile(t, recorder);
		}
	}

	setBuffers();
}

void TileHandler::deleteSelected(Modification& recorder)
{
	deleteSelectedTileCoordBased(selectRegionMinBoundary, selectRegionMaxBoundary, recorder);
	setBuffers();
}

void TileHandler::copySelected()
{
	stageSelected();
}

void TileHandler::cutSelected(Modification& recorder)
{
	stageSelected();
	deleteSelected(recorder);
}

void TileHandler::flipSelectedHorizontally()
{
	for (int i = 0; i < volatileTileData.size(); i++)
	{
		flipTileHorizontally(*volatileTileData[i], selectRegionMinBoundary.x, selectRegionMaxBoundary.x);
	}
	setVolatileBuffers();
}

void TileHandler::flipSelectedVertically()
{
	for (int i = 0; i < volatileTileData.size(); i++)
	{
		flipTileVertically(*volatileTileData[i], selectRegionMinBoundary.y, selectRegionMaxBoundary.y);
	}
	setVolatileBuffers();
}

void TileHandler::rotateSelectedClockwise()
{
	for (int i = 0; i < volatileTileData.size(); i++)
	{
		rotateTileClockwise(*volatileTileData[i], pivotTileLocation);
	}

	//The two corner of the selected region also need to be rotated
	int dx = selectRegionMinBoundary.x - pivotTileLocation.x;
	int dy = selectRegionMinBoundary.y - pivotTileLocation.y;
	selectRegionMinBoundary.x = pivotTileLocation.x + dy;
	selectRegionMinBoundary.y = pivotTileLocation.y - dx;

	dx = selectRegionMaxBoundary.x - pivotTileLocation.x;
	dy = selectRegionMaxBoundary.y - pivotTileLocation.y;
	selectRegionMaxBoundary.x = pivotTileLocation.x + dy;
	selectRegionMaxBoundary.y = pivotTileLocation.y - dx;

	int minX = std::min(selectRegionMinBoundary.x, selectRegionMaxBoundary.x);
	int maxX = std::max(selectRegionMinBoundary.x, selectRegionMaxBoundary.x);
	int minY = std::min(selectRegionMinBoundary.y, selectRegionMaxBoundary.y);
	int maxY = std::max(selectRegionMinBoundary.y, selectRegionMaxBoundary.y);

	selectRegionMinBoundary = glm::ivec2(minX, minY);
	selectRegionMaxBoundary = glm::ivec2(maxX, maxY);

	setVolatileBuffers();
}

void TileHandler::rotateSelectedCounterClockwise()
{
	for (int i = 0; i < volatileTileData.size(); i++)
	{
		rotateTileCounterClockwise(*volatileTileData[i], pivotTileLocation);
	}

	//The two corner of the selected region also need to be rotated
	int dx = selectRegionMinBoundary.x - pivotTileLocation.x;
	int dy = selectRegionMinBoundary.y - pivotTileLocation.y;
	selectRegionMinBoundary.x = pivotTileLocation.x - dy;
	selectRegionMinBoundary.y = pivotTileLocation.y + dx;

	dx = selectRegionMaxBoundary.x - pivotTileLocation.x;
	dy = selectRegionMaxBoundary.y - pivotTileLocation.y;
	selectRegionMaxBoundary.x = pivotTileLocation.x - dy;
	selectRegionMaxBoundary.y = pivotTileLocation.y + dx;

	int minX = std::min(selectRegionMinBoundary.x, selectRegionMaxBoundary.x);
	int maxX = std::max(selectRegionMinBoundary.x, selectRegionMaxBoundary.x);
	int minY = std::min(selectRegionMinBoundary.y, selectRegionMaxBoundary.y);
	int maxY = std::max(selectRegionMinBoundary.y, selectRegionMaxBoundary.y);

	selectRegionMinBoundary = glm::ivec2(minX, minY);
	selectRegionMaxBoundary = glm::ivec2(maxX, maxY);

	setVolatileBuffers();
}

void TileHandler::invertSelected()
{
	/*
	int row = (selectRegionMaxBoundary.y - selectRegionMinBoundary.y + 1); //also the height
	int col = (selectRegionMaxBoundary.x - selectRegionMinBoundary.x + 1); //also the width
	int size = row * col;

	if (volatileTileData.size() != size)
	{
		//Fill all coordinate that didn't have a tile in it with an empty tile
		std::vector<bool> hasTile(size);
		for (int i = 0; i < hasTile.size(); i++)
		{
			hasTile[i] = false;
		}

		//Order goes for row first then column
		for (int i = 0; i < volatileTileData.size(); i++)
		{
			int index = (volatileTileData[i]->tileCoordy - selectRegionMinBoundary.y) * col + (volatileTileData[i]->tileCoordx - selectRegionMinBoundary.x);
			hasTile[index] = true;
		}

		glm::vec3 selectedTileColor = palette->editorColors.highlightColor / 255.0f;

		for (int i = 0; i < hasTile.size(); i++)
		{
			if (!hasTile[i])
			{
				TileData* emptyTile = new TileData();
				emptyTile->tileCoordx = i % col + selectRegionMinBoundary.x;
				emptyTile->tileCoordy = i / col + selectRegionMinBoundary.y;
				emptyTile->type = EMPTY;
				emptyTile->color = selectedTileColor;

				volatileTileData.push_back(emptyTile);
			}
		}
	}
	*/

	for (int i = 0; i < volatileTileData.size(); i++)
	{
		invertTile(*volatileTileData[i]);
	}

	setVolatileBuffers();
}

///////////////////////////////////////////////////////////////////////

const std::vector<TileData*>& TileHandler::getTileData() const
{
	return tileDatas;
}

///////////////////////////////////////////////////////////////////////

void TileHandler::drawTiles(glm::mat4 viewProjMtx, const uint tileBuffer, GLsizei tileBufferSize)
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
	glBindBuffer(GL_ARRAY_BUFFER, tileBuffer);

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

	glDrawArrays(GL_POINTS, 0, tileBufferSize); //Use GL_POINTS because every "point" is 1 TileData.

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLenum error = 0;
	if ((error = glGetError()) != GL_NO_ERROR)
	{
		printf("%i \n", (int)error);
	}

	glEnable(GL_CULL_FACE);

	glUseProgram(0);
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

void TileHandler::setVolatileBuffers()
{
	if (volatileTileData.size() > 0)
	{
		std::vector<TileData> dataBuffer;
		dataBuffer.reserve(volatileTileData.size());

		for (int i = 0; i < volatileTileData.size(); i++)
		{
			dataBuffer.push_back(*volatileTileData[i]);
		}

		// Store vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, volatileTileDataBuffer);
		glBufferData(GL_ARRAY_BUFFER, dataBuffer.size() * sizeof(TileData), &dataBuffer[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void TileHandler::clearVolatileBuffers()
{
	glFinish();
	for (int i = 0; i < volatileTileData.size(); i++)
	{
		delete volatileTileData[i];
	}
	volatileTileData.clear();
}

void TileHandler::deleteSelectedTileCoordBased(glm::ivec2 corner1, glm::ivec2 corner2, Modification& recorder) //Tile modified
{
	int minX = std::min(corner1.x, corner2.x);
	int maxX = std::max(corner1.x, corner2.x);
	int minY = std::min(corner1.y, corner2.y);
	int maxY = std::max(corner1.y, corner2.y);

	glm::ivec2 topLeft = glm::ivec2(minX, minY);
	glm::ivec2 bottomRight = glm::ivec2(maxX, maxY);

	/*
	std::vector<TileData*>::iterator tileItr = tileDatas.begin();
	while(tileItr != tileDatas.end())
	{
		if (!((*tileItr)->tileCoordx < topLeft.x || (*tileItr)->tileCoordx > bottomRight.x ||
			  (*tileItr)->tileCoordy < topLeft.y || (*tileItr)->tileCoordy > bottomRight.y))
		{
			//Record the tile that gets changed
			if ((*tileItr)->type != EMPTY)
			{
				recorder.tiles.push_back(**tileItr);
			}

			delete (*tileItr);
			tileItr = tileDatas.erase(tileItr);
		}
		else
		{
			tileItr++;
		}
	}
	*/

	//Lazy delete method
	TileData t;
	t.type = EMPTY;
	t.rotation = TILE_DEGREE_0;
	for (int i = 0; i < tileDatas.size(); i++)
	{
		if (!(tileDatas[i]->tileCoordx < topLeft.x || tileDatas[i]->tileCoordx > bottomRight.x ||
			  tileDatas[i]->tileCoordy < topLeft.y || tileDatas[i]->tileCoordy > bottomRight.y))
		{
			t.tileCoordx = tileDatas[i]->tileCoordx;
			t.tileCoordy = tileDatas[i]->tileCoordy;
			addTile(t, recorder);
		}
	}

	setBuffers();
}

void TileHandler::flipTileHorizontally(TileData& tile, int xMinBoundary, int xMaxBoundary)
{
	tile.tileCoordx = xMaxBoundary - (tile.tileCoordx - xMinBoundary);

	switch (tile.type)
	{
		case HALF: case BORDER_TELEPORT:
		{
			if (tile.rotation == TILE_DEGREE_0) { tile.rotation = TILE_DEGREE_180; }
			else if (tile.rotation == TILE_DEGREE_180) { tile.rotation = TILE_DEGREE_0; }
			break;
		}
		default:
		{
			int baseRotation = tile.rotation / 2;
			int remainder = tile.rotation % 2;
			tile.rotation = baseRotation * 2 + (remainder + 1) % 2;
			break;
		}
	}
}

void TileHandler::flipTileVertically(TileData& tile, int yMinBoundary, int yMaxBoundary)
{
	tile.tileCoordy = yMaxBoundary - (tile.tileCoordy - yMinBoundary);

	switch (tile.type)
	{
		case HALF: case BORDER_TELEPORT:
		{
			if (tile.rotation == TILE_DEGREE_90) { tile.rotation = TILE_DEGREE_270; }
			else if (tile.rotation == TILE_DEGREE_270) { tile.rotation = TILE_DEGREE_90; }
			break;
		}
		default:
		{
			tile.rotation = 3 - tile.rotation;
			break;
		}
	}
}

void TileHandler::rotateTileClockwise(TileData& tile, const glm::ivec2& pivot)
{
	int dx = tile.tileCoordx - pivot.x;
	int dy = tile.tileCoordy - pivot.y;

	tile.tileCoordx = pivot.x + dy;
	tile.tileCoordy = pivot.y - dx;
	tile.rotation = (tile.rotation + 4 - 1) % 4;

	// Honestly I don't really understand the logic behind why the tiles are made like this but, it is what it is.
	switch (tile.type)
	{
		case SMALLSLOPE_RIGHT_60DEG:
			tile.type = SMALLSLOPE_LEFT_60DEG;
			break;
		case SMALLSLOPE_LEFT_60DEG:
			tile.type = SMALLSLOPE_RIGHT_60DEG;
			break;
		case LARGESLOPE_RIGHT_60DEG:
			tile.type = LARGESLOPE_LEFT_60DEG;
			break;
		case LARGESLOPE_LEFT_60DEG:
			tile.type = LARGESLOPE_RIGHT_60DEG;
			break;
		default:
			break;
	}
}

void TileHandler::rotateTileCounterClockwise(TileData& tile, const glm::ivec2& pivot)
{
	int dx = tile.tileCoordx - pivot.x;
	int dy = tile.tileCoordy - pivot.y;

	tile.tileCoordx = pivot.x - dy;
	tile.tileCoordy = pivot.y + dx;
	tile.rotation = (tile.rotation + 4 + 1) % 4;

	// Honestly I don't really understand the logic behind why the tiles are made like this but, it is what it is.
	switch (tile.type)
	{
		case SMALLSLOPE_RIGHT_60DEG:
			tile.type = SMALLSLOPE_LEFT_60DEG;
			break;
		case SMALLSLOPE_LEFT_60DEG:
			tile.type = SMALLSLOPE_RIGHT_60DEG;
			break;
		case LARGESLOPE_RIGHT_60DEG:
			tile.type = LARGESLOPE_LEFT_60DEG;
			break;
		case LARGESLOPE_LEFT_60DEG:
			tile.type = LARGESLOPE_RIGHT_60DEG;
			break;
		default:
			break;
	}
}

void TileHandler::invertTile(TileData& tile)
{
	switch (tile.type)
	{
		case SLOPE_45DEG: case HALF:
			tile.rotation = (tile.rotation + 2) % 4;
			break;
		case EMPTY:
			tile.type = FULL;
			break;
		case FULL:
			tile.type = EMPTY;
			break;
		case SMALLSLOPE_RIGHT_60DEG:
			tile.type = LARGESLOPE_RIGHT_60DEG;
			tile.rotation = (tile.rotation + 2) % 4;
			break;
		case SMALLSLOPE_LEFT_60DEG:
			tile.type = LARGESLOPE_LEFT_60DEG;
			tile.rotation = (tile.rotation + 2) % 4;
			break;
		case CURVE_IN:
			tile.type = CURVE_OUT;
			tile.rotation = (tile.rotation + 2) % 4;
			break;
		case LARGESLOPE_RIGHT_60DEG:
			tile.type = SMALLSLOPE_RIGHT_60DEG;
			tile.rotation = (tile.rotation + 2) % 4;
			break;
		case LARGESLOPE_LEFT_60DEG:
			tile.type = SMALLSLOPE_LEFT_60DEG;
			tile.rotation = (tile.rotation + 2) % 4;
			break;
		case CURVE_OUT:
			tile.type = CURVE_IN;
			tile.rotation = (tile.rotation + 2) % 4;
			break;
		default:
			break;
	}
}

bool TileHandler::isSameCoord(TileData data, int tileCoordx, int tileCoordy)
{
	return data.tileCoordx == tileCoordx && data.tileCoordy == tileCoordy;
}