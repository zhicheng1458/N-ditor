#include "MainProgram.h"
#include <fstream>

static MainProgram * N_DITOR = 0;

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

//GLFW stuff
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	N_DITOR->Keyboard(window, key, scancode, action, mods);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	N_DITOR->MouseButton(window, button, action, mods);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	N_DITOR->MouseMotion(window, xpos, ypos);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	N_DITOR->Scrolling(window, xoffset, yoffset);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	N_DITOR->Resize(window, width, height);
}

static void window_close_callback(GLFWwindow * window)
{
	N_DITOR->CloseWindow(window);
}

//////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		printf("GLFW init failed, exiting \n");
		return -1;
	}
	else
	{
		printf("GLFW init successfully \n");
	}

	N_DITOR = new MainProgram();
	N_DITOR->mainLoop();
	delete N_DITOR;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Program initialization
//////////////////////////////////////////////////////////////////////////

MainProgram::MainProgram()
{
	WinX = 1600;
	WinY = 900;

	init();
}

void MainProgram::init()
{
	// Create the main window
	glfwWindowHint(GLFW_SAMPLES, 4);
	window = glfwCreateWindow(WinX, WinY, "Nditor++", NULL, NULL);
	if (!window)
	{
		// Window or context creation failed
		fprintf(stderr, "window creation failed, exiting \n");
		glfwTerminate();
	}

	//Get current GLSL version
	//printf("Version GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glViewport(0, 0, WinX, WinY);
	glfwSetWindowPos(window, 160, 100);
	glfwMakeContextCurrent(window);
	glClearColor(0.0f, 1.0f, 1.0f, 1.0f); //Set clear color for the window
	glfwSwapInterval(1); //Set to 0 for maximum fps, 1 to lock to 60 fps

	// Keyboard and Mouse control
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetWindowCloseCallback(window, window_close_callback);

	glewInit(); //must be done after selecting the window, otherwise shader compile fails

	// Initialize components

	//Screen
	Program = new ShaderProgram("./Resources/Shaders/Model.glsl", ShaderProgram::eRender);
	RenderProgram = new ShaderProgram("./Resources/Shaders/RenderBuffer.glsl", ShaderProgram::eRender);
	blendingProgram = new ShaderProgram("./Resources/Shaders/BlendShader.glsl", ShaderProgram::eRender);
	glUseProgram(RenderProgram->getProgramID());
	glUniform1i(glGetUniformLocation(RenderProgram->getProgramID(), "numSample"), samplesPerFragment);
	glUseProgram(0);
	glUseProgram(blendingProgram->getProgramID());
	glUniform1i(glGetUniformLocation(blendingProgram->getProgramID(), "solidObjectTexture"), 0); //solid object texture will replace GL_TEXTURE0
	glUniform1i(glGetUniformLocation(blendingProgram->getProgramID(), "transparentObjectTexture"), 1); //transparent object texture will replace GL_TEXTURE1
	glUniform1i(glGetUniformLocation(blendingProgram->getProgramID(), "overlapCountTexture"), 2); //overlap count texture will replace GL_TEXTURE2
	glUseProgram(0);
	screen = new Model();
	screen->addTriangle(glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f),
						glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
						glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f));
	screen->addTriangle(glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f),
						glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
						glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
	screen->setBuffers();
	this->initScreenBuffers();

	//Level editor
	levelEditor = new LevelEditor((float)WinX, (float)WinY);

	//GUI
	IMGUI_CHECKVERSION();
	imguiContext = ImGui::CreateContext();
	ImGuiIO & io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("misc/fonts/Roboto-Medium.ttf", 14.0f); //Text font file
	io.Fonts->Build();
	io.IniFilename = NULL; //Use this flag to avoid initialization of window
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	gui = new GUIcontainer(window, levelEditor);
}

void MainProgram::initScreenBuffers()
{
	//Delete all existing buffers
	if (frameBuffer) { glDeleteFramebuffers(1, &frameBuffer); }
	if (renderBuffer) { glDeleteRenderbuffers(1, &renderBuffer); }
	if (frameBufferMS) { glDeleteFramebuffers(1, &frameBufferMS); }
	if (renderBufferMS) { glDeleteRenderbuffers(1, &renderBufferMS); }

	//Init screen
	screenAsTexture.loadEmptyTexture(WinX, WinY);
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenAsTexture.getTextureID(), 0);
	glGenRenderbuffers(1, &renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WinX, WinY);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	screenAsTextureMS.loadEmptyMultisampleTexture(WinX, WinY, samplesPerFragment);
	transparentObjectMS.loadEmptyMultisampleTexture(WinX, WinY, samplesPerFragment);
	overlapCountTextureMS.loadEmptyMultisampleTexture(WinX, WinY, samplesPerFragment);
	glGenFramebuffers(1, &frameBufferMS);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferMS);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, screenAsTextureMS.getTextureID(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, transparentObjectMS.getTextureID(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, overlapCountTextureMS.getTextureID(), 0);
	glGenRenderbuffers(1, &renderBufferMS);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferMS);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samplesPerFragment, GL_DEPTH24_STENCIL8, WinX, WinY);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferMS);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//////////////////////////////////////////////////////////////////////////
