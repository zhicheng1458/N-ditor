#pragma once
#include "Core.h"

#include "LevelProperty.h"
#include "EntityHandler.h"
#include "TileHandler.h"

#include <string>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <filesystem>

#include "Message.h"

class LevelParser
{

public:
	LevelParser();
	~LevelParser();
	LevelParser(const LevelParser&) = delete;
	LevelParser& operator=(const LevelParser&) = delete;

	bool importLevel(std::string levelName, TileHandler& tileHandler, EntityHandler& entityHandler, LevelProperty& levelProperty);
	bool exportLevel(std::string levelName, const TileHandler& tileHandler, const EntityHandler& entityHandler, const LevelProperty& levelProperty);
	bool checkImportExportNameExist(std::string levelName);

	Message queryError();

private:
	const char* DIRECTORY_PATH = "./Levels/";
	const char* DEFAULT_LEVEL_NAME = "Untitled-1";

	const int HEX_MULTIPLIER = 256;
	const int MINIMUM_LEVEL_FILE_SIZE = 1230;
	const int LEVEL_LENGTH_INDEX = 4;
	const int LEVEL_NAME_INDEX = 38;
	const int TILE_DATA_INDEX = 184;
	const int NUM_TILES = 966;
	const int ENTITY_COUNT_INDEX = 1150;
	const int NUM_ENTITY_TYPE = 29;
	const int ENTITY_DATA_INDEX = 1230;
	const int NUM_BYTES_PER_ENTITY = 5;
	const int MAX_TILE_ID = 38;
	const int NUM_TILES_PER_ROW = 42;
	const int NUM_TILES_PER_COLUMN = 23; //AKA num row
	const int ENTITIES_MAX_X_BOUNDARY = 256;
	const int ENTITIES_MAX_Y_BOUNDARY = 256;
	const int NUM_ROTATIONS_PER_TILE_TYPE = 4;
	const int NUM_ROTATIONS_PER_ENTITY_TYPE = 8;

	const int MAGIC_NUMBER_FIELD_LENGTH = 4;
	const int MAX_LEVEL_NAME_LENGTH = 128;

	Message errorMessage;

	std::fstream fileReader;

	bool openInputFile(std::string levelName);
	bool openOutputFile(std::string levelName);
	bool closeFile();
	bool verifyFileSize();

	bool loadLevelName(LevelProperty& levelProperty);
	bool loadTiles(TileHandler& tileHandler);
	bool loadEntities(EntityHandler& entityHandler);

	std::vector<TileData> getIncludedTiles(const TileHandler& tileHandler) const;
	std::vector<EntityData> getIncludedSingleEntities(const EntityHandler& entityHandler) const;
	std::vector<PairEntityData> getIncludedPairEntities(const EntityHandler& entityHandler) const;
	bool writeMagicNumberField();											//That's what it is called in the file format, idk what to say. 4 Bytes
	bool writeFileLengthField(long int fileSize);							//4 bytes
	bool writeStaticDataField();											//25 Bytes, check eddy's message for layout.
	bool writeLevelNameField(std::string levelName);						//127 bytes + 18 null bytes
	bool writeTileDataField(const std::vector<TileData> & tileData);		//966 bytes, 0 fill if no tile exist
	bool writeObjectCountField(const std::vector<EntityData>& entityData,
						const std::vector<PairEntityData>& pairEntityData);	//80 bytes, 2 bytes per entity count
	bool writeEntityDataField(const std::vector<EntityData> & entityData,
						const std::vector<PairEntityData>& pairEntityData);	//Varying bytes based on number of entities existed within the boundary.


	bool getNextHexByteAsInt(int& v);
};