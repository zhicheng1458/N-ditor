////////////////////////////////////////
// Model.glsl
////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#ifdef VERTEX_SHADER

layout(location=0) in vec3 Position;
layout(location=1) in vec2 TextureCoordinate;
layout(location=2) in vec4 Color;

out vec2 textureCoord;
out vec4 color;

uniform mat4 modelMtx=mat4(1);
uniform mat4 viewProjMtx=mat4(1);

////////////////////////////////////////
// Vertex shader
////////////////////////////////////////

void main() {

	gl_Position = viewProjMtx * modelMtx * vec4(Position,1);

	textureCoord = TextureCoordinate;
	color = Color;
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef FRAGMENT_SHADER

in vec2 textureCoord;
in vec4 color;

/*
uniform int useTexture = 0;
uniform sampler2D textureBindingLocation[10]; //Each 'value' in the array point to which GLTEXTURE_X it is (sampler2D don't actually have value)
uniform int isActiveTexture[10];
*/

out vec4 finalColor;

////////////////////////////////////////
// Fragment shader
////////////////////////////////////////
void main()
{
	finalColor = color;

	/*
	if(isActiveTexture[0] > 0)
	{
		vec4 textureColor = texture(textureBindingLocation[0], textureCoord);
		if(textureColor.a < 0.1)
			discard;
		finalColor = mix(textureColor, finalColor, 0.5);
	}
	*/
}

#endif

////////////////////////////////////////////////////////////////////////////////
