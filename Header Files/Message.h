#pragma once
#include "Core.h"
#include <string>

namespace CommonMessageColor
{
	const glm::vec4 SUCCESS_COLOR = glm::vec4(0.1f, 0.9f, 0.1f, 1.0f); //Green
	const glm::vec4 ERROR_COLOR = glm::vec4(0.9f, 0.1f, 0.1f, 1.0f);   //Red
	const glm::vec4 NEUTRAL_COLOR = glm::vec4(0.9f, 0.9f, 0.1f, 1.0f); //Yellow
};

struct Message
{
	std::string message = "";
	glm::vec4 color = glm::vec4(1.0f);
};
