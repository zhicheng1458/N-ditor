#include "LevelParser.h"

LevelParser::LevelParser()
{
	fileReader.setf(std::ios::hex);
}

LevelParser::~LevelParser()
{
	if (fileReader.is_open())
	{
		fileReader.close();
	}
}

void LevelParser::loadLevel(std::string levelName, TileHandler& tileHandler, EntityHandler& entityHandler)
{
	if (!openInputFile(levelName))
	{
		return;
	}

	if (!verifyFileSize())
	{
		return;
	}

	loadTiles(tileHandler);
	loadEntities(entityHandler);

	closeFile();
}

void LevelParser::saveLevel(std::string levelName, const TileHandler& tileHandler, const EntityHandler& entityHandler)
{
	if (levelName.length() > MAX_LEVEL_NAME_LENGTH)
	{
		printf("Level name cannot be longer than 128 characters.");
		return;
	}

	/* Psudo code
	 * 1. Make a list of all tiles and entities that are within boundary (require sorting the values)
	 * 2. Calculate file size (MINIMUM_LEVEL_FILE_SIZE + 5 * number of entities)
	 * 3. Fill just the file size bytes from the header for now and skip everything else (use default values, need dummy value for title)
	 * 4. Fill in tile values (easy minus the part that i have go through the whole mess of mapping the value again)
	 * 5. 2 pass fill in for entities: first pass fills in number of entities (skip lock door switches and trap door switches)
	 * 6. second pass writes the entities value afterward (Rotation is different once again)
	 * 7. run file verification afterward
	 */

	const std::vector<TileData> includedTiles = getIncludedTiles(tileHandler);
	const std::vector<EntityData> includedEntity = getIncludedEntities(entityHandler);

	for (int i = 0; i < includedEntity.size(); i++)
	{
		if (includedEntity[i].type == entityType::NONE)
		{
			printf("Invisible entities detected, unable to save level.");
			return;
		}
	}

	long int expectedFileSize = MINIMUM_LEVEL_FILE_SIZE + (long int)includedEntity.size() * NUM_BYTES_PER_ENTITY;

	std::string lvlName = levelName;
	if (levelName.length() == 0)
	{
		lvlName = DEFAULT_LEVEL_NAME;
	}

	if (!openOutputFile(lvlName))
	{
		return;
	}

	writeMagicNumberField();					//That's what it is called in the file format, idk what to say. 4 Bytes
	writeFileLengthField(expectedFileSize);		//4 bytes
	writeStaticDataField();						//25 Bytes, check eddy's message for layout.
	writeLevelNameField(lvlName);				//127 bytes + 18 null bytes
	writeTileDataField(includedTiles);			//966 bytes, 0 fill if no tile exist
	writeObjectCountField(includedEntity);		//80 bytes, 2 bytes per entity count
	writeEntityDataField(includedEntity);		//Varying bytes based on number of entities existed within the boundary.

	fileReader.flush();
	if (!verifyFileSize())
	{
		printf("Level saving may be corrupted.");
	}

	closeFile();
}

bool LevelParser::openInputFile(std::string levelName)
{
	using namespace std;

	if (fileReader.is_open())
	{
		printf("Level file is currently opened by another file reader.\n");
		return false;
	}

	string filePath = DIRECTORY_PATH + levelName;
	struct stat buffer;
	bool exist = (stat(filePath.c_str(), &buffer) == 0);
	if (!exist)
	{
		printf("Level name not found.\n");
		return false;
	}

	fileReader.open(filePath.c_str(), ifstream::in | ifstream::binary);
	if (fileReader.fail())
	{
		printf("Unable to open level file.\n");
		return false;
	}

	return true;
}

bool LevelParser::openOutputFile(std::string levelName)
{
	using namespace std;

	if (fileReader.is_open())
	{
		printf("Level file is currently opened by another file reader.\n");
		return false;
	}

	string filePath = DIRECTORY_PATH + levelName;

	fileReader.open(filePath.c_str(), ifstream::out | ifstream::binary | ifstream::trunc);
	if (fileReader.fail())
	{
		printf("Unable to open level file.\n");
		return false;
	}

	return true;
}

