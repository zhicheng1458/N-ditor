#pragma once
#include "Core.h"
#include "SpriteData.h"
#include "Shader.h"
#include <string>

class Palette
{
public:
	const uint textureWidth = 400;
	const uint textureHeight = 400;
	const uint maximumWidthPerEntity = 50;

	Palette();
	~Palette();

	//Return true if building the entity texture is successful.
	bool buildPalette(int windowWidth, int windowHeight);

	const uint& getEntityTextureID() const;

	/* All color specified here are in the range of [0, 255] in rgb value. Default vaules are Vasquez palette values */
	struct BackgroundTGA
	{
		glm::vec3 tileColor = glm::vec3(79.0f, 86.0f, 77.0f);
		//glm::vec3 tileOutlineColor = glm::vec3(79.0f, 86.0f, 77.0f); //Don't have tile outline implemented, ignored for now
		glm::vec3 backgroundColor = glm::vec3(140.0f, 148.0f, 135.0f);
		//glm::vec3 entityOutlineColor = glm::vec3(140.0f, 148.0f, 135.0f); //Honestly not sure what it does
	};
	struct EditorTGA
	{
		glm::vec3 gridColor = glm::vec3(166.0f, 170.0f, 163.0f);
		glm::vec3 fineGridColor = glm::vec3(148.0f, 155.0f, 143.0f);
		glm::vec3 switchConnectorColor = glm::vec3(255.0f, 255.0f, 255.0f);
		glm::vec3 cursorColor = glm::vec3(255.0f, 255.0f, 255.0f); //Also used for copy and pasting rectangle
		glm::vec3 highlightColor = glm::vec3(194.0f, 206.0f, 177.0f); //For when tile are selected, entities uses its own highlight function
		glm::vec3 entityModeDisplayColor = glm::vec3(251.0f, 201.0f, 2.0f); //The arrow that indicator drone rotation etc
	};
	struct EntityTGA
	{
		glm::vec3 ninjaColor = glm::vec3(0.0f, 0.0f, 0.0f);

		glm::vec3 mineExteriorColor = glm::vec3(145.0f, 10.0f, 70.0f);
		glm::vec3 mineInteriorColor = glm::vec3(154.0f, 55.0f, 99.0f);

		glm::vec3 goldInteriorColor = glm::vec3(237.0f, 220.0f, 84.0f);
		glm::vec3 goldExteriorColor = glm::vec3(223.0f, 155.0f, 67.0f);
		glm::vec3 goldShineColor = glm::vec3(255.0f, 254.0f, 251.0f);

		glm::vec3 exitDoorClosedInteriorColor = glm::vec3(185.0f, 195.0f, 178.0f);
		glm::vec3 exitDoorClosedBorderColor = glm::vec3(50.0f, 51.0f, 35.0f);

		glm::vec3 exitSwitchClosedBorderColor = glm::vec3(50.0f, 51.0f, 35.0f);
		glm::vec3 exitSwitchCenterColor = glm::vec3(255.0f, 255.0f, 255.0f); //The little square in the middle
		glm::vec3 exitSwitchClosedBackgroundColor = glm::vec3(140.0f, 148.0f, 135.0f);

		glm::vec3 regularDoorColor = glm::vec3(194.0f, 206.0f, 177.0f);

		glm::vec3 lockedDoorBarColor = glm::vec3(50.0f, 51.0f, 35.0f);
		glm::vec3 lockedDoorCenterColor = glm::vec3(194.0f, 206.0f, 177.0f);

		glm::vec3 lockedDoorSwitchClosedCenterColor = glm::vec3(0.0f, 0.0f, 0.0f); //The little bar in the middle
		glm::vec3 lockedDoorSwitchClosedBorderColor = glm::vec3(50.0f, 51.0f, 35.0f);
		glm::vec3 lockedDoorSwitchClosedBackgroundColor = glm::vec3(140.0f, 148.0f, 135.0f);

		glm::vec3 trapDoorBarColor = glm::vec3(50.0f, 51.0f, 35.0f);
		glm::vec3 trapDoorCenterColor = glm::vec3(194.0f, 206.0f, 177.0f); //It is technically the two bigger parts

