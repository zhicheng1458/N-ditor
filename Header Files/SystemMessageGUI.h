#pragma once
#include "imconfig.h"
#include "imgui.h"
#include <string>
#include <list>

#include "Message.h"

class SystemMessageGUI
{
	public:
		SystemMessageGUI();
		~SystemMessageGUI();

		void displaySystemMessageWindow();
		void clearMessageWindow();

		//Helper for pushing a message to the window to display
		void addMessage(Message message);

	private:
		unsigned int startingMessageNumber = 1;

		std::list<Message> messageList;
		const unsigned int MAX_MESSAGES = 50;
		bool messageAdded = false; //Used to scroll the window whenever new message is added

		ImGuiViewport * main_viewport;
};