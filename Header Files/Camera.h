////////////////////////////////////////
// Camera.h
////////////////////////////////////////

#pragma once

#include "Core.h"

////////////////////////////////////////////////////////////////////////////////

class Camera {
public:
	Camera(float screenWidth, float screenHeight);

	void sendCameraToWorldSpace(uint programID);
	void update(const glm::vec2 & cameraLocation);
	void reset();

	//void setFocusLocation();
	void changeFOV(float newWidth, float newHeight);
	void changeZoom(float additiveMultiplier);
	void move(float screenPixelDx, float screenPixelDy);

	// Access functions
	const glm::mat4 & getViewMtx() const		{return viewMtx; }
	const glm::mat4 & getProjectionMtx() const	{return projectionMtx; }
	const glm::mat4 & getViewProjectMtx() const	{return viewProjectMtx;}
	const glm::vec2 & getViewPosition() const	{return viewPosition; }

private:
	// Perspective controls
	float viewWidth;
	float viewHeight;
	glm::mat4 projectionMtx;	//Perspective usually wouldn't change unless resize is called
	glm::mat4 viewMtx;
	glm::mat4 viewProjectMtx;
	glm::vec3 viewPosition;

	// Polar controls
	float zoomMultiplier;		// Distance of the camera eye position to the origin (meters)
	const float maxZoomMultiplier = 2.0f; // Biggest view
	const float minZoomMultiplier = 0.1f; // Smallest view

	// World representation of the camera
	glm::mat4 world;

	//Keyboard and Mouse
	bool leftDown, middleDown, rightDown;
	float MouseX, MouseY; //For purely tracking mouse location
	bool keyStates[GLFW_KEY_LAST + 1];
};

////////////////////////////////////////////////////////////////////////////////
