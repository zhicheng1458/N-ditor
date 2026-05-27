#pragma once
#include "Core.h"
#include "UtilityFunctions.h"

// Setup ImGUI
#include "imconfig.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "SystemMessageGUI.h"

//To obtain a reference to object that need debugging
#include "LevelEditor.h"
#include "Camera.h"

class GUIcontainer
{
public:
	GUIcontainer(GLFWwindow * window, LevelEditor & levelEditor);
	~GUIcontainer();

	void update();
	void draw();

	bool wantMouseControl();
	bool wantKeyboardControl();

private:

	const bool DEBUG = true;
	SystemMessageGUI debugWindow;
	const glm::vec4 SUCCESS_COLOR = glm::vec4(0.1f, 0.9f, 0.1f, 1.0f); //Green
	const glm::vec4 ERROR_COLOR = glm::vec4(0.9f, 0.1f, 0.1f, 1.0f);   //Red
	const glm::vec4 NEUTRAL_COLOR = glm::vec4(0.9f, 0.9f, 0.1f, 1.0f); //Yellow

	GLFWwindow * window;
	LevelEditor & levelEditor;
	Camera * viewpoint;

	//Keyboard and Mouse
	bool leftDown, middleDown, rightDown;
	float MouseX, MouseY; //For purely tracking mouse location
	double mouseLeftButtonPressedX, mouseLeftButtonPressedY, mouseLeftButtonReleasedX, mouseLeftButtonReleasedY; //For tracking leftclicking
	double mouseLeftButtonPressedStartTime, mouseLeftButtonPressedEndTime;
	bool keyStates[GLFW_KEY_LAST + 1];

	//Debug messaging
	Message m;
};