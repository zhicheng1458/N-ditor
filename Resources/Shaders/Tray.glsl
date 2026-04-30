////////////////////////////////////////
// Tray.glsl
////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#ifdef VERTEX_SHADER

layout(location=0) in vec2 position;
layout(location=1) in vec2 textureCoord;
layout(location=2) in vec4 color;

out SHADING_DATA_OUT {
	vec2 position;
	vec2 textureCoord;
	vec4 color;
} vertexData;

uniform float screenWidth = 1600.0;
uniform float screenHeight = 900.0;

uniform mat4 projMtx;

////////////////////////////////////////
// Vertex shader
////////////////////////////////////////

void main()
{
	gl_Position = vec4(2.0 * position.x / screenWidth - 1.0, 1.0f - (2.0f * position.y / screenHeight), 0.0 ,1.0);

	vertexData.position = position;
	vertexData.textureCoord = textureCoord;
	vertexData.color = color;
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef FRAGMENT_SHADER

in SHADING_DATA_OUT {
	vec2 position;
	vec2 textureCoord;
	vec4 color;
} vertexData;

layout(location=0) out vec4 finalColor;
layout(location=1) out vec4 overlapCount; //Used for transparency calculation

////////////////////////////////////////
// Fragment shader
////////////////////////////////////////

void main()
{
	finalColor = glm::vec4(vertexData.color.xyz, vertexData.color.a);
	overlapCount = vec4(1.0);
}

#endif

////////////////////////////////////////////////////////////////////////////////