		glm::vec3 trapDoorSwitchClosedBorderColor = glm::vec3(50.0f, 51.0f, 35.0f);
		glm::vec3 trapDoorSwitchClosedBackgroundColor = glm::vec3(140.0f, 148.0f, 135.0f);

		glm::vec3 bouncePadBaseColor = glm::vec3(153.0f, 153.0f, 153.0f); //Bottom side of the bounce pad
		glm::vec3 bouncePadSpringColor = glm::vec3(0.0f, 0.0f, 0.0f); //The shorter end of the bounce pad

		glm::vec3 oneWayLongColor = glm::vec3(60.0f, 65.0f, 59.0f); //The color for the longer/solid side of the one way
		glm::vec3 oneWayShortColor = glm::vec3(109.0f, 118.0f, 104.0f);

		glm::vec3 chaingunDroneBackgroundColor = glm::vec3(112.0f, 112.0f, 112.0f);
		glm::vec3 chaingunDroneBorderColor = glm::vec3(0.0f, 0.0f, 0.0f);

		//glm::vec3 laserDroneBackgroundColor = glm::vec3(140.0f, 148.0f, 135.0f); //For the sake of the editor, the background is the same as the editor background
		glm::vec3 laserDroneBorderColor = glm::vec3(0.0f, 0.0f, 0.0f);

		glm::vec3 regularDroneBackgroundColor = glm::vec3(72.0f, 193.0f, 200.0f);
		glm::vec3 regularDroneBorderColor = glm::vec3(32.0f, 32.0f, 32.0f);

		glm::vec3 chaseDroneBackgroundColor = glm::vec3(72.0f, 193.0f, 200.0f);
		glm::vec3 chaseDroneBorderColor = glm::vec3(32.0f, 32.0f, 32.0f);

		glm::vec3 floorGuardMainColor = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 floorGuardEyeColor = glm::vec3(145.0f, 10.0f, 70.0f);

		glm::vec3 bounceBlockInteriorColor = glm::vec3(121.0f, 128.0f, 118.0f);
		glm::vec3 bounceBlockBorderColor = glm::vec3(0.0f);

		glm::vec3 rocketBorderColor = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 rocketCenterColor = glm::vec3(145.0f, 10.0f, 70.0f);

		glm::vec3 gaussBorderColor = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 gaussInteriorColor = glm::vec3(145.0f, 10.0f, 70.0f);
		//glm::vec3 gaussCenterColor = glm::vec3(0.0f); //AKA the bullet color

		glm::vec3 thwumpBorderColor = glm::vec3(77.0f, 77.0f, 77.0f);
		glm::vec3 thwumpInteriorColor = glm::vec3(121.0f, 128.0f, 118.0f);
		glm::vec3 thwumpRayColor = glm::vec3(72.0f, 193.0f, 200.0f); //The hurty side of thwump

		glm::vec3 toggleUntouchedColor = glm::vec3(107.0f, 115.0f, 103.0f);

		glm::vec3 evilNinjaColor = glm::vec3(0.0f, 0.0f, 0.0f);

		glm::vec3 laserTurretColor = glm::vec3(0.0f, 0.0f, 0.0f); //Not the actual laser color, just the turret's color

		glm::vec3 boosterColor = glm::vec3(0.0f, 0.0f, 0.0f); //Inactive color

		glm::vec3 deathballOuterColor = glm::vec3(145.0f, 0.0f, 164.0f);
		glm::vec3 deathballMiddleColor = glm::vec3(95.0f, 13.0f, 107.0f);
		glm::vec3 deathballInnerColor = glm::vec3(63.0f, 28.0f, 178.0f);

		glm::vec3 microDroneBackgroundColor = glm::vec3(72.0f, 193.0f, 200.0f); //Same as regular drone color
		glm::vec3 microDroneBorderColor = glm::vec3(32.0f, 32.0f, 32.0f); //Same as regular drone color

		glm::vec3 eyebatColor = glm::vec3(255.0f, 255.0f, 255.0f);

		glm::vec3 shoveThwumpExteriorColor = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 shoveThwumpRayColor = glm::vec3(72.0f, 193.0f, 200.0f); //The hurty part of the shove thwump
		glm::vec3 shoveThwumpInteriorColor = glm::vec3(79.0f, 86.0f, 77.0f);
	};

