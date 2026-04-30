#pragma once
#include "Core.h"

namespace UtilityFunctions
{
	glm::vec2 convertScreenCoordToModelCoord(glm::vec2 screenCoordinate, float windowWidth, float windowHeight, glm::mat4 viewMtx, glm::mat4 projMtx, glm::mat4 modelMtx);
	glm::ivec2 clampToNearestPlaceableEntityCoord(glm::vec2 clickedCoordinateInModelSpace, float unitSpacing);
	glm::ivec2 clampToNearestTileCoord(glm::vec2 clickedCoordinateInModelSpace, float unitSpacing);
	glm::vec2 convertEntityCoordToModelCoord(glm::ivec2 entityCoord, float unitSpacing); //Not quite right because it didn't take the level region thing into consideration
	glm::vec2 convertTileCoordToModelCoord(glm::ivec2 tileCoord, float unitSpacing); //Not quite right because it didn't take the level region thing into consideration
}
