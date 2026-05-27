#include "GUIcontainer.h"

//For debug printing
#include <fstream>

GUIcontainer::GUIcontainer(GLFWwindow * window, LevelEditor & levelEditor) : levelEditor(levelEditor)
{
	if (!window)
	{
		fprintf(stderr, "window creation failed \n");
		exit(EXIT_FAILURE);
	}

	this->window = window;
	this->viewpoint = levelEditor.getCamera();
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
		if (levelEditor.hasDebugInfo)
		{
			debugWindow.addMessage(levelEditor.debugMessage);
			levelEditor.hasDebugInfo = false;
		}
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

GUIcontainer::~GUIcontainer()
{
	//Do not delete the window here! The editor doesn't
	//control the life of the window itself
	this->window = nullptr;
}