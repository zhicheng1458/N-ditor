////////////////////////////////////////
// BlendShader.glsl
////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#ifdef VERTEX_SHADER

////////////////////////////////////////
// Vertex shader
////////////////////////////////////////

layout(location = 0) in vec3 Position;

void main()
{
	gl_Position = vec4(Position.x, Position.y, 0.0, 1.0);
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef FRAGMENT_SHADER

uniform sampler2DMS solidObjectTexture;
uniform sampler2DMS transparentObjectTexture;
uniform sampler2DMS overlapCountTexture;

uniform int numSample;

out vec4 finalColor;

////////////////////////////////////////
// Fragment shader
////////////////////////////////////////

vec4 textureMultisample4x(sampler2DMS sampler, ivec2 coord)
{
    vec4 color = vec4(0.0);

    for (int i = 0; i < 4; i++)
	{
        color += texelFetch(sampler, coord, i);
	}

    color /= 4;
	//if(abs(color.r - texelFetch(sampler, coord, 0).r) < 0.01) {return vec4(0.0);}

    return color;
}

void main()
{
	vec4 currentColor = textureMultisample4x(solidObjectTexture, ivec2(gl_FragCoord.xy));
	vec4 accumulatedColor = textureMultisample4x(transparentObjectTexture, ivec2(gl_FragCoord.xy));
	float n = max(1.0, textureMultisample4x(overlapCountTexture, ivec2(gl_FragCoord.xy)).r);
	float weightedAverageAlpha = pow(max(0.0, 1.0 - accumulatedColor.a / n), n);
	vec3 averageColor = accumulatedColor.rgb / max(accumulatedColor.a, 0.0001);

	//Blend function: essentially equivalent to glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA)
	finalColor = vec4(averageColor.xyz, weightedAverageAlpha) * (1.0 - weightedAverageAlpha) + currentColor * weightedAverageAlpha;
}

#endif