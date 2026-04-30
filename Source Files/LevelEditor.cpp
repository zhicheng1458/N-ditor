#include "LevelEditor.h"
#include <iostream>

LevelEditor::LevelEditor()
{
	//Camera
	viewpoint = new Camera(1600.0f, 900.0f);

	gridShader = new ShaderProgram("./Resources/Shaders/Model.glsl", ShaderProgram::eRender);
	fineGridShader = gridShader;
	levelRegionShader = gridShader;
	grid = new Model();
	fineGrid = new Model();
	levelRegion = new Model();

	// Keyboard and Mouse control
	leftDown = middleDown = rightDown = false;
	mouseLeftButtonPressedX = mouseLeftButtonPressedY = mouseLeftButtonReleasedX = mouseLeftButtonReleasedY = 0.0;
	mouseLeftButtonPressedStartTime = mouseLeftButtonPressedEndTime = 0.0;
	MouseX = MouseY = 0.0f;

	init();
}

LevelEditor::LevelEditor(float screenWidth, float screenHeight)
{
	viewpoint = new Camera(screenWidth, screenHeight);
	viewpoint->update(glm::vec2((float)screenWidth * 0.5f, -(float)screenHeight * 0.5f));

	gridShader = new ShaderProgram("./Resources/Shaders/Model.glsl", ShaderProgram::eRender);
	fineGridShader = gridShader;
	levelRegionShader = gridShader;
	grid = new Model();
	fineGrid = new Model();
	levelRegion = new Model();

	// Keyboard and Mouse control
	leftDown = middleDown = rightDown = false;
	mouseLeftButtonPressedX = mouseLeftButtonPressedY = mouseLeftButtonReleasedX = mouseLeftButtonReleasedY = 0.0;
	mouseLeftButtonPressedStartTime = mouseLeftButtonPressedEndTime = 0.0;
	MouseX = MouseY = 0.0f;

	init();
}

