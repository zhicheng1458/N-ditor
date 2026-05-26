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
	//std::cout << glGetError() << std::endl;
	
}

void LevelEditor::drawTransparentLayer()
{
	mouse.draw(viewpoint->getViewProjectMtx());
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
		//keyStates[key] = true;

		if (key == GLFW_KEY_ESCAPE && !leftDown) { this->resetStates(); return; } //Resetting editor state
		if (key == GLFW_KEY_SLASH) { drawFineGrid = !drawFineGrid; return; } //Toggle fine grid drawing.

		//Temporary level importing and exporting
		if (key == GLFW_KEY_L)
		{
			if (!leftDown)
			{
				this->resetStates();
				recorder.reset(); //Recorder will stop remembering all actions when loading a new level.
				levelParser.importLevel("Untitled-1", tiles, entities);
			}
			return;
		}

		if (key == GLFW_KEY_P)
		{
			if (!leftDown)
			{
				this->resetStates();
				levelParser.exportLevel("Untitled-1", tiles, entities);
			}
			return;
		}

		//Undo and redo
		if (mods == GLFW_MOD_CONTROL && key == GLFW_KEY_Z)
		{
			Modification changes;
			if (recorder.undo(changes))
			{
				tiles.undo(changes);
				entities.undo(changes);
			}
			return;
		}

		if (mods == GLFW_MOD_CONTROL && key == GLFW_KEY_X)
		{
			Modification changes;
			if (recorder.redo(changes))
			{
				tiles.redo(changes);
				entities.redo(changes);
			}
			return;
		}

		//Require knowing mouse position beyond this point
		glm::vec2 lastKnownMouseCoordinate = calculateMouseModelCoord(window, MouseX, MouseY);
		glm::ivec2 currentTileCoordinate = UtilityFunctions::clampToNearestTileCoord(lastKnownMouseCoordinate, gridSpacing);
		glm::ivec2 currentEntityCoordinate = UtilityFunctions::clampToNearestPlaceableEntityCoord(lastKnownMouseCoordinate, gridSpacing);

		switch (currentEditingMode)
		{
			case ENTITY_EDITING_MODE:
				switch (key)
				{
					case GLFW_KEY_F: //Only difference between F and space key is that F key doesn't show the tray
					{
						currentEditingMode = ENTITY_PLACEMENT_MODE;
						mouse.setCursorType(ENTITY_PLACEMENT_CURSOR);
						mouse.setHintEntityType(overlay.getSelectedEntityType());
						mouse.setHintEntityRotation(currentEntityRotation);
						mouse.update(lastKnownMouseCoordinate.x, lastKnownMouseCoordinate.y);
						entities.stopHighlight();
						break;
					}
					case GLFW_KEY_T:
					{
						Modification action;
						entities.deleteClosestEntity(currentEntityCoordinate, MAX_HIGHLIGHT_SEARCH_RADIUS, action);
						if (recorder.checkHasChanges(action))
						{
							recorder.newAction(action);
						}
						break;
					}
					case GLFW_KEY_D: case GLFW_KEY_E: case GLFW_KEY_W: case GLFW_KEY_Q: case GLFW_KEY_A: case GLFW_KEY_Z: case GLFW_KEY_S: case GLFW_KEY_C:
					{
						currentEntityRotation = getEntityRotationByKey(key);
						Modification changeRotation;
						entities.setHighlightedEntityRotation(currentEntityRotation, changeRotation);
						if (recorder.checkHasChanges(changeRotation))
						{
							recorder.newAction(changeRotation);
						}
						mouse.setHintEntityRotation(currentEntityRotation);
						break;
					}
					case GLFW_KEY_1: case GLFW_KEY_2: case GLFW_KEY_3: case GLFW_KEY_4:
					{
						currentEntityMode = getEntityModeByKey(key);
						Modification changeMode;
						entities.setHighlightedEntityMode(currentEntityMode, changeMode);
						if (recorder.checkHasChanges(changeMode))
						{
							recorder.newAction(changeMode);
						}
						mouse.setHintEntityMode(currentEntityMode);
						break;
					}
					case GLFW_KEY_SPACE:
					{
						overlay.showTray();
						currentEditingMode = ENTITY_PLACEMENT_MODE;
						mouse.setCursorType(ENTITY_PLACEMENT_CURSOR);
						mouse.setHintEntityType(overlay.getSelectedEntityType());
						mouse.setHintEntityRotation(currentEntityRotation);
						mouse.update(lastKnownMouseCoordinate.x, lastKnownMouseCoordinate.y);
						entities.stopHighlight();
						break;
					}
					case GLFW_KEY_LEFT_ALT:
					{
						currentEditingMode = TILE_EDITING_MODE;
						mouse.setCursorType(TILE_CURSOR);
						entities.stopHighlight();
						break;
					}
					default:
						//Do something when that key is pressed
						break;
				}
				break;
			case TILE_EDITING_MODE: //tile editing mode
				if (keyStates[GLFW_KEY_E] || keyStates[GLFW_KEY_D] || keyStates[GLFW_KEY_Q] ||
					keyStates[GLFW_KEY_W] || keyStates[GLFW_KEY_S] || keyStates[GLFW_KEY_A])
				{
					//No additional key may be invoked while mouse dragging tile addition/deletion is in progress
					break;
				}
				switch (key)
				{
					case GLFW_KEY_F:
						//Pressing F in tile editing mode doesn't actually switch to entity editing mode
						//currentEditingMode = ENTITY_EDITING_MODE;
						//lastClosestEntity = findClosestEntity(newEntityCoordinate, 5);
						//entities.resolveHighlight(lastClosestEntity, nullptr);
						break;
					case GLFW_KEY_1: case GLFW_KEY_2: case GLFW_KEY_3: case GLFW_KEY_4: case GLFW_KEY_5: case GLFW_KEY_6: case GLFW_KEY_7: case GLFW_KEY_8: case GLFW_KEY_9:
						currentTileType = getTileTypeKeyModifier(key);
						break;
					case GLFW_KEY_E:
					{
						if (hasRegionSelected)
						{
							tiles.fillSelected(tileChanges);
							if (recorder.checkHasChanges(tileChanges))
							{
								recorder.newAction(tileChanges);
								tileChanges.oldTiles.clear();
								tileChanges.newTiles.clear();
							}
						}
						else
						{
							tiles.addTile(currentTileCoordinate.x, currentTileCoordinate.y, TILE_DEGREE_0, FULL, tileChanges);
							//Do not record changes yet as mouse drag tile creation may occur.
						}

						break;
					}
					case GLFW_KEY_D:
					{
						//tileChanges.action = ACTION_DELETE;
						if (hasRegionSelected)
						{
							tiles.deleteSelected(tileChanges);
							if (recorder.checkHasChanges(tileChanges))
							{
								recorder.newAction(tileChanges);
								tileChanges.oldTiles.clear();
								tileChanges.newTiles.clear();
							}
						}
						else
						{
							tiles.addTile(currentTileCoordinate.x, currentTileCoordinate.y, TILE_DEGREE_0, EMPTY, tileChanges);
							//Do not record changes yet as mouse drag tile creation may occur.
						}
						break;
					}
					case GLFW_KEY_Q: case GLFW_KEY_W: case GLFW_KEY_S: case GLFW_KEY_A:
					{
						if (!hasRegionSelected) //If there is a selected region, no tile may be placed individually
						{
							currentTileRotation = getTileRotationKeyModifier(key);
							tiles.addTile(currentTileCoordinate.x, currentTileCoordinate.y, currentTileRotation, currentTileType, tileChanges);
						}
						break;
					}
					case GLFW_KEY_C:
					{
						if (!leftDown)
						{
							currentEditingMode = REGION_EDITING_MODE;
							//glm::vec2 corner1 = calculateMouseModelCoord(window, mouseLeftButtonPressedX, mouseLeftButtonPressedY);
							//glm::vec2 corner2 = calculateMouseModelCoord(window, mouseLeftButtonReleasedX, mouseLeftButtonReleasedY);

							mouse.setFollowMouse(true);
							entities.copySelected();
							entities.setHintToFollowMouse(true);
							entities.moveHint(lastKnownMouseCoordinate);
							tiles.copySelected();
							tiles.setHintToFollowMouse(true);
							tiles.moveHint(lastKnownMouseCoordinate);
						}
						break;
					}
					case GLFW_KEY_X:
					{
						Modification cut;
						if (!leftDown)
						{
							currentEditingMode = REGION_EDITING_MODE;
							//glm::vec2 corner1 = calculateMouseModelCoord(window, mouseLeftButtonPressedX, mouseLeftButtonPressedY);
							//glm::vec2 corner2 = calculateMouseModelCoord(window, mouseLeftButtonReleasedX, mouseLeftButtonReleasedY);

							mouse.setFollowMouse(true);
							entities.cutSelected(cut);
							entities.setHintToFollowMouse(true);
							entities.moveHint(lastKnownMouseCoordinate);
							tiles.cutSelected(cut);
							tiles.setHintToFollowMouse(true);
							tiles.moveHint(lastKnownMouseCoordinate);
						}

						if (recorder.checkHasChanges(cut))
						{
							recorder.newAction(cut);
						}
						break;
					}
					case GLFW_KEY_SPACE:
					{
						overlay.showTray();
						currentEditingMode = ENTITY_PLACEMENT_MODE;
						mouse.setCursorType(ENTITY_PLACEMENT_CURSOR);
						mouse.setHintEntityType(overlay.getSelectedEntityType());
						mouse.setHintEntityRotation(currentEntityRotation);
						mouse.update(lastKnownMouseCoordinate.x, lastKnownMouseCoordinate.y);
						break;
					}
					case GLFW_KEY_LEFT_ALT:
					{
						mouse.setCursorType(TILE_CURSOR);
						mouse.clearSelectionRegionBuffer();
						hasRegionSelected = false;
						break;
					}
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
						mouse.resetHintEntity();
						entities.highlightClosestEntity(currentEntityCoordinate, MAX_HIGHLIGHT_SEARCH_RADIUS);
						break;
					case GLFW_KEY_SPACE:
						overlay.showTray();
						entities.stopHighlight();
						break;
					case GLFW_KEY_LEFT_ALT:
						currentEditingMode = TILE_EDITING_MODE;
						mouse.setCursorType(TILE_CURSOR);
						mouse.resetHintEntity();
						break;
					case GLFW_KEY_D: case GLFW_KEY_E: case GLFW_KEY_W: case GLFW_KEY_Q: case GLFW_KEY_A: case GLFW_KEY_Z: case GLFW_KEY_S: case GLFW_KEY_C:
						currentEntityRotation = getEntityRotationByKey(key);
						mouse.setHintEntityRotation(currentEntityRotation);
						break;
					case GLFW_KEY_1: case GLFW_KEY_2: case GLFW_KEY_3: case GLFW_KEY_4:
						currentEntityMode = getEntityModeByKey(key);
						mouse.setHintEntityMode(currentEntityMode);
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
						entities.unstageSelected();
						entities.setHintToFollowMouse(false);
						tiles.unstageSelected();
						tiles.setHintToFollowMouse(false);
						hasRegionSelected = false;
						break;
					case GLFW_KEY_Q:
						if (!leftDown && hasRegionSelected)
						{
							mouse.rotateSelectionRegionClockwise();
							entities.rotateSelectedClockwise();
							tiles.rotateSelectedClockwise();
						}
						break;
					case GLFW_KEY_E:
						if (!leftDown && hasRegionSelected)
						{
							mouse.rotateSelectionRegionCounterClockwise();
							entities.rotateSelectedCounterClockwise();
							tiles.rotateSelectedCounterClockwise();
						}
						break;
					case GLFW_KEY_A: case GLFW_KEY_D:
						if (!leftDown && hasRegionSelected)
						{
							tiles.flipSelectedHorizontally();
							entities.flipSelectedHorizontally();
						}
						break;
					case GLFW_KEY_W: case GLFW_KEY_S:
						if (!leftDown && hasRegionSelected)
						{
							tiles.flipSelectedVertically();
							entities.flipSelectedVertically();
						}
					case GLFW_KEY_R:
						if (!leftDown && hasRegionSelected)
						{
							tiles.invertSelected();
						}
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}

		keyStates[key] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		keyStates[key] = false;

		if (key == GLFW_KEY_SPACE) { overlay.hideTray(); }

		switch (currentEditingMode)
		{
			case TILE_EDITING_MODE:
			{
				switch (key)
				{
					case GLFW_KEY_Q: case GLFW_KEY_W: case GLFW_KEY_S: case GLFW_KEY_A: case GLFW_KEY_E: case GLFW_KEY_D:
					{
						if (!tileChanges.newTiles.empty())
						{
							recorder.newAction(tileChanges);
						}
						tileChanges.oldTiles.clear();
						tileChanges.newTiles.clear();
						break;
					}
					default:
						break;
				}
				break;
			}
			default:
				break;
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

		glm::vec2 currentMouseModelCoord = this->calculateMouseModelCoord(window, mouseX, mouseY);
		glm::ivec2 currentTileCoordinate = UtilityFunctions::clampToNearestTileCoord(currentMouseModelCoord, gridSpacing);
		glm::ivec2 currentEntityCoordinate = UtilityFunctions::clampToNearestPlaceableEntityCoord(currentMouseModelCoord, gridSpacing);

		if (action == GLFW_PRESS)
		{
			leftDown = true;
			mouseLeftButtonPressedX = mouseX;
			mouseLeftButtonPressedY = mouseY;
			mouseLeftButtonPressedStartTime = glfwGetTime();

			switch (currentEditingMode)
			{
				case ENTITY_EDITING_MODE:
				{
					entities.pickupHighlightedEntity(moveEntity);
					break;
				}
				case ENTITY_PLACEMENT_MODE:
				{
					NumEntityToPlace num = mouse.placeEntity();
					Modification addEntity;
					if (num == SINGLE_ENTITY)
					{
						entities.addStaticEntity(mouse.getFirstEntityData(), addEntity);
					}
					else if (num == PAIR_ENTITY)
					{
						entities.addStaticEntity(mouse.getFirstEntityData(), mouse.getSecondEntityData(), addEntity);
					}

					if (recorder.checkHasChanges(addEntity))
					{
						recorder.newAction(addEntity);
					}
					break;
				}
				case TILE_EDITING_MODE:
				{
					//Interrupt tile building process if both tile building keys and left click are held down
					if (!tileChanges.newTiles.empty())
					{
						recorder.newAction(tileChanges);
						tileChanges.oldTiles.clear();
						tileChanges.newTiles.clear();
					}

					hasRegionSelected = false;
					mouse.setCursorType(REGION_SELECT_CURSOR);
					mouse.buildSelectionRegionBuffer(currentMouseModelCoord, currentMouseModelCoord, true, true);
					break;
				}
				case REGION_EDITING_MODE:
				{
					Modification paste;
					entities.pasteSelected(paste);
					tiles.pasteSelected(paste);
					if (recorder.checkHasChanges(paste))
					{
						recorder.newAction(paste);
					}
					break;
				}
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

			glm::vec2 oldCoord = this->calculateMouseModelCoord(window, mouseLeftButtonPressedX, mouseLeftButtonPressedY);

			//Allow some click inaccuracy due to small drags
			/*
			if (std::pow(mouseLeftButtonReleasedX - mouseLeftButtonPressedX, 2) +
				std::pow(mouseLeftButtonReleasedY - mouseLeftButtonPressedY, 2) <
				5.0 * 5.0
				&& mouseLeftButtonPressedEndTime - mouseLeftButtonPressedStartTime <= 0.2)
			{
				//Do things when mouse left key is clicked
			}
			*/

			//Do things when mouse left key is released
			switch (currentEditingMode)
			{
				case ENTITY_EDITING_MODE:
				{
					entities.placedownHighlightedEntity(moveEntity);
					if (recorder.checkHasChanges(moveEntity))
					{
						recorder.newAction(moveEntity);
						moveEntity.oldSingleEntity.clear();
						moveEntity.newSingleEntity.clear();
						moveEntity.oldPairEntity.clear();
						moveEntity.oldPairEntity.clear();
					}
					break;
				}
				case TILE_EDITING_MODE:
				{
					hasRegionSelected = true;
					mouse.buildSelectionRegionBuffer(oldCoord, currentMouseModelCoord, true, false);
					tiles.setSelectedRegion(oldCoord, currentMouseModelCoord);
					entities.setSelectedRegion(oldCoord, currentMouseModelCoord, true);
					break;
				}
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

	glm::vec2 currentMouseModelCoord = this->calculateMouseModelCoord(window, xpos, ypos);
	glm::ivec2 currentTileCoordinate = UtilityFunctions::clampToNearestTileCoord(currentMouseModelCoord, gridSpacing);
	glm::ivec2 currentEntityCoordinate = UtilityFunctions::clampToNearestPlaceableEntityCoord(currentMouseModelCoord, gridSpacing);

	if (overlay.wantControl((float)xpos, (float)ypos))
	{
		overlay.processMouseLocation((float)xpos, (float)ypos);
		mouse.resetHintEntity();
		mouse.setHintEntityType(overlay.getSelectedEntityType());
		mouse.setHintEntityRotation(currentEntityRotation);
		mouse.update(currentMouseModelCoord.x, currentMouseModelCoord.y);
		return;
	}

	mouse.update(currentMouseModelCoord.x, currentMouseModelCoord.y);

	switch (currentEditingMode)
	{
		case ENTITY_PLACEMENT_MODE:
			break;
		case ENTITY_EDITING_MODE:
		{
			if (leftDown)
			{
				entities.moveHighlightedEntity(currentEntityCoordinate.x, currentEntityCoordinate.y);
			}
			else
			{
				if (oldEntityCoordinate != currentEntityCoordinate)
				{
					entities.highlightClosestEntity(currentEntityCoordinate, MAX_HIGHLIGHT_SEARCH_RADIUS);
				}
			}
			break;
		}
		case TILE_EDITING_MODE:
		{
			if (!hasRegionSelected)
			{
				if (leftDown)
				{
					glm::vec2 oldCoord = calculateMouseModelCoord(window, mouseLeftButtonPressedX, mouseLeftButtonPressedY);

					mouse.buildSelectionRegionBuffer(oldCoord, currentMouseModelCoord, true, true);
				}
				else //Mouse drag addition/deletion for tiles
				{
					if (keyStates[GLFW_KEY_E])
					{
						if (oldTileCoordinate != currentTileCoordinate)
						{
							int key = getFirstHeldKey();
							tiles.addTile(currentTileCoordinate.x, currentTileCoordinate.y, getTileRotationKeyModifier(key), FULL, tileChanges);
						}
					}
					else if (keyStates[GLFW_KEY_D])
					{
						if (oldTileCoordinate != currentTileCoordinate)
						{
							int key = getFirstHeldKey();
							tiles.addTile(currentTileCoordinate.x, currentTileCoordinate.y, getTileRotationKeyModifier(key), EMPTY, tileChanges);
						}
					}
					else if (keyStates[GLFW_KEY_Q] || keyStates[GLFW_KEY_W] || keyStates[GLFW_KEY_S] || keyStates[GLFW_KEY_A])
					{
						if (oldTileCoordinate != currentTileCoordinate)
						{
							int key = getFirstHeldKey();
							tiles.addTile(currentTileCoordinate.x, currentTileCoordinate.y, getTileRotationKeyModifier(key), currentTileType, tileChanges);
						}
					}
				}
			}
			break;
		}
		case REGION_EDITING_MODE:
			entities.moveHint(currentMouseModelCoord);
			tiles.moveHint(currentMouseModelCoord);
			break;
		default:
			break;
	}

	if (rightDown)
	{
		// Move camera
		viewpoint->move(-dx, -dy);
	}

	//Update mouse to new position
	MouseX = (float)xpos;
	MouseY = (float)ypos;
	if (oldTileCoordinate != currentTileCoordinate) {oldTileCoordinate = currentTileCoordinate;}
	if (oldEntityCoordinate != currentEntityCoordinate) {oldEntityCoordinate = currentEntityCoordinate;}
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

TileType LevelEditor::getTileTypeKeyModifier(int key)
{
	switch (key)
	{
		case GLFW_KEY_1:
			return SLOPE_45DEG;
		case GLFW_KEY_2:
			return SMALLSLOPE_RIGHT_60DEG;
		case GLFW_KEY_3:
			return SMALLSLOPE_LEFT_60DEG;
		case GLFW_KEY_4:
			return CURVE_IN;
		case GLFW_KEY_5:
			return HALF;
		case GLFW_KEY_6:
			return LARGESLOPE_RIGHT_60DEG;
		case GLFW_KEY_7:
			return LARGESLOPE_LEFT_60DEG;
		case GLFW_KEY_8:
			return CURVE_OUT;
		case GLFW_KEY_9:
			return BORDER_TELEPORT;
		default:
			return SLOPE_45DEG; //Default in editor is 45 deg tile
	}
}

TileType LevelEditor::getCurrentTileType()
{
	return currentTileType;
}

int LevelEditor::getFirstHeldKey()
{
	for (int i = 0; i < GLFW_KEY_LAST + 1; i++)
	{
		if (keyStates[i]) { return i; }
	}

	return -1; //Unable to find held key
}

TileRotation LevelEditor::getTileRotationKeyModifier(int key)
{
	switch (key)
	{
		case GLFW_KEY_Q:
			return TILE_DEGREE_0;
		case GLFW_KEY_W:
			return TILE_DEGREE_90;
		case GLFW_KEY_S:
			return TILE_DEGREE_180;
		case GLFW_KEY_A:
			return TILE_DEGREE_270;
		case GLFW_KEY_E:
			return TILE_DEGREE_0;
		case GLFW_KEY_D:
			return TILE_DEGREE_0;
		default:
			return TILE_DEGREE_0;
	}
}

TileRotation LevelEditor::getCurrentTileRotation()
{
	return currentTileRotation;
}

EntityRotation LevelEditor::getEntityRotationByKey(int key)
{
	switch (key)
	{
		case GLFW_KEY_D:
			return ENTITY_DEGREE_0;
		case GLFW_KEY_E:
			return ENTITY_DEGREE_45;
		case GLFW_KEY_W:
			return ENTITY_DEGREE_90;
		case GLFW_KEY_Q:
			return ENTITY_DEGREE_135;
		case GLFW_KEY_A:
			return ENTITY_DEGREE_180;
		case GLFW_KEY_Z:
			return ENTITY_DEGREE_225;
		case GLFW_KEY_S:
			return ENTITY_DEGREE_270;
		case GLFW_KEY_C:
			return ENTITY_DEGREE_315;
		default:
			return ENTITY_DEGREE_0;
	}
}

EntityMode LevelEditor::getEntityModeByKey(int key)
{
	switch (key)
	{
		case GLFW_KEY_1:
			return TRACE_WALL_CLOCKWISE;
		case GLFW_KEY_2:
			return TRACE_WALL_COUNTERCLOCKWISE;
		case GLFW_KEY_3:
			return TURN_CLOCKWISE_ON_COLLISION;
		case GLFW_KEY_4:
			return TURN_COUNTERCLOCKWISE_ON_COLLISION;
		default:
			return TRACE_WALL_CLOCKWISE;
	}
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

glm::vec2 LevelEditor::calculateMouseModelCoord(GLFWwindow* window, double mouseX, double mouseY)
{
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	return UtilityFunctions::convertScreenCoordToModelCoord(
		glm::vec2((float)mouseX, (float)mouseY),
		(float)display_w, (float)display_h,
		viewpoint->getViewMtx(), viewpoint->getProjectionMtx(), levelRegionModelMtx);
}

void LevelEditor::calculateOldObjectCoord()
{

}

void LevelEditor::calculateCurrentObjectCoord()
{
	
}

void LevelEditor::resetStates()
{
	currentEditingMode = TILE_EDITING_MODE;
	mouse.setCursorType(TILE_CURSOR);
	mouse.clearSelectionRegionBuffer();
	hasRegionSelected = false;
	mouse.setFollowMouse(false);
	entities.stopHighlight();
	entities.unstageSelected();
	entities.setHintToFollowMouse(false);
	tiles.unstageSelected();
	tiles.setHintToFollowMouse(false);
}