#include "UtilityFunctions.h"

glm::vec2 UtilityFunctions::convertScreenCoordToModelCoord(glm::vec2 screenCoordinate, float windowWidth, float windowHeight, glm::mat4 viewMtx, glm::mat4 projMtx, glm::mat4 modelMtx)
{
	//Screen coordinate uses positive x -> right, and positive y = down, top left corner is 0
	//Because screen is a texture (i.e. the screen model defined the 4 corners of the triangle at (-1, -1), (-1, 1), (1, -1), (1, 1))
	//You must first scale the screen coordinate down to between -1 and 1
	//Ex. Top left corner has (0, 0) as screen coordinate, expected result is (-1, -1) as homogenous coordinate;
	//Rendering uses positive x -> right and positive y = up,

	//Model matrix was assumed to be identity matrix

	glm::vec2 normalizedScreenCoord = glm::vec2(2 * screenCoordinate.x / windowWidth - 1.0f, 1.0f - (2.0f * screenCoordinate.y / windowHeight));
	glm::vec4 screenCoord = glm::vec4(normalizedScreenCoord.x, normalizedScreenCoord.y, 0.0f, 1.0f);
	//glm::vec4 coordInCameraSpace = glm::inverse(projMtx) * screenCoord;
	//glm::vec4 coordInWorldSpace = glm::inverse(viewMtx) * coordInCameraSpace;
	//glm::vec4 coordInModelSpace = glm::inverse(levelRegionModelMtx) * coordInWorldSpace; //We are specifically using the level region model as reference
	glm::vec4 coordInModelSpace = glm::inverse(projMtx * viewMtx * modelMtx) * screenCoord; //(ABC)^-1 = C^-1 * B^-1 * A^-1

	return glm::vec2(coordInModelSpace.x, -coordInModelSpace.y);
}

glm::ivec2 UtilityFunctions::clampToNearestPlaceableEntityCoord(glm::vec2 clickedCoordinateInModelSpace, float unitSpacing)
{
	glm::vec2 result = glm::floor((clickedCoordinateInModelSpace / unitSpacing) * 4.0f) + glm::vec2(4.0f);
	return glm::ivec2((int)result.x, (int)result.y); //Entities can be placed 1 tile higher/lefter
}

glm::ivec2 UtilityFunctions::clampToNearestTileCoord(glm::vec2 clickedCoordinateInModelSpace, float unitSpacing)
{
	glm::vec2 result = glm::floor(clickedCoordinateInModelSpace / unitSpacing);
	return glm::ivec2((int)result.x, (int)result.y);
}

glm::vec2 UtilityFunctions::convertEntityCoordToModelCoord(glm::ivec2 entityCoord, float unitSpacing)
{
	float x = entityCoord.x * (unitSpacing * 0.25f) - unitSpacing;
	float y = entityCoord.y * (-unitSpacing * 0.25f) + unitSpacing;
	return glm::vec2(x, y);
}

/* Return the model coordinate for the center of the tile */
glm::vec2 UtilityFunctions::convertTileCoordToModelCoord(glm::ivec2 tileCoord, float unitSpacing)
{
	float x = tileCoord.x * unitSpacing + 0.5f * unitSpacing;
	float y = tileCoord.y * (-unitSpacing) - 0.5f * unitSpacing;
	return glm::vec2(x, y);
}
