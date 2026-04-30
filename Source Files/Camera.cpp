////////////////////////////////////////
// Camera.cpp
////////////////////////////////////////

#include "Camera.h"

////////////////////////////////////////////////////////////////////////////////

Camera::Camera(float screenWidth, float screenHeight) {
	viewWidth = screenWidth;
	viewHeight = screenHeight;

	reset();
}

////////////////////////////////////////////////////////////////////////////////

void Camera::sendCameraToWorldSpace(uint programID)
{
	glUseProgram(programID);
	glUniformMatrix4fv(glGetUniformLocation(programID, "ViewProjMtx"), 1, false, (float*)&viewProjectMtx);
	glUseProgram(0);
}

void Camera::update(const glm::vec2 & offset) {

	//Update new view position
	viewPosition = viewPosition + glm::vec3(offset, 0.0f);

	// Compute camera world matrix
	world = glm::translate(glm::mat4(1.0f), glm::vec3(viewPosition.x, viewPosition.y, 0.0f));

	// Compute view matrix (inverse of world matrix)
	viewMtx=glm::inverse(world);

	// Compute final view-projection matrix
	viewProjectMtx = projectionMtx * viewMtx;

}

////////////////////////////////////////////////////////////////////////////////

void Camera::reset()
{
	zoomMultiplier = 1.0f;
	viewPosition = glm::vec3(0.0f, 0.0f, 5.0f); //Focus on origin
	viewMtx = glm::mat4(1.0f);
	world = glm::mat4(1.0f);
	projectionMtx = glm::ortho(-viewWidth * 0.5f * zoomMultiplier, viewWidth * 0.5f * zoomMultiplier, -viewHeight * 0.5f * zoomMultiplier, viewHeight * 0.5f * zoomMultiplier);
	this->update(glm::vec2(0.0f));

	// Keyboard and Mouse control
	leftDown = false;
	MouseX = MouseY = 0.0f;
}

void Camera::changeZoom(float additiveMultiplier)
{
	zoomMultiplier = glm::clamp(zoomMultiplier + additiveMultiplier, minZoomMultiplier, maxZoomMultiplier);
	projectionMtx = glm::ortho(-viewWidth * 0.5f * zoomMultiplier, viewWidth * 0.5f * zoomMultiplier, -viewHeight * 0.5f * zoomMultiplier, viewHeight * 0.5f * zoomMultiplier);
	this->update(glm::vec2(0.0f));
}

void Camera::move(float screenPixelDx, float screenPixelDy)
{
	this->update(glm::vec2(screenPixelDx * zoomMultiplier, screenPixelDy * zoomMultiplier));
}

////////////////////////////////////////////////////////////////////////////////

void Camera::changeFOV(float newWidth, float newHeight)
{
	viewWidth = newWidth;
	viewHeight = newHeight;
	projectionMtx = glm::ortho(-viewWidth * 0.5f * zoomMultiplier, viewWidth * 0.5f * zoomMultiplier, -viewHeight * 0.5f * zoomMultiplier, viewHeight * 0.5f * zoomMultiplier);
	this->update(glm::vec2(0.0f));
}
