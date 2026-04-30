#pragma once

#include "Core.h"

////////////////////////////////////////////////////////////////////////////////
// Shader
////////////////////////////////////////////////////////////////////////////////

class Shader
{
public:
	enum ShaderType
	{	
		eGeometry = 0,
		eVertex = 1,
		eFragment = 2,
		eCompute = 3
	};

	Shader(const char *filename, ShaderType type);
	~Shader();

	uint getShaderID() {return shaderID;}

private:
	uint shaderID;
};

////////////////////////////////////////////////////////////////////////////////
// ShaderProgram (Think program as a bunch of shaders working together)
////////////////////////////////////////////////////////////////////////////////

class ShaderProgram
{
public:
	enum ProgramType
	{
		eGeometry = 0,
		eRender = 1,
		eCompute = 2
	};

	ShaderProgram(const char *filename,ProgramType type);
	~ShaderProgram();

	uint getProgramID() const				{return programID;}
	const char * getShaderName() const		{return shaderName;}

private:
	enum
	{
		eMaxShaders = 3
	};

	ProgramType type;
	Shader * shaders[eMaxShaders];
	uint programID = 0;
	const char * shaderName;
};
