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

	//Don't create another context if it already exist
	if (ImGui::GetCurrentContext() == NULL)
	{
		printf("There is no ImGui Context, exiting program. \n");
		exit(EXIT_FAILURE);
	}

	this->window = window;
	main_viewport = ImGui::GetMainViewport();

	init();
}

void GUIcontainer::init()
{
	//Loader/Saver
	getImportExportFileNames();

	//GUI style initialization
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.12f, 0.12f, 0.6f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 0.9f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.70f, 1.00f, 0.00f, 1.0f); //Don't need for now
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.353f, 0.569f, 0.235f, 0.6f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.392f, 0.804f, 0.175f, 0.6f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.47f, 0.922f, 0.078f, 0.6f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.8f);
	style.WindowRounding = 6.0f;

	glfwGetFramebufferSize(window, &display_w, &display_h);
	windowCreationLocationX_Loader = (float)display_w - windowPaddingFromViewport_Loader - windowWidth_Loader;
	windowCreationLocationY_Loader = windowPaddingFromViewport_Loader;
}

void GUIcontainer::update()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Miscellaneous Information");

	ImGui::SetWindowPos(ImVec2(5.0f, 5.0f), ImGuiCond_FirstUseEver); //(width, height)
	ImGui::SetWindowSize(ImVec2(200.0f, 50.0f), ImGuiCond_FirstUseEver);

	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

	ImGui::End();

	/* Loading/saving window */
	buildLoadingSavingWindow();

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

	//ImGui::ShowDemoWindow();
}

