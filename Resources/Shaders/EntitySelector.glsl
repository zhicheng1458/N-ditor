////////////////////////////////////////
// EntitySelector.glsl
////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#ifdef VERTEX_SHADER

layout(location=0) in vec2 position;
layout(location=1) in vec2 textureCoord;
layout(location=2) in int entityType;

out SHADING_DATA_OUT {
	vec2 position;
	vec2 textureCoord;
	int entityType;
} vertexData;

uniform float screenWidth = 1600.0;
uniform float screenHeight = 900.0;

uniform float scrollValue = 0.0;

////////////////////////////////////////
// Vertex shader
////////////////////////////////////////

void main()
{
	gl_Position = vec4(2.0 * (position.x - scrollValue) / screenWidth - 1.0, 1.0f - (2.0f * position.y / screenHeight), 0.0 ,1.0);

	vertexData.position = position;
	vertexData.textureCoord = textureCoord;
	vertexData.entityType = entityType;
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef FRAGMENT_SHADER

in SHADING_DATA_OUT {
	vec2 position;
	vec2 textureCoord;
	flat int entityType;
} vertexData;

uniform sampler2D entitySpriteSheet; //Texture layout: bottom left is (0, 0), top right is (1, 1)

layout(location=0) out vec4 finalColor;
layout(location=1) out vec4 overlapCount; //Used for transparency calculation

uniform int typeToHighlight;

////////////////////////////////////////
// Fragment shader
////////////////////////////////////////

void main()
{
	finalColor = texture(entitySpriteSheet, vertexData.textureCoord);
	if(vertexData.entityType == typeToHighlight)
	{
		finalColor.xyz += vec3(0.3, 0.5, 0.1) * finalColor.a; //Yellow highlight, multiply by alpha value due to the texture uses GL_LINEAR as filter.
		finalColor += vec4(0.2, 0.2, 0.0, 0.2);
	}
	//if(finalColor[3] == 0) {discard;} //Clear background
	overlapCount = vec4(1.0);
}

#endif

////////////////////////////////////////////////////////////////////////////////
