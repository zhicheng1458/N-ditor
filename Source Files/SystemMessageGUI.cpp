#include "SystemMessageGUI.h"

SystemMessageGUI::SystemMessageGUI()
{
	//Don't create another context if it already exist
	if (ImGui::GetCurrentContext() == NULL)
	{
		printf("There is no ImGui Context, exiting program. \n");
		exit(EXIT_FAILURE);
	}

	main_viewport = ImGui::GetMainViewport();

	Message m;
	m.message = "This window contains all the system messages. If something goes " 
				"wrong, please check here for any error messages.";
	addMessage(m);
}

SystemMessageGUI::~SystemMessageGUI()
{
}

void SystemMessageGUI::displaySystemMessageWindow()
{
	bool beginflag = false;

	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 5.0f, main_viewport->WorkPos.y + main_viewport->WorkSize.y - 205.0f), ImGuiCond_FirstUseEver); //(width, height)
	ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(300.0f, 200.0f), ImVec2(500.0f, 800.0f));
	ImGui::Begin("System Message", &beginflag, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

	unsigned int messageNumber = startingMessageNumber;
	for (std::list<Message>::iterator itr = messageList.begin(); itr != messageList.end(); ++itr)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("[%u] ", messageNumber);
		ImGui::PopStyleColor();

		ImGui::SameLine(0.0f, 0.0f);

		ImGui::PushStyleColor(ImGuiCol_Text, (*itr).color);
		ImGui::TextWrapped((*itr).message.c_str());
		ImGui::PopStyleColor();
		++messageNumber;
	}

	if (messageAdded)
	{
		ImGui::SetScrollHereY(1.0f);
		messageAdded = false;
	}

	ImGui::End();
}

void SystemMessageGUI::clearMessageWindow()
{
	messageList.clear();
	startingMessageNumber = 1;
}

void SystemMessageGUI::addMessage(Message message)
{
	if (messageList.size() >= MAX_MESSAGES)
	{
		messageList.pop_front();
		++startingMessageNumber;
	}
	messageList.push_back(message);
	messageAdded = true;
}