bool LevelParser::closeFile()
{
	fileReader.close();
	if (fileReader.fail())
	{
		std::printf("Unable to close the current level file.\n");
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool LevelParser::verifyFileSize()
{
	if (!fileReader.is_open())
	{
		std::printf("No level file opened.\n");
		return false;
	}

	fileReader.seekg(0, std::ios::end);
	long int fileSize = fileReader.tellg();

	if (fileSize < MINIMUM_LEVEL_FILE_SIZE)
	{
		std::printf("Invalid level file: File is not big enough to be a valid level file.\n");
		return false;
	}

	fileReader.clear();
	fileReader.seekg(LEVEL_LENGTH_INDEX);
	long int b1 = fileReader.get();
	long int b2 = fileReader.get() * HEX_MULTIPLIER;
	long int b3 = fileReader.get() * HEX_MULTIPLIER * HEX_MULTIPLIER;
	//long int b4 = fileReader.get() * HEX_MULTIPLIER * HEX_MULTIPLIER * HEX_MULTIPLIER; //If the file is this big.... you might have a virus....
	long int total = b1 + b2 + b3;

	if (fileSize < total)
	{
		std::printf("Invalid level file: File size is less than the declared size in bytes by the level.");
		return false;
	}

	fileReader.seekg(ENTITY_COUNT_INDEX);
	long int totalEntities = 0;
	for (int i = 0; i < NUM_ENTITY_TYPE; i++)
	{
		if (i == 6 || i == 8) //Locked door and Trap Door field
		{
			i++;
			long int b1 = fileReader.get();
			long int b2 = fileReader.get() * HEX_MULTIPLIER;
			totalEntities += (b1 + b2) * 2;
			fileReader.ignore(2); //Skip the next 2 byte due to door not having a count
			continue;
		}
		else
		{
			long int b1 = fileReader.get();
			long int b2 = fileReader.get() * HEX_MULTIPLIER;
			totalEntities += b1 + b2;
		}
	}
	long int entityDataFieldLength = totalEntities * NUM_BYTES_PER_ENTITY;
	if (MINIMUM_LEVEL_FILE_SIZE + entityDataFieldLength > fileSize)
	{
		std::printf("Invalid level file: Not enough entity datas supplied.\n");
		return false;
	}

	fileReader.clear();
	fileReader.seekg(0);
	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool LevelParser::loadTiles(TileHandler& tileHandler)
{
	tileHandler.clearData();

	fileReader.clear();
	fileReader.seekg(TILE_DATA_INDEX);
	for (int i = 0; i < NUM_TILES; i++)
	{
		int tileID = fileReader.get();
		if (tileID >= 0 && tileID < MAX_TILE_ID)
		{
			TileData t;
			t.tileCoordx = i % NUM_TILES_PER_ROW;
			t.tileCoordy = i / NUM_TILES_PER_ROW;

			switch (tileID)
			{
				case 0:
					t.type = TileType::EMPTY;
					t.rotation = TileRotation::TILE_DEGREE_0;
					break;
				case 1:
					t.type = TileType::FULL;
					t.rotation = TileRotation::TILE_DEGREE_0;
					break;
				case 2: case 3: case 4: case 5:
					t.type = TileType::HALF;
					t.rotation = (tileID - 2 + 1) % NUM_ROTATIONS_PER_TILE_TYPE; //-2 from offset, +1 because actual file rotation arrangement....
					break;
				case 6: case 7: case 8: case 9:
					t.type = TileType::SLOPE_45DEG;
					t.rotation = (tileID - 2 + 2) % NUM_ROTATIONS_PER_TILE_TYPE; //-2 from offset, +2 because actual file rotation arrangement....
					break;
				case 10: case 11: case 12: case 13:
					t.type = TileType::CURVE_OUT;
					t.rotation = (tileID - 2 + 2) % NUM_ROTATIONS_PER_TILE_TYPE;
					break;
				case 14: case 15: case 16: case 17:
					t.type = TileType::CURVE_IN;
					t.rotation = (tileID - 2 + 2) % NUM_ROTATIONS_PER_TILE_TYPE;
					break;
				case 18: case 19: case 20: case 21:
					t.type = TileType::SMALLSLOPE_LEFT_60DEG;
					t.rotation = (tileID - 2 + 2) % NUM_ROTATIONS_PER_TILE_TYPE;
					break;
				case 22: case 23: case 24: case 25:
					t.type = TileType::LARGESLOPE_LEFT_60DEG;
					t.rotation = (tileID - 2 + 2) % NUM_ROTATIONS_PER_TILE_TYPE;
					break;
				case 26: case 27: case 28: case 29:
					t.type = TileType::SMALLSLOPE_RIGHT_60DEG;
					t.rotation = (tileID - 2 + 2) % NUM_ROTATIONS_PER_TILE_TYPE;
					break;
				case 30: case 31: case 32: case 33:
					t.type = TileType::LARGESLOPE_RIGHT_60DEG;
					t.rotation = (tileID - 2 + 2) % NUM_ROTATIONS_PER_TILE_TYPE;
					break;
				case 34: case 35: case 36: case 37:
					t.type = TileType::BORDER_TELEPORT;
					t.rotation = (tileID - 2 + 1) % NUM_ROTATIONS_PER_TILE_TYPE; //-2 from offset, +1 because actual file rotation arrangement....
					break;
				default:
					break;
			}

			t.color = glm::vec3(0.5f); //Dummy color for now, palette will take care of the rest

			tileHandler.addTile(t);
		}
		else
		{
			//TODO: Add warning that unknown tile id are in the file
		}
	}

	return true;
}

bool LevelParser::loadEntities(EntityHandler& entityHandler)
{
	std::vector<int> entityCount(NUM_ENTITY_TYPE);

	fileReader.seekg(ENTITY_COUNT_INDEX);
	for (int i = 0; i < NUM_ENTITY_TYPE; i++)
	{
		long int b1 = fileReader.get();
		long int b2 = fileReader.get() * HEX_MULTIPLIER;
		entityCount[i] = b1 + b2;
	}

	entityHandler.clearData();

	//TODO: Probably fix the rotation order since it is backward according to the level data.
	fileReader.seekg(ENTITY_DATA_INDEX);
	std::vector<EntityData> exitDoorData;
	std::vector<EntityData> exitSwitchData;
	for (int i = 0; i < NUM_ENTITY_TYPE; i++)
	{
		for (int j = 0; j < entityCount[i]; j++)
		{
			if (i == 3)
			{
				int typeByte = fileReader.get();
				int coordXByte = fileReader.get();
				int coordYByte = fileReader.get();
				int rotationByte = fileReader.get();
				int modeByte = fileReader.get();

				EntityData e;
				// if(validateType()) //TODO: Type checking if requested
				e.type = entityType::EXIT;
				e.entityCoordx = coordXByte;
				e.entityCoordy = coordYByte;
				e.rotation = (NUM_ROTATIONS_PER_ENTITY_TYPE - rotationByte) % NUM_ROTATIONS_PER_ENTITY_TYPE;
				e.mode = modeByte;
				
				exitDoorData.push_back(e);
			}
			else if (i == 4)
			{
				int typeByte = fileReader.get();
				int coordXByte = fileReader.get();
				int coordYByte = fileReader.get();
				int rotationByte = fileReader.get();
				int modeByte = fileReader.get();

				EntityData e;
				// if(validateType()) //TODO: Type checking if requested
				e.type = entityType::EXIT_SWITCH;
				e.entityCoordx = coordXByte;
				e.entityCoordy = coordYByte;
				e.rotation = (NUM_ROTATIONS_PER_ENTITY_TYPE - rotationByte) % NUM_ROTATIONS_PER_ENTITY_TYPE;
				e.mode = modeByte;

				exitSwitchData.push_back(e);
			}
			else if (i == 6 || i == 8) //Locked door and Trap Door field
			{

				int doorTypeByte = fileReader.get();
				int doorCoordXByte = fileReader.get();
				int doorCoordYByte = fileReader.get();
				int doorRotationByte = fileReader.get();
				int doorModeByte = fileReader.get();
				int switchTypeByte = fileReader.get();
				int switchCoordXByte = fileReader.get();
				int switchCoordYByte = fileReader.get();
				int switchRotationByte = fileReader.get();
				int switchModeByte = fileReader.get();

				EntityData doorEntity;
				EntityData switchEntity;

				// if(validateType()) //TODO: Type checking if requested
				doorEntity.type = (entityType)i;
				doorEntity.entityCoordx = doorCoordXByte;
				doorEntity.entityCoordy = doorCoordYByte;
				doorEntity.rotation = (NUM_ROTATIONS_PER_ENTITY_TYPE - doorRotationByte) % NUM_ROTATIONS_PER_ENTITY_TYPE;
				doorEntity.mode = doorModeByte;
				switchEntity.type = (entityType)(i+1);
				switchEntity.entityCoordx = switchCoordXByte;
				switchEntity.entityCoordy = switchCoordYByte;
				switchEntity.rotation = (NUM_ROTATIONS_PER_ENTITY_TYPE - switchRotationByte) % NUM_ROTATIONS_PER_ENTITY_TYPE;
				switchEntity.mode = switchModeByte;

				entityHandler.addStaticEntity(doorEntity, switchEntity);
			}
			else
			{
				int typeByte = fileReader.get();
				int coordXByte = fileReader.get();
				int coordYByte = fileReader.get();
				int rotationByte = fileReader.get();
				int modeByte = fileReader.get();

				EntityData e;
				// if(validateType()) //TODO: Type checking if requested
				e.type = (entityType)i;
				e.entityCoordx = coordXByte;
				e.entityCoordy = coordYByte;
				e.rotation = (NUM_ROTATIONS_PER_ENTITY_TYPE - rotationByte) % NUM_ROTATIONS_PER_ENTITY_TYPE;
				e.mode = modeByte;

				entityHandler.addStaticEntity(e);
			}
		}

		//Skip checking lock door switches and trap door switches
		if (i == 6 || i == 8)
		{
			i++;
		}
	}

	int minSize = exitDoorData.size() < exitSwitchData.size() ? exitDoorData.size() : exitSwitchData.size();
	for (int i = 0; i < minSize; i++)
	{
		entityHandler.addStaticEntity(exitDoorData[i], exitSwitchData[i]);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////

std::vector<TileData> LevelParser::getIncludedTiles(const TileHandler& tileHandler) const
{
	const std::vector<TileData*> & tileDatas = tileHandler.getTileData();
	std::vector<TileData> includedTiles;
	includedTiles.reserve(NUM_TILES);

	//TODO: take the level region into account when checking which tile and entities is within the boundary
	for (int i = 0; i < tileDatas.size(); i++)
	{
		if (tileDatas[i]->tileCoordx >= 0 && tileDatas[i]->tileCoordx < NUM_TILES_PER_ROW &&
			tileDatas[i]->tileCoordy >= 0 && tileDatas[i]->tileCoordy < NUM_TILES_PER_COLUMN)
		{
			includedTiles.push_back(*tileDatas[i]);
		}
	}

	//Coord sorting tiles for faster file writing
	std::sort(includedTiles.begin(), includedTiles.end(), [](TileData t1, TileData t2) {
		if (t1.tileCoordy == t2.tileCoordy)
		{
			return t1.tileCoordx < t2.tileCoordx;
		}
		else
		{
			return t1.tileCoordy < t2.tileCoordy;
		}
		});

	return includedTiles;
}

std::vector<EntityData> LevelParser::getIncludedEntities(const EntityHandler& entityHandler) const
{
	const std::vector<EntityData*>& entityDatas = entityHandler.getEntityData();
	const std::vector<EntityConnector>& entityConnectors = entityHandler.getEntityConnectorData(); //For getting pair entities

	std::vector<EntityData> includedEntity;

	for (int i = 0; i < entityConnectors.size(); i++)
	{
		if (entityConnectors[i].e1->entityCoordx >= 0 && entityConnectors[i].e1->entityCoordx < ENTITIES_MAX_X_BOUNDARY &&
			entityConnectors[i].e1->entityCoordy >= 0 && entityConnectors[i].e1->entityCoordy < ENTITIES_MAX_Y_BOUNDARY &&
			entityConnectors[i].e2->entityCoordx >= 0 && entityConnectors[i].e2->entityCoordx < ENTITIES_MAX_X_BOUNDARY &&
			entityConnectors[i].e2->entityCoordy >= 0 && entityConnectors[i].e2->entityCoordy < ENTITIES_MAX_Y_BOUNDARY)
		{
			includedEntity.push_back(*(entityConnectors[i].e1));
			includedEntity.push_back(*(entityConnectors[i].e2));
		}
	}

	for (int i = 0; i < entityDatas.size(); i++)
	{
		if (entityDatas[i]->pair == nullptr &&
			entityDatas[i]->entityCoordx >= 0 && entityDatas[i]->entityCoordx < ENTITIES_MAX_X_BOUNDARY &&
			entityDatas[i]->entityCoordy >= 0 && entityDatas[i]->entityCoordy < ENTITIES_MAX_Y_BOUNDARY)
		{
			includedEntity.push_back(*entityDatas[i]);
		}
	}

	//Type sorting entities for faster file writing
	std::sort(includedEntity.begin(), includedEntity.end(), [](EntityData e1, EntityData e2) {
		return (int)(e1.type) < (int)(e2.type);
		});

	return includedEntity;
}

bool LevelParser::writeMagicNumberField()
{
	for(int i = 0; i < MAGIC_NUMBER_FIELD_LENGTH; i++)
	{
		fileReader.put((unsigned char)0);
	}

	return true;
}

bool LevelParser::writeFileLengthField(long int fileSize)
{
	int current = fileSize;
	for (int i = 0; i < 3; i++)
	{
		int b = current % HEX_MULTIPLIER;
		fileReader.put((unsigned char)b);
		current = current / HEX_MULTIPLIER;
	}
	fileReader.put((unsigned char)0); //Last byte is put as 0 just in case of malicious value.

	return true;
}

bool LevelParser::writeStaticDataField()
{
	fileReader.put((unsigned char)255);		//Level ID: Default value is -1 for 4 bytes, aka 4 bytes of FF
	fileReader.put((unsigned char)255);
	fileReader.put((unsigned char)255);
	fileReader.put((unsigned char)255);

	fileReader.put((unsigned char)0);		//Game Mode byte: Default is 0, 4 bytes
	fileReader.put((unsigned char)0);		
	fileReader.put((unsigned char)0);
	fileReader.put((unsigned char)0);

	fileReader.put((unsigned char)37);		//Query Byte: Default to 37, 4 bytes
	fileReader.put((unsigned char)0);		
	fileReader.put((unsigned char)0);
	fileReader.put((unsigned char)0);

	fileReader.put((unsigned char)255);		//Author ID: Default is -1, 4 bytes
	fileReader.put((unsigned char)255);		
	fileReader.put((unsigned char)255);
	fileReader.put((unsigned char)255);

	fileReader.put((unsigned char)0);		//Fav count byte: Default to 0, 4 bytes
	fileReader.put((unsigned char)0);
	fileReader.put((unsigned char)0);
	fileReader.put((unsigned char)0);

	fileReader.put((unsigned char)0);		//Publish time: Default to 0, takes 10 bytes
	fileReader.put((unsigned char)0);
	fileReader.put((unsigned char)0);
	fileReader.put((unsigned char)0);
	fileReader.put((unsigned char)0);
	fileReader.put((unsigned char)0);
	fileReader.put((unsigned char)0);
	fileReader.put((unsigned char)0);
	fileReader.put((unsigned char)0);
	fileReader.put((unsigned char)0);

	return true;
}

bool LevelParser::writeLevelNameField(std::string levelName)
{
	int levelNameLength = levelName.length();
	fileReader.write(levelName.c_str(), std::min(levelNameLength, MAX_LEVEL_NAME_LENGTH));
	if (levelNameLength < MAX_LEVEL_NAME_LENGTH)
	{
		int fillCount = MAX_LEVEL_NAME_LENGTH - levelNameLength;
		for (int i = 0; i < fillCount; i++)
		{
			fileReader.put((unsigned char)0);
		}
	}

	//16 bytes of 0 for author name
	for (int i = 0; i < 16; i++)
	{
		fileReader.put((unsigned char)0);
	}

	//2 bytes of 0 for alignment
	for (int i = 0; i < 2; i++)
	{
		fileReader.put((unsigned char)0);
	}

	return true;
}

bool LevelParser::writeTileDataField(const std::vector<TileData>& tileData)
{
	if (tileData.size() == 0)
	{
		for (int i = 0; i < NUM_TILES; i++)
		{
			fileReader.put((unsigned char)0);
		}
		return true;
	}

	std::vector<TileData>::const_iterator itr = tileData.begin();
	//TODO: need to keep the level region box in mind in the future
	for (int r = 0; r < NUM_TILES_PER_COLUMN; r++)
	{
		for (int c = 0; c < NUM_TILES_PER_ROW; c++)
		{
			if (r == itr->tileCoordy && c == itr->tileCoordx)
			{
				switch (itr->type)
				{
				case TileType::EMPTY:
					fileReader.put((unsigned char)0);
					break;
				case TileType::FULL:
					fileReader.put((unsigned char)1);
					break;
				case TileType::HALF:
				{
					int base = 2;
					int rotationOffset = (itr->rotation + 3) % NUM_ROTATIONS_PER_TILE_TYPE;
					fileReader.put((unsigned char)(base + rotationOffset));
					break;
				}
				case TileType::SLOPE_45DEG:
				{
					int base = 6;
					int rotationOffset = (itr->rotation + 2) % NUM_ROTATIONS_PER_TILE_TYPE;
					fileReader.put((unsigned char)(base + rotationOffset));
					break;
				}
				case TileType::CURVE_OUT:
				{
					int base = 10;
					int rotationOffset = (itr->rotation + 2) % NUM_ROTATIONS_PER_TILE_TYPE;
					fileReader.put((unsigned char)(base + rotationOffset));
					break;
				}
				case TileType::CURVE_IN:
				{
					int base = 14;
					int rotationOffset = (itr->rotation + 2) % NUM_ROTATIONS_PER_TILE_TYPE;
					fileReader.put((unsigned char)(base + rotationOffset));
					break;
				}
				case TileType::SMALLSLOPE_LEFT_60DEG:
				{
					int base = 18;
					int rotationOffset = (itr->rotation + 2) % NUM_ROTATIONS_PER_TILE_TYPE;
					fileReader.put((unsigned char)(base + rotationOffset));
					break;
				}
				case TileType::LARGESLOPE_LEFT_60DEG:
				{
					int base = 22;
					int rotationOffset = (itr->rotation + 2) % NUM_ROTATIONS_PER_TILE_TYPE;
					fileReader.put((unsigned char)(base + rotationOffset));
					break;
				}
				case TileType::SMALLSLOPE_RIGHT_60DEG:
				{
					int base = 26;
					int rotationOffset = (itr->rotation + 2) % NUM_ROTATIONS_PER_TILE_TYPE;
					fileReader.put((unsigned char)(base + rotationOffset));
					break;
				}
				case TileType::LARGESLOPE_RIGHT_60DEG:
				{
					int base = 30;
					int rotationOffset = (itr->rotation + 2) % NUM_ROTATIONS_PER_TILE_TYPE;
					fileReader.put((unsigned char)(base + rotationOffset));
					break;
				}
				case TileType::BORDER_TELEPORT:
				{
					int base = 34;
					int rotationOffset = (itr->rotation + 3) % NUM_ROTATIONS_PER_TILE_TYPE;
					fileReader.put((unsigned char)(base + rotationOffset));
					break;
				}
				default: //Unknown value/invalid value
					fileReader.put((unsigned char)0);
					break;
				}

				itr++;
			}
			else
			{
				fileReader.put((unsigned char)0);
			}
		}
	}

	return true;
}

bool LevelParser::writeObjectCountField(const std::vector<EntityData>& entityData)
{
	std::vector<int> countVector(NUM_ENTITY_TYPE);
	for (int i = 0; i < countVector.size(); i++)
	{
		countVector[i] = 0;
	}

	for (int i = 0; i < entityData.size(); i++)
	{
		if (entityData[i].type >= 0 && entityData[i].type < NUM_ENTITY_TYPE)
		{
			countVector[entityData[i].type]++;
		}
		else
		{
			printf("Unknown entity type detected, cannot save level.");
			return false;
		}
	}

	for (int i = 0; i < countVector.size(); i++)
	{
		if (i == LOCKED_DOOR_SWITCH || i == TRAP_DOOR_SWITCH)
		{
			fileReader.put((unsigned char)0);
			fileReader.put((unsigned char)0);
		}
		else
		{
			int b1 = countVector[i] % HEX_MULTIPLIER;
			fileReader.put((unsigned char)b1);
			int b2 = (countVector[i] / HEX_MULTIPLIER) % HEX_MULTIPLIER;
			fileReader.put((unsigned char)b2);
		}
	}

	for (int i = 0; i < 11; i++) //After the 29 known type, there are 11 more that are unused.
	{
		fileReader.put((unsigned char)0);
		fileReader.put((unsigned char)0);
	}

	return true;
}

bool LevelParser::writeEntityDataField(const std::vector<EntityData>& entityData)
{
	for (int i = 0; i < entityData.size(); i++)
	{
		if (entityData[i].type == LOCKED_DOOR_SWITCH or entityData[i].type == TRAP_DOOR_SWITCH) { continue; }

		if (entityData[i].type == LOCKED_DOOR or entityData[i].type == TRAP_DOOR)
		{
			fileReader.put((unsigned char)(entityData[i].type));
			fileReader.put((unsigned char)(entityData[i].entityCoordx));
			fileReader.put((unsigned char)(entityData[i].entityCoordy));
			int doorRotation = (NUM_ROTATIONS_PER_ENTITY_TYPE - entityData[i].rotation) % NUM_ROTATIONS_PER_ENTITY_TYPE;
			fileReader.put((unsigned char)doorRotation);
			fileReader.put((unsigned char)(entityData[i].mode));

			fileReader.put((unsigned char)(entityData[i].pair->type));
			fileReader.put((unsigned char)(entityData[i].pair->entityCoordx));
			fileReader.put((unsigned char)(entityData[i].pair->entityCoordy));
			int switchRotation = (NUM_ROTATIONS_PER_ENTITY_TYPE - entityData[i].pair->rotation) % NUM_ROTATIONS_PER_ENTITY_TYPE;
			fileReader.put((unsigned char)switchRotation);
			fileReader.put((unsigned char)(entityData[i].pair->mode));
		}
		else
		{
			fileReader.put((unsigned char)(entityData[i].type));
			fileReader.put((unsigned char)(entityData[i].entityCoordx));
			fileReader.put((unsigned char)(entityData[i].entityCoordy));
			int rotation = (NUM_ROTATIONS_PER_ENTITY_TYPE - entityData[i].rotation) % NUM_ROTATIONS_PER_ENTITY_TYPE;
			fileReader.put((unsigned char)rotation);
			fileReader.put((unsigned char)(entityData[i].mode));
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////

bool LevelParser::getNextHexByteAsInt(int & v)
{
	v = fileReader.get();
	if (v < 0 || v >= 256)
	{
		return false;
	}

	return true;
}
