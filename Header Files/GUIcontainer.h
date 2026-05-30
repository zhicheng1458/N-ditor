#pragma once
#include "Core.h"
#include "UtilityFunctions.h"

// Setup ImGUI
#include "imconfig.h"
#include "imgui_internal.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Message.h"
#include "SystemMessageGUI.h"
#include <filesystem>

//To obtain a reference to object that need debugging
#include "LevelEditor.h"
#include "Camera.h"

class GUIcontainer
{
public:
	//GUIcontainer() = delete;
	GUIcontainer(GLFWwindow * window, LevelEditor & levelEditor);
	~GUIcontainer();
	void init();

	void update();
	void draw();

	bool wantMouseControl();
	bool wantKeyboardControl();

	void buildLoadingSavingWindow();

private:

	LevelEditor& levelEditor;

	GLFWwindow * window;
	int display_w, display_h;

	ImGuiViewport* main_viewport;

	//Loading/saving window
	bool isOpen_Loader = true;
	float windowCreationLocationX_Loader, windowCreationLocationY_Loader;
	float windowWidth_Loader = 500.0f, windowHeight_Loader = 150.0f;
	float windowPaddingFromViewport_Loader = 5.0f;
	float inputBoxWidth = 260.0f;
	float buttonWidth = 60.0f;
	float itemSpacing = 5.0f;
	char importExportLevelText[128];
	//char saveLevelText[128];
	std::vector<std::string> levelNames;
	int selectedIndex = 0;
	bool displayImportWarningCheckbox = false;
	bool discardUnsaveChanges = false;
	bool displayExportWarningCheckbox = false;
	bool overrideFile = false;

	const char* DIRECTORY_PATH = "./Levels/";
	void getImportExportFileNames();

	//System message window
	const bool DEBUG = true;
	SystemMessageGUI debugWindow;

	const int MESSAGE_BUFFER_SIZE = 512;
	char mBuffer[512];
	Message m;
};