void LevelEditor::init()
{
	grid->clearBuffers();
	fineGrid->clearBuffers();
	levelRegion->clearBuffers();

	modelMtx = glm::mat4(1.0f);

	glm::vec3 backgroundColor = palette.backgroundColors.backgroundColor / 255.0f;
	glm::vec3 gridColor = palette.editorColors.gridColor / 255.0f;
	glm::vec3 finegridColor = palette.editorColors.fineGridColor / 255.0f;

	glm::vec4 backgroundColor4f = glm::vec4(backgroundColor, 1.0f);
	glm::vec4 gridColor4f = glm::vec4(gridColor, 1.0f);
	glm::vec4 finegridColor4f = glm::vec4(finegridColor, 1.0f);

	//Grid
	glm::vec2 v1 = glm::vec2(-gridlineLength, -gridlineLength);
	glm::vec2 v2 = glm::vec2(-gridlineLength, gridlineLength);
	glm::vec2 v3 = glm::vec2(gridlineLength, -gridlineLength);
	glm::vec2 v4 = glm::vec2(gridlineLength, gridlineLength);
	grid->addTriangle(v1, v2, v3, backgroundColor4f, backgroundColor4f, backgroundColor4f, glm::vec2(-1.0f, -1.0f), glm::vec2(-1.0f, -1.0f), glm::vec2(-1.0f, -1.0f));
	grid->addTriangle(v2, v3, v4, backgroundColor4f, backgroundColor4f, backgroundColor4f, glm::vec2(-1.0f, -1.0f), glm::vec2(-1.0f, -1.0f), glm::vec2(-1.0f, -1.0f));
	for (float x = 0.0f - gridSpacing * 10.0f; x <= gridlineLength; x += gridSpacing)
	{
		grid->addLine(glm::vec2(x, -gridlineLength), gridColor4f, glm::vec2(x, gridlineLength), gridColor4f, gridlineThickness);
	}
	for (float y = 0.0f + gridSpacing * 10.0f; y >= -gridlineLength; y -= gridSpacing)
	{
		grid->addLine(glm::vec2(-gridlineLength, y), gridColor4f, glm::vec2(gridlineLength, y), gridColor4f, gridlineThickness);
	}
	grid->addLine(glm::vec2(-gridlineLength, 0.0f), gridColor4f, glm::vec2(gridlineLength, 0.0f), gridColor4f, gridlineThickness * 2);
	grid->addLine(glm::vec2(0.0f, -gridlineLength), gridColor4f, glm::vec2(0.0f, gridlineLength), gridColor4f, gridlineThickness * 2);
	grid->setBuffers();

	int repetition = 0;
	for (float x = 0.0f - gridSpacing * 10.0f; x <= gridlineLength; x += fineGridSpacing)
	{
		if (repetition % currentGridLevel != 0)
		{
			fineGrid->addLine(glm::vec2(x, -gridlineLength), finegridColor4f, glm::vec2(x, gridlineLength), finegridColor4f, fineGridlineThickness);
		}
		repetition++;
	}
	repetition = 0;
	for (float y = 0.0f + gridSpacing * 10.0f; y >= -gridlineLength; y -= fineGridSpacing)
	{
		if (repetition % currentGridLevel != 0)
		{
			fineGrid->addLine(glm::vec2(-gridlineLength, y), finegridColor4f, glm::vec2(gridlineLength, y), finegridColor4f, fineGridlineThickness);
		}
		repetition++;
	}
	fineGrid->setBuffers();

	//Level Region
	levelRegionModelMtx = glm::mat4(1.0f);
	v1 = glm::vec2(0.0f, 0.0f);
	v2 = glm::vec2(playableRegionLineWidth, 0.0f);
	v3 = glm::vec2(0.0f, -playableRegionLineHeight);
	v4 = glm::vec2(playableRegionLineWidth, -playableRegionLineHeight);
	glm::vec2 offsetv1horizontal = glm::vec2(-playableRegionLineThickness, playableRegionLineThickness * 0.5f);
	glm::vec2 offsetv1vertical = glm::vec2(-playableRegionLineThickness * 0.5f, playableRegionLineThickness);
	glm::vec2 offsetv2horizontal = glm::vec2(playableRegionLineThickness, playableRegionLineThickness * 0.5f);
	glm::vec2 offsetv2vertical = glm::vec2(playableRegionLineThickness * 0.5f, playableRegionLineThickness);
	glm::vec2 offsetv3horizontal = glm::vec2(-playableRegionLineThickness, -playableRegionLineThickness * 0.5f);
	glm::vec2 offsetv3vertical = glm::vec2(-playableRegionLineThickness * 0.5f, -playableRegionLineThickness);
	glm::vec2 offsetv4horizontal = glm::vec2(playableRegionLineThickness, -playableRegionLineThickness * 0.5f);
	glm::vec2 offsetv4vertical = glm::vec2(playableRegionLineThickness * 0.5f, -playableRegionLineThickness);
	levelRegion->addLine(v1 + offsetv1horizontal, playableRegionLineColor, v2 + offsetv2horizontal, playableRegionLineColor, playableRegionLineThickness);
	levelRegion->addLine(v1 + offsetv1vertical, playableRegionLineColor, v3 + offsetv3vertical, playableRegionLineColor, playableRegionLineThickness);
	levelRegion->addLine(v2 + offsetv2vertical, playableRegionLineColor, v4 + offsetv4vertical, playableRegionLineColor, playableRegionLineThickness);
	levelRegion->addLine(v3 + offsetv3horizontal, playableRegionLineColor, v4 + offsetv4horizontal, playableRegionLineColor, playableRegionLineThickness);

	v1 = glm::vec2(0.0f - gridSpacing, 0.0f + gridSpacing);
	v2 = glm::vec2(entityPlaceableRegionLineWidth - gridSpacing, 0.0f + gridSpacing);
	v3 = glm::vec2(0.0f - gridSpacing, -entityPlaceableRegionLineHeight + gridSpacing);
	v4 = glm::vec2(entityPlaceableRegionLineWidth - gridSpacing, -entityPlaceableRegionLineHeight + gridSpacing);
	offsetv1horizontal = glm::vec2(-entityPlaceableRegionLineThickness, entityPlaceableRegionLineThickness * 0.5f);
	offsetv1vertical = glm::vec2(-entityPlaceableRegionLineThickness * 0.5f, entityPlaceableRegionLineThickness);
	offsetv2horizontal = glm::vec2(entityPlaceableRegionLineThickness, entityPlaceableRegionLineThickness * 0.5f);
	offsetv2vertical = glm::vec2(entityPlaceableRegionLineThickness * 0.5f, entityPlaceableRegionLineThickness);
	offsetv3horizontal = glm::vec2(-entityPlaceableRegionLineThickness, -entityPlaceableRegionLineThickness * 0.5f);
	offsetv3vertical = glm::vec2(-entityPlaceableRegionLineThickness * 0.5f, -entityPlaceableRegionLineThickness);
	offsetv4horizontal = glm::vec2(entityPlaceableRegionLineThickness, -entityPlaceableRegionLineThickness * 0.5f);
	offsetv4vertical = glm::vec2(entityPlaceableRegionLineThickness * 0.5f, -entityPlaceableRegionLineThickness);
	levelRegion->addLine(v1 + offsetv1horizontal, entityPlaceableRegionLineColor, v2 + offsetv2horizontal, entityPlaceableRegionLineColor, entityPlaceableRegionLineThickness);
	levelRegion->addLine(v1 + offsetv1vertical, entityPlaceableRegionLineColor, v3 + offsetv3vertical, entityPlaceableRegionLineColor, entityPlaceableRegionLineThickness);
	levelRegion->addLine(v2 + offsetv2vertical, entityPlaceableRegionLineColor, v4 + offsetv4vertical, entityPlaceableRegionLineColor, entityPlaceableRegionLineThickness);
	levelRegion->addLine(v3 + offsetv3horizontal, entityPlaceableRegionLineColor, v4 + offsetv4horizontal, entityPlaceableRegionLineColor, entityPlaceableRegionLineThickness);
	levelRegion->setBuffers();
}

