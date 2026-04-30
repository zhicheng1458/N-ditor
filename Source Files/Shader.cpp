#include "Shader.h"

#include <string>
#include <fstream>

////////////////////////////////////////////////////////////////////////////////
// Shader
////////////////////////////////////////////////////////////////////////////////

Shader::Shader(const char * filename, ShaderType type)
{
	// Open shader file
	std::string fileData = "";
	std::fstream stream(filename, std::ios::in);
	if(!stream.is_open())
	{
		printf("ERROR: Can't open shader file '%s'\n", filename);
		shaderID=0;
		return;
	}

	// Read shader file
	std::string line = "";
	while (getline(stream, line))
	{
		fileData += "\n" + line;
	}

	// Close file
	stream.close();

	// Append header & create shader
	std::string shaderSource;
	switch(type) {
		case eGeometry:
			shaderSource = "#define GEOMETRY_SHADER\n"+fileData;
			shaderID = glCreateShader(GL_GEOMETRY_SHADER);
			break;
		case eVertex:
			shaderSource = "#define VERTEX_SHADER\n"+fileData;
			shaderID = glCreateShader(GL_VERTEX_SHADER);
			break;
		case eFragment:
			shaderSource = "#define FRAGMENT_SHADER\n"+fileData;
			shaderID = glCreateShader(GL_FRAGMENT_SHADER);
			break;
		case eCompute:
			shaderSource = "#define COMPUTE_SHADER\n"+fileData;
			shaderID = glCreateShader(GL_COMPUTE_SHADER);
			break;
	}
	shaderSource = "#version 460\n" + shaderSource;

	// Compile shader
	const char* rawShaderSource = shaderSource.c_str();
	glShaderSource(shaderID, 1, &rawShaderSource, NULL);
	glCompileShader(shaderID);

	// Verify compile worked
	GLint isCompiled = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		printf("ERROR: Can't compile shader '%s'\n",filename);

		// Print error message
		GLint maxLength = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);
		printf("GL ERRORS:\n%s\n",&errorLog[0]);

		// Delete shader
		glDeleteShader(shaderID);
		shaderID=0;
	}
}

Shader::~Shader()
{
	glDeleteShader(shaderID);
}

////////////////////////////////////////////////////////////////////////////////
// ShaderProgram (Think program as a bunch of shaders working together)
////////////////////////////////////////////////////////////////////////////////

ShaderProgram::ShaderProgram(const char *filename,ProgramType type)
{
	this->type = type;
	for (int i = 0; i < eMaxShaders; i++)
	{
		shaders[i] = 0; //Create empty shader pointer
	}

	// Load & compile shaders
	if(type == eGeometry)
	{
		shaders[0] = new Shader(filename, Shader::eVertex);
		shaders[1] = new Shader(filename, Shader::eGeometry);
		shaders[2] = new Shader(filename, Shader::eFragment);
	}
	else if(type == eRender)
	{
		shaders[0] = new Shader(filename, Shader::eVertex);
		shaders[1] = new Shader(filename, Shader::eFragment);
	}
	else if(type == eCompute)
	{
		shaders[0] = new Shader(filename, Shader::eCompute);
	}

	// Link shader program
	bool shaderCompileFailed = false;
	programID = glCreateProgram();
	for(int i = 0; i < eMaxShaders; i++)
	{
		if (shaders[i] == 0)
		{
			break;
		}

		if (shaders[i]->getShaderID() == 0)
		{
			shaderCompileFailed = true;
		}
		else
		{
			glAttachShader(programID, shaders[i]->getShaderID());
		}
	}

	if (!shaderCompileFailed)
	{
		glLinkProgram(programID);
		GLint isCompiled = 0;
		glGetProgramiv(programID, GL_LINK_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			printf("ERROR: Can't compile program '%s'\n", filename);

			// Print error message
			GLint maxLength = 0;
			glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> errorLog(maxLength);
			glGetProgramInfoLog(programID, maxLength, &maxLength, &errorLog[0]);
			printf("GL ERRORS:\n%s\n", &errorLog[0]);

			shaderCompileFailed = true;
		}
	}

	if (shaderCompileFailed)
	{
		// Delete shader and program
		for (int i = 0; i < eMaxShaders; i++)
		{
			delete shaders[i];
		}
		glDeleteProgram(programID);
		programID = 0;
	}
	shaderName = filename;
}


ShaderProgram::~ShaderProgram()
{
	for (int i = 0; i < eMaxShaders; i++)
	{
		delete shaders[i];
	}
	glDeleteProgram(programID);
}

