#include "Model.h"

Model::Model()
{
	glGenBuffers(1,&vertexBuffer);
	glGenBuffers(1,&indexBuffer);

	count = 0;
}

Model::~Model()
{
	if (indexBuffer) { glDeleteBuffers(1, &indexBuffer); }
	if (vertexBuffer) { glDeleteBuffers(1, &vertexBuffer); }
}

void Model::draw(const glm::mat4 & modelMtx, const glm::mat4 & viewProjMtx, uint shader)
{
	//Since MANY shader may reuse this same draw command, layout location on
	//all the shader that use this draw command must be consistent.

	if (allowModify)
	{
		fprintf(stderr, "Drawing may not be proceed as buffer is not set. Ending draw function \n");
		return;
	}

	// Set up shader
	glDisable(GL_CULL_FACE);
	//glEnable(GL_CULL_FACE);
	glUseProgram(shader);

	//Optimization: Let camera send view projection matrix to shader instead of sending it here on a per object basis.
	glUniformMatrix4fv(glGetUniformLocation(shader, "modelMtx"), 1, false, (float*)&modelMtx);
	glUniformMatrix4fv(glGetUniformLocation(shader, "viewProjMtx"), 1, false, (float*)&viewProjMtx);

	// Set up state
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	uint posLoc = 0;
	glEnableVertexAttribArray(posLoc);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(0 * sizeof(float)));

	//These are really just hacks to send non-vertex information to the vertex shader, because the vertex shader expects vertices....
	uint textureLoc = 1;
	glEnableVertexAttribArray(textureLoc);
	glVertexAttribPointer(textureLoc, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(3 * sizeof(float)));

	uint colorLoc = 2;
	glEnableVertexAttribArray(colorLoc);
	glVertexAttribPointer(colorLoc, 4, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)(5 * sizeof(float)));

	// Draw geometry
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Only draw the outline of the shape/triangles
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Draw the entire shape/triangles
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);

	// Clean up state
	glDisableVertexAttribArray(posLoc);
	glDisableVertexAttribArray(textureLoc);
	glDisableVertexAttribArray(colorLoc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnable(GL_CULL_FACE);

	glUseProgram(0);
}

////////////////////////////////////////////////////////////////////////////////

void Model::addLine(const glm::vec2 & startLoc, const glm::vec4 & startColor,
					const glm::vec2 & endLoc,   const glm::vec4 & endColor,
					float thickness)
{
	if (!allowModify)
	{
		fprintf(stderr, "You may not modify buffer list once drawing occurs. Please clear buffer before attempting again. \n");
		return;
	}

	float halfThickness = thickness / 2.0f;
	glm::vec2 direction = endLoc - startLoc;
	glm::vec2 normal = glm::normalize(glm::vec2(direction.y, -direction.x));
	//glm::vec2 thicknessVector = halfThickness * normal;
	glm::vec2 bottomLeft = startLoc - halfThickness * normal;
	glm::vec2 bottomRight = startLoc + halfThickness * normal;
	glm::vec2 topLeft = endLoc - halfThickness * normal;
	glm::vec2 topRight = endLoc + halfThickness * normal;

	uint vertexCount = (uint)vtx.size();

	vtx.push_back({ {bottomLeft.x,	bottomLeft.y,	0.0f},	{-1.0f, -1.0f}, {startColor.x, startColor.y, startColor.z, startColor.w} });
	vtx.push_back({ {bottomRight.x, bottomRight.y,	0.0f},	{-1.0f, -1.0f}, {startColor.x, startColor.y, startColor.z, startColor.w} });
	vtx.push_back({ {topLeft.x,		topLeft.y,		0.0f},	{-1.0f, -1.0f}, {endColor.x, endColor.y, endColor.z, endColor.w} });
	vtx.push_back({ {topRight.x,	topRight.y,		0.0f},	{-1.0f, -1.0f}, {endColor.x, endColor.y, endColor.z, endColor.w} });

	idx.push_back(vertexCount + (uint)0); idx.push_back(vertexCount + (uint)1); idx.push_back(vertexCount + (uint)2);
	idx.push_back(vertexCount + (uint)2); idx.push_back(vertexCount + (uint)1); idx.push_back(vertexCount + (uint)3);
}

void Model::addTriangle(const glm::vec2 &v1, const glm::vec2 &v2, const glm::vec2 &v3,
						const glm::vec4 &c1, const glm::vec4 &c2, const glm::vec4 &c3,
						const glm::vec2 &t1, const glm::vec2 &t2, const glm::vec2 &t3)
{
	if (!allowModify)
	{
		fprintf(stderr, "You may not modify buffer list once drawing occurs. Please clear buffer before attempting again. \n");
		return;
	}

	uint vertexCount = (uint)vtx.size();

	vtx.push_back({ {v1.x, v1.y, 0.0f}, {t1.x, t1.y}, {c1.x, c1.y, c1.z, c1.w} });
	vtx.push_back({ {v2.x, v2.y, 0.0f}, {t2.x, t2.y}, {c2.x, c2.y, c2.z, c2.w} });
	vtx.push_back({ {v3.x, v3.y, 0.0f}, {t3.x, t3.y}, {c3.x, c3.y, c3.z, c3.w} });

	idx.push_back(vertexCount + (uint)0); idx.push_back(vertexCount + (uint)1); idx.push_back(vertexCount + (uint)2);
}

const bool & Model::canBeModified() const
{
	return allowModify;
}

const uint & Model::getVertexBuffer() const
{
	return vertexBuffer;
}

const uint & Model::getIndexBuffer() const
{
	return indexBuffer;
}

////////////////////////////////////////////////////////////////////////////////

void Model::setBuffers()
{
	count = (uint)idx.size();

	if (count != 0)
	{
		// Store vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vtx.size() * sizeof(ModelVertex), &vtx[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Store index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(uint), &idx[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	allowModify = false;
}

void Model::clearBuffers()
{
	vtx.clear();
	idx.clear();

	count = 0;

	allowModify = true;
}

////////////////////////////////////////////////////////////////////////////////