LevelEditor::~LevelEditor()
{
	delete viewpoint;

	delete grid;
	delete gridShader;
	delete fineGrid;
	fineGridShader = nullptr;
	delete levelRegion;
	levelRegionShader = nullptr;
}

void LevelEditor::loadPalette(const char* paletteFolderPath)
{
	//TODO

	//Psudo code:
	//1. Call loading method from palette, then run the buildPalette function
	//2. re-init level editor
	//3. Call usePalette for tiles, entities, and possibly overlay later
}

//////////////////////////////////////////////////////////////

void LevelEditor::update()
{
	
}

void LevelEditor::drawSolidLayer()
{
	grid->draw(modelMtx, viewpoint->getViewProjectMtx(), gridShader->getProgramID()); //Solid
	if (drawFineGrid) { fineGrid->draw(modelMtx, viewpoint->getViewProjectMtx(), fineGridShader->getProgramID()); } //Solid
	levelRegion->draw(levelRegionModelMtx, viewpoint->getViewProjectMtx(), levelRegionShader->getProgramID()); // Solid?
	tiles.draw(viewpoint->getViewProjectMtx()); //Transparent
	mouse.draw(viewpoint->getViewProjectMtx());
	//std::cout << glGetError() << std::endl;
	
}

void LevelEditor::drawTransparentLayer()
{
	entities.draw(viewpoint->getViewProjectMtx()); //Transparent
	//std::cout << glGetError() << std::endl;
}

void LevelEditor::drawOverlay()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	overlay.drawSolidObj();
	overlay.drawTransparentObj();
	//glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);
	//std::cout << glGetError() << std::endl;
}

//////////////////////////////////////////////////////////////

