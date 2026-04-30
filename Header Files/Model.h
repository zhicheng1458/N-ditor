#pragma once

#include "Core.h"
#include "Texture.h"
//#include "Tokenizer.h" //Will need later for map loading

struct ModelVertex
{
	glm::vec3 position;
	glm::vec2 textureCoord;
	glm::vec4 color;
};

class Model {
public:
	Model();
	~Model();

	//In general use custom draw command, this only provide a very basic variant
	void draw(const glm::mat4 &modelMtx, const glm::mat4 &viewProjMtx, uint shader);

	void addLine(const glm::vec2 & startLoc, const glm::vec4 & startColor,
				 const glm::vec2 & endLoc,	 const glm::vec4 & endColor,
				 float thickness);
	void addTriangle(const glm::vec2 &v1, const glm::vec2 &v2, const glm::vec2 &v3, //Coordinate. Z will be stored as depth
					 const glm::vec4 &c1, const glm::vec4 &c2, const glm::vec4 &c3, //Color
					 const glm::vec2 &t1, const glm::vec2 &t2, const glm::vec2 &t3);//TextureCoord (in vec2(x,y))

	//void loadFromFile(const char * fileName);

	const bool & canBeModified() const;
	const uint & getVertexBuffer() const;
	const uint & getIndexBuffer() const;
	
	void setBuffers();
	void clearBuffers();

private:
	uint vertexBuffer;
	uint indexBuffer;

	std::vector<ModelVertex> vtx;
	std::vector<uint> idx;

	bool allowModify = true;

	uint count;
};
