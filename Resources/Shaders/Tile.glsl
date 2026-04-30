////////////////////////////////////////
// Tile.glsl
////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#ifdef VERTEX_SHADER

layout(location=0) in int tileType;
layout(location=1) in ivec2 tileCoordinate;
layout(location=2) in int tileRotation;
layout(location=3) in vec3 tileColor;

out TILE_DATA_OUT {
	int tileType;
	ivec2 tileCoordinate;
	int tileRotation;
	vec3 tileColor;
} tileData;

////////////////////////////////////////
// Vertex shader
////////////////////////////////////////

void main()
{

	tileData.tileType = tileType;
	tileData.tileCoordinate = tileCoordinate;
	tileData.tileRotation = tileRotation;
	tileData.tileColor = tileColor;
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef GEOMETRY_SHADER

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

in TILE_DATA_OUT {
	int tileType;
	ivec2 tileCoordinate;
	int tileRotation;
	vec3 tileColor;
} tileData[];

out SHADING_DATA_OUT {
	vec4 color;
	int isCurveTile;
	vec2 fragPosition;
} shadingData;

uniform mat4 viewProjMtx = mat4(1);

uniform float tileSize = 10.0;

////////////////////////////////////////
// Geometry shader
////////////////////////////////////////

//Yes you can technically hard coded the vertex data in but.... that's not very flexbile....
void generateTile()
{
	shadingData.color = vec4(tileData[0].tileColor, 1.0);
	shadingData.isCurveTile = 0; //value of -1 means discard inner, value of 1 means discard outer.
	shadingData.fragPosition = vec2(0.0);

	//Basic model
	//Tile coordinate uses the floor of the x and y value, and therefore correspond to the top left corner of the tile
	//Note that the vertex has been pre-scaled by tileSize
	vec4 topLeft = vec4(tileSize * -0.5, tileSize * 0.5, 0.0, 1.0);
	vec4 topRight = vec4(tileSize * 0.5, tileSize * 0.5, 0.0, 1.0);
	vec4 bottomLeft = vec4(tileSize * -0.5, tileSize * -0.5, 0.0, 1.0);
	vec4 bottomRight = vec4(tileSize * 0.5, tileSize * -0.5, 0.0, 1.0);

	//Makeshift matrices for model manipulation
	mat4 rotationMtx90Deg = mat4(1.0);
	rotationMtx90Deg[1][0] = 1.0;
	rotationMtx90Deg[0][1] = -1.0;
	rotationMtx90Deg[0][0] = 0.0;
	rotationMtx90Deg[1][1] = 0.0;

	mat4 translationMtx = mat4(1.0); //You can't translate before rotation!
	translationMtx[3][0] = tileData[0].tileCoordinate.x * tileSize + tileSize * 0.5;
	translationMtx[3][1] = tileData[0].tileCoordinate.y * (-tileSize) + (-tileSize) * 0.5;

	mat4 mvp = mat4(1.0);

	//Determine rotation
	if(tileData[0].tileRotation == 0)
	{
		mvp = viewProjMtx * translationMtx;
	}
	if(tileData[0].tileRotation == 1)
	{
		mvp = viewProjMtx * translationMtx * rotationMtx90Deg;
	}
	if(tileData[0].tileRotation == 2)
	{
		mvp = viewProjMtx * translationMtx * rotationMtx90Deg * rotationMtx90Deg;
	}
	if(tileData[0].tileRotation == 3)
	{
		mvp = viewProjMtx * translationMtx * rotationMtx90Deg * rotationMtx90Deg * rotationMtx90Deg;
	}

	//Full tile
	if(tileData[0].tileType == 0)
	{
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
	
	//45 deg slope
	if(tileData[0].tileType == 1)
	{
		//Triangle 1
		gl_Position = mvp * topRight;
		EmitVertex();
		gl_Position = mvp * bottomLeft;
		EmitVertex();
		gl_Position = mvp * bottomRight;
		EmitVertex();
		EndPrimitive();
	}

	//60 deg small slope, vertical half block
	if(tileData[0].tileType == 2)
	{
		if(tileData[0].tileRotation == 0 || tileData[0].tileRotation == 2)
		{
			bottomLeft = vec4(0.0, tileSize * -0.5, 0.0, 1.0);
		}
		else if(tileData[0].tileRotation == 1 || tileData[0].tileRotation == 3)
		{
			topRight = vec4(tileSize * 0.5, 0.0, 0.0, 1.0);
		}

		//Triangle 1
		gl_Position = mvp * topRight;
		EmitVertex();
		gl_Position = mvp * bottomLeft;
		EmitVertex();
		gl_Position = mvp * bottomRight;
		EmitVertex();
		EndPrimitive();
	}

	//60 deg small slope, horizontal half block
	if(tileData[0].tileType == 3)
	{
		if(tileData[0].tileRotation == 1 || tileData[0].tileRotation == 3)
		{
			bottomLeft = vec4(0.0, tileSize * -0.5, 0.0, 1.0);
		}
		else if(tileData[0].tileRotation == 0 || tileData[0].tileRotation == 2)
		{
			topRight = vec4(tileSize * 0.5, 0.0, 0.0, 1.0);
		}

		//Triangle 1
		gl_Position = mvp * topRight;
		EmitVertex();
		gl_Position = mvp * bottomLeft;
		EmitVertex();
		gl_Position = mvp * bottomRight;
		EmitVertex();
		EndPrimitive();
	}

	//convex curve, need special instruction in fragment shader
	if(tileData[0].tileType == 4)
	{
		shadingData.isCurveTile = -1;

		//Triangle 1
		shadingData.fragPosition = topRight.xy;
		gl_Position = mvp * topRight;
		EmitVertex();
		shadingData.fragPosition = bottomLeft.xy;
		gl_Position = mvp * bottomLeft;
		EmitVertex();
		shadingData.fragPosition = bottomRight.xy;
		gl_Position = mvp * bottomRight;
		EmitVertex();
		EndPrimitive();
	}

	//Half block
	if(tileData[0].tileType == 5)
	{
		topRight = vec4(0.5, tileSize * 0.5, 0.0, 1.0);
		bottomRight = vec4(0.5, -tileSize * 0.5, 0.0, 1.0);

		//Triangle 1
		gl_Position = mvp * topRight;
		EmitVertex();
		gl_Position = mvp * bottomLeft;
		EmitVertex();
		gl_Position = mvp * bottomRight;
		EmitVertex();
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

	//60 deg big slope, vertical half block
	if(tileData[0].tileType == 6)
	{
		if(tileData[0].tileRotation == 0 || tileData[0].tileRotation == 2)
		{
			topLeft = vec4(0.0, tileSize * 0.5, 0.0, 1.0);
		}
		else if(tileData[0].tileRotation == 1 || tileData[0].tileRotation == 3)
		{
			topLeft = vec4(-tileSize * 0.5, 0.0, 0.0, 1.0);
		}

		//Triangle 1
		gl_Position = mvp * topRight;
		EmitVertex();
		gl_Position = mvp * bottomLeft;
		EmitVertex();
		gl_Position = mvp * bottomRight;
		EmitVertex();
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

	//60 deg big slope, horizontal half block
	if(tileData[0].tileType == 7)
	{
		if(tileData[0].tileRotation == 1 || tileData[0].tileRotation == 3)
		{
			topLeft = vec4(0.0, tileSize * 0.5, 0.0, 1.0);
		}
		else if(tileData[0].tileRotation == 0 || tileData[0].tileRotation == 2)
		{
			topLeft = vec4(-tileSize * 0.5, 0.0, 0.0, 1.0);
		}

		//Triangle 1
		gl_Position = mvp * topRight;
		EmitVertex();
		gl_Position = mvp * bottomLeft;
		EmitVertex();
		gl_Position = mvp * bottomRight;
		EmitVertex();
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

	//concave curve, need special instruction in fragment shader
	if(tileData[0].tileType == 8)
	{
		shadingData.isCurveTile = 1;

		//Triangle 1
		shadingData.fragPosition = topRight.xy;
		gl_Position = mvp * topRight;
		EmitVertex();
		shadingData.fragPosition = bottomLeft.xy;
		gl_Position = mvp * bottomLeft;
		EmitVertex();
		shadingData.fragPosition = bottomRight.xy;
		gl_Position = mvp * bottomRight;
		EmitVertex();
		EndPrimitive();

		//Triangle 2
		shadingData.fragPosition = topRight.xy;
		gl_Position = mvp * topRight;
		EmitVertex();
		shadingData.fragPosition = topLeft.xy;
		gl_Position = mvp * topLeft;
		EmitVertex();
		shadingData.fragPosition = bottomLeft.xy;
		gl_Position = mvp * bottomLeft;
		EmitVertex();
		EndPrimitive();
	}
}

void main()
{
	generateTile();
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef FRAGMENT_SHADER

in SHADING_DATA_OUT {
	vec4 color;
	flat int isCurveTile;
	vec2 fragPosition;
} shadingData;

uniform float tileSize = 10.0;

out vec4 finalColor;

////////////////////////////////////////
// Fragment shader
////////////////////////////////////////
void main()
{
	if(shadingData.isCurveTile == -1)
	{
		//Need to use the top left corner of the tile as the center for calculating frag discarding (frag position is measured from center of tile)
		vec2 offsetFragPos = vec2(shadingData.fragPosition.x + tileSize * 0.5, shadingData.fragPosition.y - tileSize * 0.5);
		if(dot(offsetFragPos, offsetFragPos) < tileSize * tileSize) { discard; }
	}

	if(shadingData.isCurveTile == 1)
	{
		//Similar idea as above, but in opposite direction
		vec2 offsetFragPos = vec2(shadingData.fragPosition.x - tileSize * 0.5, shadingData.fragPosition.y + tileSize * 0.5);
		if(dot(offsetFragPos, offsetFragPos) > tileSize * tileSize) { discard; }
	}

	finalColor = shadingData.color;
}

#endif

////////////////////////////////////////////////////////////////////////////////