void LevelEditor::keyboard(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_UNKNOWN) { return; }

	if (action == GLFW_PRESS)
	{
		keyStates[key] = true;

		if (key == GLFW_KEY_SLASH) { drawFineGrid = !drawFineGrid; return; } //Toggle fine grid drawing.
		if (key == GLFW_KEY_ESCAPE)
		{
			currentEditingMode = TILE_EDITING_MODE;
			mouse.setCursorType(TILE_CURSOR);
			mouse.clearSelectionRegionBuffer();
			mouse.setFollowMouse(false);
			entities.hideHintEntity();
			entities.stopHighlight();
			return;
		}

		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glm::vec2 mouseCoordinate = UtilityFunctions::convertScreenCoordToModelCoord(
									glm::vec2((float)MouseX, (float)MouseY),
									(float)display_w, (float)display_h,
									viewpoint->getViewMtx(), viewpoint->getProjectionMtx(), levelRegionModelMtx);
		glm::ivec2 tileCoordinate = UtilityFunctions::clampToNearestTileCoord(mouseCoordinate, gridSpacing);
		glm::ivec2 newEntityCoordinate = UtilityFunctions::clampToNearestPlaceableEntityCoord(mouseCoordinate, gridSpacing);

		switch (currentEditingMode)
		{
			case ENTITY_EDITING_MODE:
				switch (key)
				{
					case GLFW_KEY_F: //Only difference between F and space key is that F key doesn't show the tray
						currentEditingMode = ENTITY_PLACEMENT_MODE;
						mouse.setCursorType(ENTITY_PLACEMENT_CURSOR);
						entities.stopHighlight();
						entities.showHintEntity();
						entities.setHintEntity(overlay.getSelectedEntityType(), (int)newEntityCoordinate.x, (int)newEntityCoordinate.y, currentEntityRotation);
						break;
					case GLFW_KEY_T:
						entities.deleteClosestEntity(newEntityCoordinate, MAX_HIGHLIGHT_SEARCH_RADIUS);
						break;
					case GLFW_KEY_D:
						currentEntityRotation = ENTITY_DEGREE_0;
						entities.setHighlightedEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_E:
						currentEntityRotation = ENTITY_DEGREE_45;
						entities.setHighlightedEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_W:
						currentEntityRotation = ENTITY_DEGREE_90;
						entities.setHighlightedEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_Q:
						currentEntityRotation = ENTITY_DEGREE_135;
						entities.setHighlightedEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_A:
						currentEntityRotation = ENTITY_DEGREE_180;
						entities.setHighlightedEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_Z:
						currentEntityRotation = ENTITY_DEGREE_225;
						entities.setHighlightedEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_S:
						currentEntityRotation = ENTITY_DEGREE_270;
						entities.setHighlightedEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_C:
						currentEntityRotation = ENTITY_DEGREE_315;
						entities.setHighlightedEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_SPACE:
						overlay.showTray();
						currentEditingMode = ENTITY_PLACEMENT_MODE;
						mouse.setCursorType(ENTITY_PLACEMENT_CURSOR);
						entities.stopHighlight();
						entities.showHintEntity();
						entities.setHintEntity(overlay.getSelectedEntityType(), (int)newEntityCoordinate.x, (int)newEntityCoordinate.y, currentEntityRotation);
						break;
					case GLFW_KEY_LEFT_ALT:
						currentEditingMode = TILE_EDITING_MODE;
						mouse.setCursorType(TILE_CURSOR);
						entities.hideHintEntity();
						entities.stopHighlight();
						break;
					default:
						//Do something when that key is pressed
						break;
				}
				break;
			case TILE_EDITING_MODE: //tile editing mode
				switch (key)
				{
					case GLFW_KEY_F:
						//currentEditingMode = ENTITY_EDITING_MODE;
						//entities.hideHintEntity();
						//lastClosestEntity = findClosestEntity(newEntityCoordinate, 5);
						//entities.resolveHighlight(lastClosestEntity, nullptr);
						break;
					case GLFW_KEY_1: case GLFW_KEY_2: case GLFW_KEY_3: case GLFW_KEY_4: case GLFW_KEY_5: case GLFW_KEY_6: case GLFW_KEY_7: case GLFW_KEY_8:
						this->setTileTypeKeyModifier(key);
						break;
					case GLFW_KEY_E:
					{
						TileData tile;

						tile.rotation = 0;
						tile.tileCoordx = tileCoordinate.x;
						tile.tileCoordy = tileCoordinate.y;
						tile.type = FULL;
						tiles.addTile(tile);

						break;
					}
					case GLFW_KEY_D:
					{
						TileData tile;

						tile.rotation = 0;
						tile.tileCoordx = tileCoordinate.x;
						tile.tileCoordy = tileCoordinate.y;
						tile.type = EMPTY;
						tiles.addTile(tile);

						break;
					}
					case GLFW_KEY_Q: case GLFW_KEY_W: case GLFW_KEY_S: case GLFW_KEY_A:
					{
						this->setTileRotationKeyModifier(key);
						TileData tile;

						tile.rotation = this->getTileRotationKeyModifier();
						tile.tileCoordx = tileCoordinate.x;
						tile.tileCoordy = tileCoordinate.y;
						tile.type = this->getTileTypeKeyModifier();
						tiles.addTile(tile);
						break;
					}
					case GLFW_KEY_SPACE:
						overlay.showTray();
						entities.showHintEntity();
						currentEditingMode = ENTITY_PLACEMENT_MODE;
						mouse.setCursorType(ENTITY_PLACEMENT_CURSOR);
						entities.setHintEntity(overlay.getSelectedEntityType(), newEntityCoordinate.x, newEntityCoordinate.y, currentEntityRotation);
						break;
					default:
						break;
				}
				break;
			case ENTITY_PLACEMENT_MODE:
				switch (key)
				{
					case GLFW_KEY_F:
						currentEditingMode = ENTITY_EDITING_MODE;
						mouse.setCursorType(ENTITY_CURSOR);
						entities.hideHintEntity();
						entities.highlightClosestEntity(newEntityCoordinate, MAX_HIGHLIGHT_SEARCH_RADIUS);
						break;
					case GLFW_KEY_SPACE:
						overlay.showTray();
						entities.stopHighlight();
						break;
					case GLFW_KEY_LEFT_ALT:
						currentEditingMode = TILE_EDITING_MODE;
						mouse.setCursorType(TILE_CURSOR);
						entities.hideHintEntity();
						break;
					case GLFW_KEY_D:
						currentEntityRotation = ENTITY_DEGREE_0;
						entities.setHintEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_E:
						currentEntityRotation = ENTITY_DEGREE_45;
						entities.setHintEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_W:
						currentEntityRotation = ENTITY_DEGREE_90;
						entities.setHintEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_Q:
						currentEntityRotation = ENTITY_DEGREE_135;
						entities.setHintEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_A:
						currentEntityRotation = ENTITY_DEGREE_180;
						entities.setHintEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_Z:
						currentEntityRotation = ENTITY_DEGREE_225;
						entities.setHintEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_S:
						currentEntityRotation = ENTITY_DEGREE_270;
						entities.setHintEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_C:
						currentEntityRotation = ENTITY_DEGREE_315;
						entities.setHintEntityRotation(currentEntityRotation);
						break;
					default:
						break;
				}
				break;
			case REGION_EDITING_MODE:
				switch (key)
				{
					case GLFW_KEY_LEFT_ALT:
						currentEditingMode = TILE_EDITING_MODE;
						mouse.setCursorType(TILE_CURSOR);
						mouse.setFollowMouse(false);
						mouse.clearSelectionRegionBuffer();
						break;
					case GLFW_KEY_C:
						if (!leftDown)
						{
							mouse.setFollowMouse(true);
						}
						break;
					case GLFW_KEY_X:
						if (!leftDown)
						{
							mouse.setFollowMouse(true);
						}
						break;
					case GLFW_KEY_Q:
						if (!leftDown)
						{
							mouse.rotateSelectionRegionClockwise();
						}
						break;
					case GLFW_KEY_E:
						if (!leftDown)
						{
							mouse.rotateSelectionRegionCounterClockwise();
						}
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		keyStates[key] = false;

		switch (key)
		{
			case GLFW_KEY_SPACE:
				overlay.hideTray();
				break;
			default:
			{
				//Do something when that key is pressed
				break;
			}
		}
	}
}

void LevelEditor::mouseButton(GLFWwindow * window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		/* This part should really be done with every mouse action */
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		MouseX = (float)mouseX;
		MouseY = (float)mouseY;
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glm::vec2 newCoord = UtilityFunctions::convertScreenCoordToModelCoord(
							 glm::vec2(MouseX, MouseY),
							 (float)display_w, (float)display_h,
							 viewpoint->getViewMtx(), viewpoint->getProjectionMtx(), levelRegionModelMtx);
		glm::ivec2 newTileCoordinate = UtilityFunctions::clampToNearestTileCoord(newCoord, gridSpacing);
		glm::ivec2 newEntityCoordinate = UtilityFunctions::clampToNearestPlaceableEntityCoord(newCoord, gridSpacing);

		if (action == GLFW_PRESS)
		{
			leftDown = true;
			mouseLeftButtonPressedX = mouseX;
			mouseLeftButtonPressedY = mouseY;
			mouseLeftButtonPressedStartTime = glfwGetTime();

			switch (currentEditingMode)
			{
			case ENTITY_PLACEMENT_MODE:
				entities.placeEntity();
				break;
			case TILE_EDITING_MODE:
				currentEditingMode = REGION_EDITING_MODE;
				mouse.setCursorType(REGION_SELECT_CURSOR);
				mouse.buildSelectionRegionBuffer(newCoord, newCoord, true, true);
				break;
			case REGION_EDITING_MODE:
				mouse.buildSelectionRegionBuffer(newCoord, newCoord, true, true);
				break;
			default:
				break;
			}

			//Do things when mouse left key is pressed down
		}
		else if (action == GLFW_RELEASE)
		{
			leftDown = false;
			mouseLeftButtonReleasedX = mouseX;
			mouseLeftButtonReleasedY = mouseY;
			mouseLeftButtonPressedEndTime = glfwGetTime();

			glm::vec2 oldCoord = UtilityFunctions::convertScreenCoordToModelCoord(
								 glm::vec2(mouseLeftButtonPressedX, mouseLeftButtonPressedY),
								 (float)display_w, (float)display_h,
								 viewpoint->getViewMtx(), viewpoint->getProjectionMtx(), levelRegionModelMtx);
			

			//Allow some click inaccuracy due to small drags
			if (std::pow(mouseLeftButtonReleasedX - mouseLeftButtonPressedX, 2) +
				std::pow(mouseLeftButtonReleasedY - mouseLeftButtonPressedY, 2) <
				5.0 * 5.0
				&& mouseLeftButtonPressedEndTime - mouseLeftButtonPressedStartTime <= 0.2)
			{
				//Do things when mouse left key is clicked
			}

			//Do things when mouse left key is released
			switch (currentEditingMode)
			{
			case ENTITY_PLACEMENT_MODE:
				break;
			case TILE_EDITING_MODE:
				break;
			case REGION_EDITING_MODE:
				mouse.buildSelectionRegionBuffer(oldCoord, newCoord, true, false);
				break;
			default:
				break;
			}
		}
		else
		{
			leftDown = false;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
		{
			rightDown = true;
		}
		else if (action == GLFW_RELEASE)
		{
			rightDown = false;
		}
		else
		{
			rightDown = false;
		}
	}
}

void LevelEditor::mouseMotion(GLFWwindow * window, double xpos, double ypos)
{
	float dx = (float)xpos - MouseX;
	float dy = -((float)ypos - MouseY); //Up and down are inverted

	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glm::vec2 newCoord = UtilityFunctions::convertScreenCoordToModelCoord(
						 glm::vec2((float)xpos, (float)ypos),
						 (float)display_w, (float)display_h,
						 viewpoint->getViewMtx(), viewpoint->getProjectionMtx(), levelRegionModelMtx);
	glm::ivec2 newTileCoordinate = UtilityFunctions::clampToNearestTileCoord(newCoord, gridSpacing);
	glm::ivec2 newEntityCoordinate = UtilityFunctions::clampToNearestPlaceableEntityCoord(newCoord, gridSpacing);

	if (overlay.wantControl((float)xpos, (float)ypos))
	{
		overlay.processMouseLocation((float)xpos, (float)ypos);
		entities.setHintEntity(overlay.getSelectedEntityType(), newEntityCoordinate.x, newEntityCoordinate.y, currentEntityRotation);
		return;
	}

	MouseX = (float)xpos;
	MouseY = (float)ypos;

	mouse.update(newCoord.x, newCoord.y);

	switch (currentEditingMode)
	{
		case ENTITY_PLACEMENT_MODE:
			entities.moveHint(newEntityCoordinate.x, newEntityCoordinate.y);
			break;
		case ENTITY_EDITING_MODE:
		{
			if (leftDown)
			{
				entities.moveHighlightedEntity(newEntityCoordinate.x, newEntityCoordinate.y);
			}
			else
			{
				if (oldEntityCoordinate != newEntityCoordinate)
				{
					entities.highlightClosestEntity(newEntityCoordinate, MAX_HIGHLIGHT_SEARCH_RADIUS);
				}
			}
			break;
		}
		case TILE_EDITING_MODE:
		{
			if (keyStates[GLFW_KEY_E])
			{
				if (oldTileCoordinate.x != newTileCoordinate.x || oldTileCoordinate.y != newTileCoordinate.y)
				{
					TileData fulltile;
					fulltile.rotation = TILE_DEGREE_0;
					fulltile.tileCoordx = newTileCoordinate.x;
					fulltile.tileCoordy = newTileCoordinate.y;
					fulltile.type = FULL;

					tiles.addTile(fulltile);
				}
			}
			else if (keyStates[GLFW_KEY_D])
			{
				if (oldTileCoordinate.x != newTileCoordinate.x || oldTileCoordinate.y != newTileCoordinate.y)
				{
					TileData emptyTile;
					emptyTile.rotation = TILE_DEGREE_0;
					emptyTile.tileCoordx = newTileCoordinate.x;
					emptyTile.tileCoordy = newTileCoordinate.y;
					emptyTile.type = EMPTY;

					tiles.addTile(emptyTile);
				}
			}
			else if (keyStates[GLFW_KEY_Q])
			{
				if (oldTileCoordinate.x != newTileCoordinate.x || oldTileCoordinate.y != newTileCoordinate.y)
				{
					TileData tile;
					tile.rotation = TILE_DEGREE_0;
					tile.tileCoordx = newTileCoordinate.x;
					tile.tileCoordy = newTileCoordinate.y;
					tile.type = this->getTileTypeKeyModifier();

					tiles.addTile(tile);
				}
			}
			else if (keyStates[GLFW_KEY_W])
			{
				if (oldTileCoordinate.x != newTileCoordinate.x || oldTileCoordinate.y != newTileCoordinate.y)
				{
					TileData tile;
					tile.rotation = TILE_DEGREE_90;
					tile.tileCoordx = newTileCoordinate.x;
					tile.tileCoordy = newTileCoordinate.y;
					tile.type = this->getTileTypeKeyModifier();

					tiles.addTile(tile);
				}
			}
			else if (keyStates[GLFW_KEY_S])
			{
				if (oldTileCoordinate.x != newTileCoordinate.x || oldTileCoordinate.y != newTileCoordinate.y)
				{
					TileData tile;
					tile.rotation = TILE_DEGREE_180;
					tile.tileCoordx = newTileCoordinate.x;
					tile.tileCoordy = newTileCoordinate.y;
					tile.type = this->getTileTypeKeyModifier();

					tiles.addTile(tile);
				}
			}
			else if (keyStates[GLFW_KEY_A])
			{
				if (oldTileCoordinate.x != newTileCoordinate.x || oldTileCoordinate.y != newTileCoordinate.y)
				{
					TileData tile;
					tile.rotation = TILE_DEGREE_270;
					tile.tileCoordx = newTileCoordinate.x;
					tile.tileCoordy = newTileCoordinate.y;
					tile.type = this->getTileTypeKeyModifier();

					tiles.addTile(tile);
				}
			}

			break;
		}
		case REGION_EDITING_MODE:
			if (leftDown)
			{
				glm::vec2 oldCoord = UtilityFunctions::convertScreenCoordToModelCoord(
					glm::vec2(mouseLeftButtonPressedX, mouseLeftButtonPressedY),
					(float)display_w, (float)display_h,
					viewpoint->getViewMtx(), viewpoint->getProjectionMtx(), levelRegionModelMtx);

				mouse.buildSelectionRegionBuffer(oldCoord, newCoord, true, true);
			}
			break;
		default:
			break;
	}

	if (rightDown)
	{
		// Move camera
		viewpoint->move(-dx, -dy);
	}

	if (oldTileCoordinate != newTileCoordinate) {oldTileCoordinate = newTileCoordinate;}
	if (oldEntityCoordinate != newEntityCoordinate) {oldEntityCoordinate = newEntityCoordinate;}
}

void LevelEditor::scrolling(GLFWwindow * window, double xoffset, double yoffset)
{
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	if (overlay.wantControl((float)mouseX, (float)mouseY))
	{
		overlay.scrollTray(-(float)yoffset);
	}
	else
	{
		float additiveMultiplier = -(float)yoffset * 0.1f;
		viewpoint->changeZoom(additiveMultiplier);
	}
}

void LevelEditor::resize(GLFWwindow * window, int width, int height)
{
	viewpoint->changeFOV((float)width, (float)height);
	overlay.resize((float)width, (float)height);
}

//////////////////////////////////////////////////////////////////////////

void LevelEditor::setTileTypeKeyModifier(int key)
{
	switch (key)
	{
	case GLFW_KEY_1:
		currentTileType = SLOPE_45DEG;
		break;
	case GLFW_KEY_2:
		currentTileType = SMALLSLOPE_RIGHT_60DEG;
		break;
	case GLFW_KEY_3:
		currentTileType = SMALLSLOPE_LEFT_60DEG;
		break;
	case GLFW_KEY_4:
		currentTileType = CURVE_IN;
		break;
	case GLFW_KEY_5:
		currentTileType = HALF;
		break;
	case GLFW_KEY_6:
		currentTileType = LARGESLOPE_RIGHT_60DEG;
		break;
	case GLFW_KEY_7:
		currentTileType = LARGESLOPE_LEFT_60DEG;
		break;
	case GLFW_KEY_8:
		currentTileType = CURVE_OUT;
		break;
	default:
		currentTileType = SLOPE_45DEG; //Default in editor is 45 deg tile
		break;
	}
}

TileType LevelEditor::getTileTypeKeyModifier()
{
	return currentTileType;
}

void LevelEditor::setTileRotationKeyModifier(int key)
{
	switch (key)
	{
	case GLFW_KEY_Q:
		currentTileRotation = TILE_DEGREE_0;
		break;
	case GLFW_KEY_W:
		currentTileRotation = TILE_DEGREE_90;
		break;
	case GLFW_KEY_S:
		currentTileRotation = TILE_DEGREE_180;
		break;
	case GLFW_KEY_A:
		currentTileRotation = TILE_DEGREE_270;
		break;
	default:
		currentTileRotation = TILE_DEGREE_0; //Default in editor is 45 deg tile
		break;
	}
}

TileRotation LevelEditor::getTileRotationKeyModifier()
{
	return currentTileRotation;
}

/** Return true if in entity editing mode.
 *  Return false if in tile eidting mode.
 */
const editingMode & LevelEditor::getEditingMode()
{
	return currentEditingMode;
}

const float LevelEditor::getGridSpacing()
{
	return gridSpacing;
}

