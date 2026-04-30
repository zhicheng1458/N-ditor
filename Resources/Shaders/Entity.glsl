////////////////////////////////////////
// Entity.glsl
////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#ifdef VERTEX_SHADER

layout(location=0) in int entityType;
layout(location=1) in ivec2 entityCoordinate;
layout(location=2) in int entityRotation;
layout(location=3) in vec3 entityColor;
layout(location=4) in int highlight;

out ENTITY_DATA_OUT {
	int entityType;
	ivec2 entityCoordinate;
	int entityRotation;
	vec3 entityColor;
	int highlight;
} entityData;

////////////////////////////////////////
// Vertex shader
////////////////////////////////////////

void main()
{
	entityData.entityType = entityType;
	entityData.entityCoordinate = entityCoordinate;
	entityData.entityRotation = entityRotation;
	entityData.entityColor = entityColor;
	entityData.highlight = highlight;
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef GEOMETRY_SHADER

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

in ENTITY_DATA_OUT {
	int entityType;
	ivec2 entityCoordinate;
	int entityRotation;
	vec3 entityColor;
	int highlight;
} entityData[];

out SHADING_DATA_OUT {
	int entityType;
	vec2 textureSampleLocation;
	vec4 color;
	int highlight;
} shadingData;

uniform mat4 rotationMtx45Deg = mat4(1);
uniform mat4 viewProjMtx = mat4(1);

uniform float tileSize = 10.0;

////////////////////////////////////////
// Geometry shader
////////////////////////////////////////

mat4 mtxPow(mat4 base, int power)
{
    mat4 result = mat4(1);
    for(int i = 0; i < power; i++)
    {
        result = base * result; //right-multiply rule
    }
	return result;
}

//Yes you can technically hard coded the vertex data in but.... that's not very flexbile....
void generateEntity()
{
	shadingData.color = vec4(entityData[0].entityColor.xyz, 1.0);
	shadingData.entityType = entityData[0].entityType;
	shadingData.highlight = entityData[0].highlight;

	//Basic model
	//Tile coordinate uses the floor of the x and y value, and therefore correspond to the top left corner of the tile
	//Note that the vertex has been pre-scaled by tileSize
	vec4 topLeft = vec4(tileSize * -0.5, tileSize * 0.5, 0.0, 1.0);
	vec4 topRight = vec4(tileSize * 0.5, tileSize * 0.5, 0.0, 1.0);
	vec4 bottomLeft = vec4(tileSize * -0.5, tileSize * -0.5, 0.0, 1.0);
	vec4 bottomRight = vec4(tileSize * 0.5, tileSize * -0.5, 0.0, 1.0);

	//Makeshift matrices for model manipulation
	//Entity uses coordinate based on 1/4 of the actual tile size
	//Additionally, the coordinate of the entities start at (4, 4) in actual editor, so it need to be shifted up and left by a full tile
	mat4 translationMtx = mat4(1.0); //You can't translate before rotation!
	translationMtx[3][0] = entityData[0].entityCoordinate.x * tileSize * 0.25 - tileSize;
	translationMtx[3][1] = entityData[0].entityCoordinate.y * (-tileSize * 0.25) + tileSize;

	mat4 mvp = viewProjMtx * translationMtx * mtxPow(rotationMtx45Deg, entityData[0].entityRotation);

	//All entities will draw a full box
	//Triangle 1
	gl_Position = mvp * topRight;
	shadingData.textureSampleLocation = vec2(1.0, 1.0);
	EmitVertex();
	gl_Position = mvp * bottomLeft;
	shadingData.textureSampleLocation = vec2(0.0, 0.0);
	EmitVertex();
	gl_Position = mvp * bottomRight;
	shadingData.textureSampleLocation = vec2(1.0, 0.0);
	EmitVertex();
	//Every call to this ends 1 triangle_strip, so calling it every
	//time after 3 vertices will gives 1 triangle. Because there is
	//no triangle primitive in geometry shader, this is pretty much
	//a hack to change triangle_strip to triangle.
	EndPrimitive();

	//Triangle 2
	gl_Position = mvp * topRight;
	shadingData.textureSampleLocation = vec2(1.0, 1.0);
	EmitVertex();
	gl_Position = mvp * topLeft;
	shadingData.textureSampleLocation = vec2(0.0, 1.0);
	EmitVertex();
	gl_Position = mvp * bottomLeft;
	shadingData.textureSampleLocation = vec2(0.0, 0.0);
	EmitVertex();
	EndPrimitive();
}

void main()
{
	generateEntity();
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef FRAGMENT_SHADER

in SHADING_DATA_OUT {
	flat int entityType;
	vec2 textureSampleLocation;
	vec4 color;
	flat int highlight;
} shadingData;

uniform float tileSize = 10.0;
uniform sampler2D entitySpriteSheet; //Texture layout: bottom left is (0, 0), top right is (1, 1)

//ENTITY SPRITE SHEET LAYOUT
//entity of ID between 00-1C currently in use (aka 0 - 28 as base 10 value)

uniform int textureWidth = 400;
uniform int textureHeight = 400;
uniform int widthPerEntity = 50;

layout(location=0) out vec4 finalColor;
layout(location=1) out vec4 overlapCount; //Used for transparency calculation

////////////////////////////////////////
// Fragment shader
////////////////////////////////////////

void main()
{
	float numRows = textureHeight / widthPerEntity;
	float numCols = textureWidth / widthPerEntity;
	float entitySizeOnSpriteSheet = 1.0 / numRows; //Expressed as proportion due to texture coordinate range from 0 to 1

	//glTexSubImage2D loads data using origin on the bottom left corner, then go right and up.
	//However, stbi usually obtain the texture data from image starting from the top left corner,
	//resulting in an texture that is upside down. Therefore, the y coordinate needs to be inverted. (use 1 - actual value)
	//Alternatively, use stbi_set_flip_vertically_on_load(true) on the texture data before using stbi

	vec2 sampleCoord = vec2((mod(shadingData.entityType,numCols) + shadingData.textureSampleLocation.x) * entitySizeOnSpriteSheet,
							(floor(shadingData.entityType / numRows) + shadingData.textureSampleLocation.y) * entitySizeOnSpriteSheet);
	finalColor = texture(entitySpriteSheet, sampleCoord);
	if(shadingData.highlight == 1)
	{
		finalColor.xyz += vec3(0.5, 0.5, 0.0) * finalColor[3]; //Yellow highlight, multiply by alpha value due to the texture uses GL_LINEAR as filter.
	}
	if(finalColor[3] == 0) {discard;} //Clear background
	overlapCount = vec4(1.0);
}

#endif

////////////////////////////////////////////////////////////////////////////////