// Program updates and draws
//////////////////////////////////////////////////////////////////////////

void MainProgram::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		//Main animation window
		//glfwMakeContextCurrent(window);
		glClear(GL_COLOR_BUFFER_BIT);
		this->update();
		this->draw();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void MainProgram::update()
{
	gui->update();
}

void MainProgram::draw()
{
	GLenum error = 0;
	if ((error = glGetError()) != GL_NO_ERROR)
	{
		printf("%i \n", (int)error);
	}

	glUseProgram(Program->getProgramID());
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferMS);
	{
		//Solid object drawing
		glDrawBuffer(GL_COLOR_ATTACHMENT0); //screen texture was bound to color attachment 0
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glEnable(GL_DEPTH_TEST);
		glDisable(GL_DEPTH_TEST); // TODO: Optimization, sort rendering order to enable z-culling
		levelEditor->drawSolidLayer();
	}
	{
		//Transparent object drawing
		glDrawBuffers(2, multiTargetRender);
		glClearBufferfv(GL_COLOR, 0, clearRGBA); //transparent object texture
		glClearBufferfv(GL_COLOR, 1, clearRGBA); //count object texture
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_ONE, GL_ONE);
		levelEditor->drawTransparentLayer();
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glDrawBuffer(GL_COLOR_ATTACHMENT0); //Reset draw target
	}
	glUseProgram(0);

	glUseProgram(blendingProgram->getProgramID());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, screenAsTextureMS.getTextureID());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, transparentObjectMS.getTextureID());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, overlapCountTextureMS.getTextureID());
	screen->draw(glm::mat4(1), glm::mat4(1), blendingProgram->getProgramID());
	glUseProgram(0);

	levelEditor->drawOverlay();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Anti-Aliasing via multi-sample (**ONLY WORK FOR TRIANGLE EDGES, NOT INDIVIDUAL FRAGMENTS**)
	//Reason is that when drawing circle by using mathematical formula and discard the fragment,
	//The shader will still consider the whole triangle that makes up the circle to be there,
	//which means when attempting to average the color out in each sample, every samples is "inside the triangle".
	//Additionally, the color of each fragment is only calculated once instead of for each sample,
	//hence why you cannot anti alias a circle with this method.
	//TODO: Make a custom anti aliasing fragment shader.
	glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferMS);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
	glBlitFramebuffer(0, 0, WinX, WinY, 0, 0, WinX, WinY, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glFinish();

	// Draw screen + post processing, if any
	glUseProgram(RenderProgram->getProgramID());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screenAsTexture.getTextureID());
	//glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	screen->draw(glm::mat4(1), glm::mat4(1), RenderProgram->getProgramID());
	glBindTexture(GL_TEXTURE_2D, 0);

	gui->draw();

	if ((error = glGetError()) != GL_NO_ERROR)
	{
		printf("%i \n", (int)error);
	}
}

//////////////////////////////////////////////////////////////////////////
// Keyboard and Mouse control
//////////////////////////////////////////////////////////////////////////

void MainProgram::Keyboard(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	if (!gui->wantKeyboardControl())
	{
		gui->keyboard(window, key, scancode, action, mods);
		levelEditor->keyboard(window, key, scancode, action, mods);
	}
}

void MainProgram::MouseButton(GLFWwindow * window, int button, int action, int mods)
{
	if (!gui->wantMouseControl())
	{
		gui->mouseButton(window, button, action, mods);
		levelEditor->mouseButton(window, button, action, mods);
	}
}

void MainProgram::MouseMotion(GLFWwindow * window, double xpos, double ypos)
{
	if (!gui->wantMouseControl())
	{
		gui->mouseMotion(window, xpos, ypos);
		levelEditor->mouseMotion(window, xpos, ypos);
	}
}

void MainProgram::Scrolling(GLFWwindow * window, double xoffset, double yoffset)
{
	if (!gui->wantMouseControl())
	{
		gui->scrolling(window, xoffset, yoffset);
		levelEditor->scrolling(window, xoffset, yoffset);
	}
}

void MainProgram::Resize(GLFWwindow* window, int width, int height)
{
	WinX = width;
	WinY = height;
	glViewport(0, 0, WinX, WinY);
	this->initScreenBuffers();

	if (!gui->wantMouseControl())
	{
		levelEditor->resize(window, width, height);
	}
}

void MainProgram::CloseWindow(GLFWwindow * window)
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

MainProgram::~MainProgram() {
	glFinish();

	//delete viewpoint;
	delete levelEditor;

	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteRenderbuffers(1, &renderBuffer);
	glDeleteFramebuffers(1, &frameBufferMS);
	glDeleteFramebuffers(1, &renderBufferMS);
	delete screen;

	delete gui;
	glfwDestroyWindow(window);
	glfwTerminate();
}