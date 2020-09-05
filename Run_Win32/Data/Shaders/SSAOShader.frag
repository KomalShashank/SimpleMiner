#version 410 core

uniform sampler2D g_DepthStencil;

uniform float g_RenderWidth;
uniform float g_RenderHeight;

uniform float g_NearDistance;
uniform float g_FarDistance;

uniform int g_NumberOfSamples;
uniform float g_OcclusionRadius;
uniform float g_OcclusionStrength;
uniform float g_Luminance;

in vec2 passTextureCoordinates;

out vec4 outColor;

const float PI_VALUE = 3.1415926535897932384626433832795;
const float g_MinimumOcclusion = 0.125;
const float g_GaussianWidthDifference = 0.3;
const float g_GaussianDisplacement = 0.4;
const float g_NoiseFactor = 0.0002;

vec2 GenerateDither(vec2 textureCoordinates)
{
	float noiseX = ((fract(1.0 - textureCoordinates.x * (g_RenderWidth / 2.0)) * 0.25) + (fract(textureCoordinates.y * (g_RenderHeight / 2.0)) * 0.75)) * 2.0 - 1.0;
	float noiseY = ((fract(1.0 - textureCoordinates.x * (g_RenderWidth / 2.0)) * 0.75) + (fract(textureCoordinates.y * (g_RenderHeight / 2.0)) * 0.25)) * 2.0 - 1.0;

	noiseX = clamp(fract(sin(dot(textureCoordinates, vec2(12.9898, 78.233))) * 43758.5453), 0.0, 1.0) * 2.0 - 1.0;
	noiseY = clamp(fract(sin(dot(textureCoordinates, vec2(12.9898, 78.233) * 2.0)) * 43758.5453), 0.0, 1.0) * 2.0 - 1.0;

	return vec2(noiseX, noiseY);
}

float LinearizeDepth(float nonLinearDepth)
{
	float linearDepth = (2.0 * g_NearDistance) / (g_FarDistance + g_NearDistance - (nonLinearDepth * (g_FarDistance - g_NearDistance)));

	return linearDepth;
}

float CompareDepths(float firstDepth, float secondDepth, inout bool extrapolateDepth)
{
	float gaussianWidth = 2.0;
	float depthDifference = (firstDepth - secondDepth) * 100.0;
	if (depthDifference < g_GaussianDisplacement)
	{
		gaussianWidth = g_GaussianWidthDifference;
		extrapolateDepth = false;
	}
	else
	{
		extrapolateDepth = true;
	}

	return exp(-2.0 * (depthDifference - g_GaussianDisplacement) * (depthDifference - g_GaussianDisplacement) / (gaussianWidth * gaussianWidth));
}

float CalculateAmbientOcclusion(float depthValue, float deltaWidth, float deltaHeight)
{
	float deltaDepth = (1.0 - depthValue) * g_OcclusionRadius;

	vec2 firstTextureCoordinates = vec2(passTextureCoordinates.x + (deltaWidth * deltaDepth), passTextureCoordinates.y + (deltaHeight * deltaDepth));
	vec2 secondTextureCoordinates = vec2(passTextureCoordinates.x - (deltaWidth * deltaDepth), passTextureCoordinates.y - (deltaHeight * deltaDepth));

	float firstDepth = texture(g_DepthStencil, firstTextureCoordinates).r;
	firstDepth = LinearizeDepth(firstDepth);

	bool extrapolateDepth;
	float occlusionValue = CompareDepths(depthValue, firstDepth, extrapolateDepth);
	if (extrapolateDepth)
	{
		float secondDepth = texture(g_DepthStencil, secondTextureCoordinates).r;
		secondDepth = LinearizeDepth(secondDepth);

		occlusionValue += (1.0 - occlusionValue) * CompareDepths(secondDepth, depthValue, extrapolateDepth);
	}

	return occlusionValue;
}

void main(void)
{
	vec2 ditherNoise = GenerateDither(passTextureCoordinates);
	ditherNoise *= g_NoiseFactor;

	float depthValue = texture(g_DepthStencil, passTextureCoordinates).r;
	depthValue = LinearizeDepth(depthValue);

	float width = (1.0 / g_RenderWidth) / clamp(depthValue, g_MinimumOcclusion, 1.0) + (ditherNoise.x * (1.0 - ditherNoise.x));
	float height = (1.0 / g_RenderHeight) / clamp(depthValue, g_MinimumOcclusion, 1.0) + (ditherNoise.y * (1.0 - ditherNoise.y));

	float deltaAngle = PI_VALUE * (3.0 - sqrt(5.0));
	float deltaDistance = 1.0 / float(g_NumberOfSamples);

	float currentAngle = 0.0;
	float currentDistance = 1.0 - deltaDistance / 2.0;

	float finalOcclusion = 0.0;
	for (int sampleIndex = 0; sampleIndex < g_NumberOfSamples; ++sampleIndex)
	{
		float radius = sqrt(1.0 - currentDistance);
		float widthRatio = cos(currentAngle) * radius;
		float heightRatio = sin(currentAngle) * radius;

		finalOcclusion += CalculateAmbientOcclusion(depthValue, width * widthRatio, height * heightRatio);

		currentDistance -= deltaDistance;
		currentAngle += deltaAngle;
	}

	finalOcclusion /= float(g_NumberOfSamples);
	finalOcclusion *= g_OcclusionStrength;
	finalOcclusion = 1.0 - finalOcclusion;

	vec3 finalColor = mix(vec3(finalOcclusion), vec3(1.0), g_Luminance);
	outColor = vec4(finalColor, 1.0);
}