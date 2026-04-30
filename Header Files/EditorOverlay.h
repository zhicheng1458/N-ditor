#pragma once
#include "Core.h"
#include "Shader.h"
#include "Texture.h"
#include "EntityHandler.h"

struct ScreenBasedModelVtx
{
	glm::vec2 position;
	glm::vec2 textureCoord;
	glm::vec4 color;
	int entityType;
};

class EditorOverlay
{
public:
	EditorOverlay(float screenWidth, float screenHeight, float tileSize);
	~EditorOverlay();

	void showTray();
	void hideTray();
	void scrollTray(float offset);
	bool wantControl(float mouseX, float mouseY);
	void processMouseLocation(float mouseX, float mouseY);
	entityType getSelectedEntityType();

	void update();
	void drawSolidObj();
	void drawTransparentObj();
	void resize(float screenWidth, float screenHeight);

private:

	entityType lastSelectedEntityType = MINE;

	ShaderProgram* trayShader;
	ShaderProgram* entitySelectorShader;

	//const char* SPRITESHEET_PATH = "./Resources/Textures/EntitiesSpriteSheetV2.png";
	Texture* entitySpriteSheet;
	const int numRows = 8;
	const int numCols = 8;

	void buildOverlay();
	void setBuffers();
	void clearBuffers();

	float screenWidth; //For entity pannel, will ignore view projection mtx and work in screen space
	float screenHeight; //For entity pannel, will ignore view projection mtx and work in screen space
	float tileSize; //For cursor indicator
	const float traySize = 80.0f;
	const float trayBorderSize = 10.0f;
	float trayScrollMinOffset = 0.0f;
	float trayScrollMaxOffset = 100.0f;
	float currentScroll = 0.0f;
	bool drawTray = false;

	glm::mat4 screenProjMtx;

	uint trayVertexBuffer;
	uint trayIndexBuffer;
	uint entitySelectorVertexBuffer;
	uint entitySelectorIndexBuffer;

	std::vector<ScreenBasedModelVtx> trayVtx;
	std::vector<uint> trayIdx;
	std::vector<ScreenBasedModelVtx> entitySelectorVtx;
	std::vector<uint> entitySelectorIdx;

	void addTriangle(const glm::vec2& v1, const glm::vec2& v2, const glm::vec2& v3, //Coordinate. Z will be stored as depth
					 const glm::vec4& c1, const glm::vec4& c2, const glm::vec4& c3, //Color
					 const glm::vec2& t1, const glm::vec2& t2, const glm::vec2& t3,
					 int type, std::vector<ScreenBasedModelVtx> & vtx, std::vector<uint> & idx);//TextureCoord (in vec2(x,y))
};