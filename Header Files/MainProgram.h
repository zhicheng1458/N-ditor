#pragma once

#include "Core.h"
#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "LevelEditor.h"
#include "GUIcontainer.h"

class MainProgram {
public:
	MainProgram();
	~MainProgram();

	void init();

	void mainLoop();

	void Keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
	void MouseButton(GLFWwindow* window, int button, int action, int mods);
	void MouseMotion(GLFWwindow* window, double xpos, double ypos);
	void Scrolling(GLFWwindow* window, double xoffset, double yoffset);
	void Resize(GLFWwindow* window, int width, int height);
	void CloseWindow(GLFWwindow* window);

private:
	// Window management
	GLFWwindow * window;
	int WinX, WinY;

	//Rendering
	ShaderProgram * Program; //Does most of the model drawing
	ShaderProgram * RenderProgram; //Responsible for rendering the final quad to the screen as a texture
	ShaderProgram * blendingProgram; //Combining solid and transparent object together

	const uint samplesPerFragment = 4;
	uint frameBufferMS = 0;
	uint renderBufferMS = 0;
	Texture screenAsTextureMS; // The 2 triangle made up the screen, pre-process
	Texture transparentObjectMS;
	Texture overlapCountTextureMS; // This texture essentially count the number of stacked object at each fragment position
	GLfloat clearRGBA[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; //Clear to black
	GLenum multiTargetRender[2] = { GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };

	Model * screen;
	uint frameBuffer = 0;
	uint renderBuffer = 0;
	Texture screenAsTexture; // The 2 triangle made up the screen, post-process

	//Level Editor
	LevelEditor * levelEditor;

	//ImGUI
	GUIcontainer * gui;
	const char* glsl_version = "#version 460";
	ImGuiContext * imguiContext;

	//Program updates
	void update();
	void draw();
	//void reset();
	//void quit();

	void initScreenBuffers();
};