	BackgroundTGA backgroundColors;
	EditorTGA editorColors;
	EntityTGA entityColors;

private:

	uint entityTexture = 0;

	const std::string entitySpriteLocation = "./Resources/Textures/";
	std::vector<std::string> files = {
		"ninja.png",							"mineExterior.png",				"mineInterior.png",					"goldInterior.png",
		"goldExterior.png",						"goldShine.png",				"exitDoorClosedInterior.png",		"exitDoorClosedBorder.png",
		"exitSwitchClosedBorder.png",			"exitSwitchCenter.png",			"exitSwitchClosedBackground.png",	"regularDoor.png",
		"lockedDoorBar.png",					"lockedDoorCenter.png",			"lockedDoorSwitchClosedCenter.png",	"lockedDoorSwitchClosedBorder.png",
		"lockedDoorSwitchClosedBackground.png",	"trapDoorBar.png",				"trapDoorCenter.png",				"trapDoorSwitchClosedBorder.png",
		"trapDoorSwitchClosedBackground.png",	"bouncePadBase.png",			"bouncePadSpring.png",				"oneWayLong.png",
		"oneWayShort.png",						"chaingunDroneBackground.png",	"chaingunDroneBorder.png",			"laserDroneBorder.png",
		"regularDroneBackground.png",			"regularDroneBorder.png",		"chaseDroneBackground.png",			"chaseDroneBorder.png",
		"floorGuardMain.png",					"floorGuardEye.png",			"bounceBlockInterior.png",			"bounceBlockBorder.png",
		"rocketBorder.png",						"rocketCenter.png",				"gaussBorder.png",					"gaussInterior.png",
		"thwumpBorder.png",						"thwumpInterior.png",			"thwumpRay.png",					"toggleUntouched.png",
		"evilNinja.png",						"laserTurret.png",				"booster.png",						"deathballOuter.png",
		"deathballMiddle.png",					"deathballInner.png",			"microDroneBackground.png",			"microDroneBorder.png",
		"eyebat.png",							"shoveThwumpExterior.png",		"shoveThwumpRay.png",				"shoveThwumpInterior.png"};
	std::vector<uint> spriteTextureLocation = { 0,	1,	1,	2,
												2,	2,	3,	3,
												4,	4,	4,	5,
												6,	6,	7,	7,
												7,	8,	8,	9,
												9,	10,	10,	11,
												11,	12,	12,	13,
												14,	14,	15,	15,
												16,	16,	17,	17,
												18,	18,	19,	19,
												20,	20,	20,	21,
												22,	23,	24,	25,
												25,	25,	26,	26,
												27,	28,	28,	28};

	struct TextureMappingInfo
	{
		glm::vec2 vertexLoc;
		glm::vec2 textureLoc;
	};
	
	/* Helper functions */
	//Location are using 0 indexing -> (25, 25)
	glm::ivec2 getPlacementCoordinate(uint location, uint spriteWidth, uint spriteHeight, uint txtWidth, uint txtHeight, uint widthPerEntity);
	//Helper function to add color in to data before using glTexSubImage2D
	//Return true if the operation is successful.
	bool addSpriteToTexture(const std::string & filepath, const glm::vec3 & spriteColor, const uint & location,
							uint txtWidth, uint txtHeight, uint widthPerEntity);

	void addTriangle(const glm::vec2& v1, const glm::vec2& v2, const glm::vec2& v3,
					 const glm::vec2& t1, const glm::vec2& t2, const glm::vec2& t3,
					 std::vector<TextureMappingInfo>& data);
	void addSquare(const glm::vec2& topLeftVtx, const glm::vec2& bottomRightVtx,
				   const glm::vec2& topLeftTxtLoc, const glm::vec2& bottomRightTxtLoc,
				   std::vector<TextureMappingInfo>& data);
	void addEntityBuildingData(const uint entityLocOnDestinationTexture, const uint entityComponentLocOnSourceTexture,
							   const uint destinationTextureWidth, const uint destinationTextureHeight, const uint destinationSizePerEntity,
							   const uint sourceTextureWidth, const uint sourceTextureHeight, const uint sourceSizePerComponent,
							   std::vector<TextureMappingInfo>& data);
};