void GUIcontainer::draw()
{
	ImGui::Render();
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

void GUIcontainer::buildLoadingSavingWindow()
{
	ImGui::Begin("Loading / Saving", NULL, ImGuiWindowFlags_NoResize); //Pass NULL to open status to prevent window from being closed
	ImGui::SetWindowPos(ImVec2(windowCreationLocationX_Loader, windowCreationLocationY_Loader), ImGuiCond_FirstUseEver); //(width, height)
	ImGui::SetWindowSize(ImVec2(windowWidth_Loader, windowHeight_Loader), ImGuiCond_FirstUseEver);

	float startingX = (ImGui::GetWindowWidth() - (inputBoxWidth + 3 * buttonWidth + 3 * itemSpacing)) / 2.0f;
	ImGui::NewLine();
	ImGui::SameLine(startingX);
	ImGui::PushID(&importExportLevelText);
	ImGui::SetNextItemWidth(inputBoxWidth);
	bool importExport_EnterKeyPressed = ImGui::InputTextWithHint("", "Type the level name here or use the dropdown!", importExportLevelText, sizeof(importExportLevelText));
	ImGui::PopID();
	bool importExport_isTyping = ImGui::IsItemActive();
	bool importExport_isTextboxInteracted = ImGui::IsItemActivated();
	if (importExport_isTextboxInteracted)
	{
		ImGui::OpenPopup("Names");
		displayImportWarningCheckbox = false;
		displayExportWarningCheckbox = false;
		discardUnsaveChanges = false;
		overrideFile = false;
	}

	ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
	float h = std::min(ImGui::GetWindowHeight() - ImGui::GetItemRectMax().y - 5.0f, ImGui::GetItemRectSize().y * 3.0f + ImGui::GetStyle().FramePadding.y);
	ImGui::SetNextWindowSize(ImVec2(inputBoxWidth, h));
	if (ImGui::BeginPopup("Names", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_ChildWindow))
	{
		for (int i = 0; i < levelNames.size(); i++)
		{
			if (ImGui::Selectable(levelNames[i].c_str()))
			{
				ImGui::ClearActiveID();
				snprintf(importExportLevelText, sizeof(importExportLevelText), "%s", levelNames[i].c_str());
			}
		}

		if (importExport_EnterKeyPressed || (!importExport_isTyping && !ImGui::IsWindowFocused()))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::SameLine(0.0f, itemSpacing);
	if(ImGui::Button("Import", ImVec2(buttonWidth, 0.0f)))
	{
		overrideFile = false; //Export operation canceled in case it was asking for overriding permission
		displayExportWarningCheckbox = false;

		if (!levelEditor.recorder.hasUnsavedWork() || discardUnsaveChanges)
		{
			discardUnsaveChanges = false;
			displayImportWarningCheckbox = false;
			levelEditor.resetStates();
			levelEditor.recorder.reset(); //Recorder will stop remembering all actions when loading a new level.
			if (levelEditor.levelParser.importLevel(importExportLevelText, levelEditor.tiles, levelEditor.entities, levelEditor.levelProperty))
			{
				snprintf(mBuffer, MESSAGE_BUFFER_SIZE, "Successfully imported level named \"%s\" into the editor.", importExportLevelText);
				m.message = mBuffer;
				m.color = CommonMessageColor::SUCCESS_COLOR;
				debugWindow.addMessage(m);
			}
			else
			{
				debugWindow.addMessage(levelEditor.levelParser.queryError());
			}
		}
		else
		{
			displayImportWarningCheckbox = true;
			m.message = "There are unsaved changed! Check the \"Discard all unsaved changes\" box, then import again to confirm.";
			m.color = CommonMessageColor::NEUTRAL_COLOR;
			debugWindow.addMessage(m);
		}
	}
	ImGui::SameLine(0.0f, itemSpacing);
	if (ImGui::Button("Export", ImVec2(buttonWidth, 0.0f)))
	{
		discardUnsaveChanges = false; //Import operation canceled in case it was asking for discard permission
		displayImportWarningCheckbox = false;

		if (!levelEditor.levelParser.checkImportExportNameExist(importExportLevelText) || overrideFile)
		{
			overrideFile = false;
			displayExportWarningCheckbox = false;
			levelEditor.resetStates();
			levelEditor.recorder.flagCurrentStepSaved();
			if (levelEditor.levelParser.exportLevel(importExportLevelText, levelEditor.tiles, levelEditor.entities, levelEditor.levelProperty))
			{
				snprintf(mBuffer, MESSAGE_BUFFER_SIZE, "Successfully exported level file named \"%s\".", importExportLevelText);
				m.message = mBuffer;
				m.color = CommonMessageColor::SUCCESS_COLOR;
				debugWindow.addMessage(m);
			}
			else
			{
				debugWindow.addMessage(levelEditor.levelParser.queryError());
			}
		}
		else
		{
			displayExportWarningCheckbox = true;
			m.message = "You are about to override an existing file! Check the \"Override existing file\" box, then export again to confirm.";
			m.color = CommonMessageColor::NEUTRAL_COLOR;
			debugWindow.addMessage(m);
		}

		getImportExportFileNames();
	}
	ImGui::SameLine(0.0f, itemSpacing);
	if (ImGui::Button("Refresh", ImVec2(buttonWidth, 0.0f)))
	{
		getImportExportFileNames();
	}

	if (displayImportWarningCheckbox)
	{
		ImGui::NewLine();
		ImGui::SameLine(startingX);
		ImGui::Checkbox("Discard all unsave changes", &discardUnsaveChanges);
	}
	if (displayExportWarningCheckbox)
	{
		ImGui::NewLine();
		ImGui::SameLine(startingX);
		ImGui::Checkbox("Override existing file", &overrideFile);
	}

	/*
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::NewLine();
	ImGui::SameLine(startingX);
	ImGui::SetNextItemWidth(inputBoxWidth);
	ImGui::PushID(&saveLevelText);
	ImGui::SetNextItemWidth(inputBoxWidth);
	ImGui::InputTextWithHint("", "Type the level name here or use the dropdown!", saveLevelText, IM_ARRAYSIZE(saveLevelText));
	ImGui::PopID();
	*/

	ImGui::End();
}

void GUIcontainer::getImportExportFileNames()
{
	levelNames.clear();
	if (!std::filesystem::is_directory(DIRECTORY_PATH))
	{
		fprintf(stderr, "Unable to find directory to load level");
		exit(EXIT_FAILURE);
	}

	for (const auto& entry : std::filesystem::directory_iterator(DIRECTORY_PATH))
	{
		if (entry.is_regular_file())
		{
			levelNames.push_back(entry.path().filename().string());
		}
	}
}

GUIcontainer::~GUIcontainer()
{
	//Do not delete the window here! The editor doesn't
	//control the life of the window itself
	this->window = nullptr;
}