////////////////////////////////////////
// EntityConnector.glsl
////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#ifdef VERTEX_SHADER

layout(location=0) in ivec2 entity1Coordinate;
layout(location=1) in ivec2 entity2Coordinate;
layout(location=2) in int highlight;

out TILE_DATA_OUT {
	ivec2 entity1Coordinate;
	ivec2 entity2Coordinate;
	int highlight;
} entityData;

////////////////////////////////////////
// Vertex shader
////////////////////////////////////////

void main()
{
	entityData.entity1Coordinate = entity1Coordinate;
	entityData.entity2Coordinate = entity2Coordinate;
	entityData.highlight = highlight;
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef GEOMETRY_SHADER

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

in TILE_DATA_OUT {
	ivec2 entity1Coordinate;
	ivec2 entity2Coordinate;
	int highlight;
} entityData[];

out SHADING_DATA_OUT {
	int highlight;
} shadingData;

uniform mat4 viewProjMtx = mat4(1);

uniform float tileSize = 10.0;

////////////////////////////////////////
// Geometry shader
////////////////////////////////////////

//Yes you can technically hard coded the vertex data in but.... that's not very flexbile....
void generateConnector()
{
	float thickness = 0.5;
	shadingData.highlight = entityData[0].highlight;

	//Basic model
	//Tile coordinate uses the floor of the x and y value, and therefore correspond to the top left corner of the tile
	//Note that the vertex has been pre-scaled by tileSize

	ivec2 difference = entityData[0].entity2Coordinate - entityData[0].entity1Coordinate;
	difference.y = -difference.y; //Note that the coordinate system with entity has y going downward, so it need to be inverted.
	if(difference.x != 0 || difference.y != 0)
	{
		vec2 normal = vec2(difference.y, -difference.x);
		vec2 halfThicknessVector = normal / sqrt(dot(normal, normal)) * thickness;

		vec4 topLeft = vec4(halfThicknessVector.x, halfThicknessVector.y, 0.0, 1.0);
		vec4 bottomLeft = vec4(-halfThicknessVector.x, -halfThicknessVector.y, 0.0, 1.0);
		vec4 topRight = vec4(difference.x * tileSize * 0.25 + halfThicknessVector.x, difference.y * tileSize * 0.25 + halfThicknessVector.y, 0.0, 1.0);
		vec4 bottomRight = vec4(difference.x * tileSize * 0.25 - halfThicknessVector.x, difference.y * tileSize * 0.25 - halfThicknessVector.y, 0.0, 1.0);

		//Makeshift matrices for model manipulation
		//Entity uses coordinate based on 1/4 of the actual tile size
		//Additionally, the coordinate of the entities start at (4, 4) in actual editor, so it need to be shifted up and left by a full tile
		mat4 translationMtx = mat4(1.0); //You can't translate before rotation!
		translationMtx[3][0] = entityData[0].entity1Coordinate.x * tileSize * 0.25 - tileSize;
		translationMtx[3][1] = entityData[0].entity1Coordinate.y * (-tileSize * 0.25) + tileSize;

		mat4 mvp = viewProjMtx * translationMtx;
		//mat4 mvp = viewProjMtx;

		//All entities will draw a full box
		//Triangle 1
		gl_Position = mvp * topRight;
		EmitVertex();
		gl_Position = mvp * bottomLeft;
		EmitVertex();
		gl_Position = mvp * bottomRight;
		EmitVertex();
		//Every call to this ends 1 triangle_strip, so calling it every
		//time after 3 vertices will gives 1 triangle. Because there is
		//no triangle primitive in geometry shader, this is pretty much
		//a hack to change triangle_strip to triangle.
		EndPrimitive();

		//Triangle 2
		gl_Position = mvp * topRight;
		EmitVertex();
		gl_Position = mvp * topLeft;
		EmitVertex();
		gl_Position = mvp * bottomLeft;
		EmitVertex();
		EndPrimitive();
	}
}

void main()
{
	generateConnector();
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef FRAGMENT_SHADER

in SHADING_DATA_OUT {
	flat int highlight;
} shadingData;

layout(location=0) out vec4 finalColor;
layout(location=1) out vec4 overlapCount; //Used for transparency calculation

////////////////////////////////////////
// Fragment shader
////////////////////////////////////////

void main()
{
	if(shadingData.highlight == 1)
	{
		finalColor = vec4(2.0, 0.0, 0.0, 1.0);
	}
	else
	{
		finalColor = vec4(2.0, 2.0, 2.0, 1.0); //This kinda makes the white color stronger?...
	}
	overlapCount = vec4(1.0);
}

#endif

////////////////////////////////////////////////////////////////////////////////
