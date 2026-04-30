#include "GUIcontainer.h"

//For debug printing
#include <fstream>

GUIcontainer::GUIcontainer(GLFWwindow * window, LevelEditor * levelEditor)
{
	if (!window)
	{
		fprintf(stderr, "window creation failed \n");
		exit(EXIT_FAILURE);
	}

	this->window = window;
	this->levelEditor = levelEditor;
	this->viewpoint = levelEditor->getCamera();
}

void GUIcontainer::update()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Miscellaneous Information");

	ImGui::SetWindowPos(ImVec2(5, 5), ImGuiCond_FirstUseEver); //(width, height)
	ImGui::SetWindowSize(ImVec2(200, 50), ImGuiCond_FirstUseEver);

	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

	ImGui::End();

	if (DEBUG)
	{
		//ImGui::ShowDemoWindow();
		debugWindow.displaySystemMessageWindow();
	}
}

void GUIcontainer::draw()
{
	ImGui::Render();
	//int display_w, display_h;
	//glfwGetFramebufferSize(window, &display_w, &display_h);
	//glViewport(0, 0, display_w, display_h);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool GUIcontainer::wantMouseControl()
{
	ImGuiIO & io = ImGui::GetIO();
	return io.WantCaptureMouse;
}

bool GUIcontainer::wantKeyboardControl()
{
	ImGuiIO & io = ImGui::GetIO();
	return io.WantCaptureKeyboard;
}

//////////////////////////////////////////////////////////////

void GUIcontainer::keyboard(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		keyStates[key] = true;

		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glm::vec2 mouseCoordinate = UtilityFunctions::convertScreenCoordToModelCoord(
									glm::vec2((float)MouseX, (float)MouseY),
									(float)display_w, (float)display_h,
									viewpoint->getViewMtx(), viewpoint->getProjectionMtx(), glm::mat4(1.0f));
		glm::ivec2 tileCoordinate = UtilityFunctions::clampToNearestTileCoord(mouseCoordinate, levelEditor->getGridSpacing());
		editingMode mode = levelEditor->getEditingMode();

		switch (mode)
		{
			case TILE_EDITING_MODE:
			{
				switch(key)
				{
				case GLFW_KEY_UNKNOWN:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Unknown key press detected");
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_1:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Tile modifier (code): %1i", SLOPE_45DEG);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_2:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Tile modifier (code): %1i", SMALLSLOPE_RIGHT_60DEG);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_3:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Tile modifier (code): %1i", SMALLSLOPE_LEFT_60DEG);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_4:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Tile modifier (code): %1i", CURVE_IN);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_5:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Tile modifier (code): %1i", HALF);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_6:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Tile modifier (code): %1i", LARGESLOPE_RIGHT_60DEG);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_7:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Tile modifier (code): %1i", LARGESLOPE_LEFT_60DEG);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_8:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Tile modifier (code): %1i", CURVE_OUT);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_E:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Placed tile at tile coord (x, y) = (%i, %i)\n"
							"Tile type = %i", tileCoordinate.x, tileCoordinate.y, FULL);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_D:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Deleted tile at tile coord (x, y) (if any) = (%i, %i)\n", tileCoordinate.x, tileCoordinate.y);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_Q:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Placed tile at tile coord (x, y) = (%i, %i)\n"
							"Tile type = %i\n"
							"Tile rotation (non-60deg tile): %i", tileCoordinate.x, tileCoordinate.y, levelEditor->getTileTypeKeyModifier(), TILE_DEGREE_0);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_W:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Placed tile at tile coord (x, y) = (%i, %i)\n"
							"Tile type = %i\n"
							"Tile rotation (non-60deg tile): %i", tileCoordinate.x, tileCoordinate.y, levelEditor->getTileTypeKeyModifier(), TILE_DEGREE_90);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_S:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Placed tile at tile coord (x, y) = (%i, %i)\n"
							"Tile type = %i\n"
							"Tile rotation (non-60deg tile): %i", tileCoordinate.x, tileCoordinate.y, levelEditor->getTileTypeKeyModifier(), TILE_DEGREE_180);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_A:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Placed tile at tile coord (x, y) = (%i, %i)\n"
							"Tile type = %i\n"
							"Tile rotation (non-60deg tile): %i", tileCoordinate.x, tileCoordinate.y, levelEditor->getTileTypeKeyModifier(), TILE_DEGREE_270);
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_SPACE:
					if (DEBUG)
					{
						char tempString[256] = "";
						//GUI performed the message first before the value is actually changed
						snprintf(tempString, 256, "Switching to entity placement mode.");
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_LEFT_ALT:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Editing mode is already in tile editing mode.");
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				default:
					//Do something when that key is pressed
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Key pressed: %c", (char)key);

						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				}
				break;
			}
			case ENTITY_EDITING_MODE:
			{
				switch (key)
				{
				case GLFW_KEY_UNKNOWN:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Unknown key press detected");
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_F:
					if (DEBUG)
					{
						char tempString[256] = "";
						//GUI performed the message first before the value is actually changed
						snprintf(tempString, 256, "Switching to entity placement mode.");
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_T:
					if (DEBUG)
					{
						char tempString[256] = "";
						//GUI performed the message first before the value is actually changed
						snprintf(tempString, 256, "Attempting to delete an entity");
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_Q: case GLFW_KEY_W: case GLFW_KEY_E: case GLFW_KEY_D: case GLFW_KEY_C: case GLFW_KEY_S: case GLFW_KEY_Z: case GLFW_KEY_A:
					if (DEBUG)
					{
						char tempString[256] = "";
						//GUI performed the message first before the value is actually changed
						snprintf(tempString, 256, "Rotated the selected entity, if any");
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_SPACE:
					if (DEBUG)
					{
						char tempString[256] = "";
						//GUI performed the message first before the value is actually changed
						snprintf(tempString, 256, "Switching to entity placement mode.");
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_LEFT_ALT:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Switching to tile editing mode.");
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				default:
					//Do something when that key is pressed
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Key pressed: %c", (char)key);

						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				}
				break;
			}
			case ENTITY_PLACEMENT_MODE:
			{
				switch (key)
				{
				case GLFW_KEY_F:
					if (DEBUG)
					{
						char tempString[256] = "";
						//GUI performed the message first before the value is actually changed
						snprintf(tempString, 256, "Switching to entity editing mode.");
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_Q: case GLFW_KEY_W: case GLFW_KEY_E: case GLFW_KEY_D: case GLFW_KEY_C: case GLFW_KEY_S: case GLFW_KEY_Z: case GLFW_KEY_A:
					if (DEBUG)
					{
						char tempString[256] = "";
						//GUI performed the message first before the value is actually changed
						snprintf(tempString, 256, "Changed the rotation of the to-be-placed entity.");
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				case GLFW_KEY_SPACE:
					break;
				case GLFW_KEY_LEFT_ALT:
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Switching to tile editing mode.");
						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				default:
					//Do something when that key is pressed
					if (DEBUG)
					{
						char tempString[256] = "";
						snprintf(tempString, 256, "Key pressed: %c", (char)key);

						m.message = tempString;
						m.color = NEUTRAL_COLOR;
						debugWindow.addMessage(m);
					}
					break;
				}
				break;
			}
			default:
				break;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_E)
		{
			keyStates[GLFW_KEY_E] = false;
		}
		 else if (key != GLFW_KEY_UNKNOWN)
		{
			//Do something when that key is released
			keyStates[key] = false;
		}
	}
}

void GUIcontainer::mouseButton(GLFWwindow * window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		MouseX = (float)mouseX;
		MouseY = (float)mouseY;

		editingMode mode = levelEditor->getEditingMode();

		if (action == GLFW_PRESS)
		{
			leftDown = true;
			mouseLeftButtonPressedX = mouseX;
			mouseLeftButtonPressedY = mouseY;
			mouseLeftButtonPressedStartTime = glfwGetTime();

			//Do things when mouse left key is pressed down
			switch (mode)
			{
			case ENTITY_PLACEMENT_MODE:
				if (DEBUG)
				{
					char tempString[256] = "";
					snprintf(tempString, 256, "Placed an entity at mouse location.");
					m.message = tempString;
					m.color = NEUTRAL_COLOR;
					debugWindow.addMessage(m);
				}
				break;
			default:
				break;
			}
		}
		else if (action == GLFW_RELEASE)
		{
			leftDown = false;
			mouseLeftButtonReleasedX = mouseX;
			mouseLeftButtonReleasedY = mouseY;
			mouseLeftButtonPressedEndTime = glfwGetTime();

			//Allow some click inaccuracy due to small drags
			if (std::pow(mouseLeftButtonReleasedX - mouseLeftButtonPressedX, 2) +
				std::pow(mouseLeftButtonReleasedY - mouseLeftButtonPressedY, 2) <
				5.0 * 5.0
				&& mouseLeftButtonPressedEndTime - mouseLeftButtonPressedStartTime <= 0.2)
			{
				//Do things when mouse left key is clicked
				int display_w, display_h;
				glfwGetFramebufferSize(window, &display_w, &display_h);
				glm::vec2 clickedCoordinate = UtilityFunctions::convertScreenCoordToModelCoord(
											  glm::vec2((float)mouseLeftButtonPressedX, (float)mouseLeftButtonPressedY),
											  (float)display_w, (float)display_h,
											  viewpoint->getViewMtx(), viewpoint->getProjectionMtx(), glm::mat4(1.0f));
				glm::ivec2 tileCoordinate = UtilityFunctions::clampToNearestTileCoord(clickedCoordinate, levelEditor->getGridSpacing());
				glm::ivec2 entityCoordinate = UtilityFunctions::clampToNearestPlaceableEntityCoord(clickedCoordinate, levelEditor->getGridSpacing());

				if (DEBUG)
				{
					char tempString[512] = "";
					snprintf(tempString, 512, "Mouse clicked location:\n"
											  "Screen coordinate: (x, y) = (%.1f, %.1f)\n"
											  "Clicked coordinate on editor: (x, y) = (%.1f, %.1f)\n"
											  "Tile coordinate: = (%i, %i)\n"
											  "Nearest entity-placeable location = (%i, %i)",
							(float)mouseLeftButtonPressedX, (float)mouseLeftButtonPressedY,
							clickedCoordinate.x, clickedCoordinate.y,
							tileCoordinate.x, tileCoordinate.y,
							entityCoordinate.x, entityCoordinate.y);

					m.message = tempString;
					m.color = NEUTRAL_COLOR;
					debugWindow.addMessage(m);
				}

			}

			//Do things when mouse left key is released
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

void GUIcontainer::mouseMotion(GLFWwindow * window, double xpos, double ypos)
{
	float dx = (float)xpos - MouseX;
	float dy = -((float)ypos - MouseY);

	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glm::vec2 oldCoord = UtilityFunctions::convertScreenCoordToModelCoord(
						 glm::vec2((float)MouseX, (float)MouseY),
						 (float)display_w, (float)display_h,
						 viewpoint->getViewMtx(), viewpoint->getProjectionMtx(), glm::mat4(1.0f));
	glm::vec2 oldTileCoordinate = UtilityFunctions::clampToNearestTileCoord(oldCoord, levelEditor->getGridSpacing());
	glm::vec2 newCoord = UtilityFunctions::convertScreenCoordToModelCoord(
						 glm::vec2((float)xpos, (float)ypos),
						 (float)display_w, (float)display_h,
						 viewpoint->getViewMtx(), viewpoint->getProjectionMtx(), glm::mat4(1.0f));
	glm::vec2 newTileCoordinate = UtilityFunctions::clampToNearestTileCoord(newCoord, levelEditor->getGridSpacing());

	MouseX = (float)xpos;
	MouseY = (float)ypos;

	if (!levelEditor->getEditingMode())
	{
		if (keyStates[GLFW_KEY_E])
		{
			if (oldTileCoordinate.x != newTileCoordinate.x || oldTileCoordinate.y != newTileCoordinate.y)
			{
				if (DEBUG)
				{
					char tempString[256] = "";
					snprintf(tempString, 256, "Placed tile at tile coord (x, y) = (%i, %i)\n"
						"Tile type = %i", (int)newTileCoordinate.x, (int)newTileCoordinate.y, FULL);

					m.message = tempString;
					m.color = NEUTRAL_COLOR;
					debugWindow.addMessage(m);
				}
			}
		}
		else if (keyStates[GLFW_KEY_D])
		{
			if (oldTileCoordinate.x != newTileCoordinate.x || oldTileCoordinate.y != newTileCoordinate.y)
			{
				if (DEBUG)
				{
					char tempString[256] = "";
					snprintf(tempString, 256, "Deleted tile at tile coord (x, y) (if any) = (%i, %i)\n", (int)newTileCoordinate.x, (int)newTileCoordinate.y);

					m.message = tempString;
					m.color = NEUTRAL_COLOR;
					debugWindow.addMessage(m);
				}
			}
		}
	}

	// Move camera
	if (leftDown)
	{
		//Do something if left-click dragging
		/*
		if (DEBUG)
		{
			char tempString[256] = "";
			snprintf(tempString, 256, "Relocating camera by (x, y) = (%.1f, %.1f)", dx, dy);

			Message m;
			m.message = tempString;
			m.color = NEUTRAL_COLOR;
			debugWindow.addMessage(m);
		}
		*/
	}
}

void GUIcontainer::scrolling(GLFWwindow * window, double xoffset, double yoffset)
{
	
}

void GUIcontainer::resize(GLFWwindow * window, int width, int height)
{

}


GUIcontainer::~GUIcontainer()
{
	//Do not delete the window here! The editor doesn't
	//control the life of the window itself
	this->window = nullptr;
}