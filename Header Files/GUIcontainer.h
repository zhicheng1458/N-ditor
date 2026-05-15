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
	GUIcontainer(GLFWwindow * window, LevelEditor * levelEditor);
	~GUIcontainer();

	void update();
	void draw();

	bool wantMouseControl();
	bool wantKeyboardControl();

	void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouseButton(GLFWwindow* window, int button, int action, int mods);
	void mouseMotion(GLFWwindow* window, double xpos, double ypos);
	void scrolling(GLFWwindow* window, double xoffset, double yoffset);
	void resize(GLFWwindow* window, int width, int height);

private:

	const bool DEBUG = true;
	SystemMessageGUI debugWindow;
	const ImVec4 SUCCESS_COLOR = ImVec4(0.1f, 0.9f, 0.1f, 1.0f); //Green
	const ImVec4 ERROR_COLOR = ImVec4(0.9f, 0.1f, 0.1f, 1.0f);   //Red
	const ImVec4 NEUTRAL_COLOR = ImVec4(0.9f, 0.9f, 0.1f, 1.0f); //Yellow

	GLFWwindow * window;
	LevelEditor * levelEditor;
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