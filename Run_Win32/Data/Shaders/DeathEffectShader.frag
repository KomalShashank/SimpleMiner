#version 410 core

uniform sampler2D g_ColorTargets[];
uniform sampler2D g_DepthStencil;
uniform float g_ColorBlendFactor;

uniform float g_RenderWidth;
uniform float g_RenderHeight;

in vec4 passColor;
in vec2 passTextureCoordinates;

out vec4 outColor;

float pixelOffset[3] = float[](0.0, 1.3846153846, 3.2307692308);
float pixelWeight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

void main(void)
{
	vec4 initialColor = texture(g_ColorTargets[0], passTextureCoordinates);

	float blurIntensity = 5.0 * (1.0 - g_ColorBlendFactor);
	vec3 blurColor = texture(g_ColorTargets[0], passTextureCoordinates).rgb * pixelWeight[0];
	for (int offsetIndex = 1; offsetIndex < 3; ++offsetIndex)
	{
		blurColor += texture(g_ColorTargets[0], passTextureCoordinates + vec2(blurIntensity * pixelOffset[offsetIndex] / g_RenderWidth, blurIntensity * pixelOffset[offsetIndex] / g_RenderHeight)).rgb * pixelWeight[offsetIndex];
		blurColor += texture(g_ColorTargets[0], passTextureCoordinates - vec2(blurIntensity * pixelOffset[offsetIndex] / g_RenderWidth, blurIntensity * pixelOffset[offsetIndex] / g_RenderHeight)).rgb * pixelWeight[offsetIndex];
	}

	float grayscaleAverage = 0.2126 * blurColor.r + 0.7152 * blurColor.g + 0.0722 * blurColor.b;
	vec3 grayscaleBlur = vec3(grayscaleAverage, grayscaleAverage, grayscaleAverage);

	vec4 finalColor = vec4(grayscaleBlur, 1.0);
	outColor = mix(finalColor, initialColor, g_ColorBlendFactor);
}