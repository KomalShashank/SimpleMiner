#version 410 core

uniform sampler2D g_ColorTargets[];
uniform sampler2D g_DepthStencil;

uniform float g_RenderWidth;
uniform float g_RenderHeight;

uniform float g_NearDistance;
uniform float g_FarDistance;

in vec4 passColor;
in vec2 passTextureCoordinates;

out vec4 outColor;



const float PI_VALUE = 3.1415926535897932384626433832795;

const int g_NumberOfSamples = 3;
const int g_NumberOfRings = 5;

const vec2 g_FocalPoint = vec2(0.5, 0.5);
const float g_FocalRange = 4.0;
const float g_MaximumBlur = 0.75;

const float g_BokehEdgeBias = 0.4;
const float g_BokehFringe = 0.5;

const float g_NoiseFactor = 0.0001;
const float g_DepthBlurSize = 2.0;

const int g_NumberOfOcclusionSamples = 64;
const float g_OcclusionRadius = 5.0;
const float g_Luminance = 0.1;

const float g_MinimumOcclusion = 0.125;
const float g_GaussianWidthDifference = 0.3;
const float g_GaussianDisplacement = 0.4;



vec2 renderTexel = vec2(1.0 / g_RenderWidth, 1.0 / g_RenderHeight);



float BlurDepth(vec2 textureCoordinates)
{
	float blurredValue = 0.0;
	float kernel[9];
	vec2 offset[9];

	vec2 blurPoint = renderTexel * g_DepthBlurSize;

	offset[0] = vec2(-blurPoint.x, -blurPoint.y);
	offset[1] = vec2(0.0, -blurPoint.y);
	offset[2] = vec2(blurPoint.x, -blurPoint.y);

	offset[3] = vec2(-blurPoint.x, 0.0);
	offset[4] = vec2(0.0, 0.0);
	offset[5] = vec2(blurPoint.x, 0.0);

	offset[6] = vec2(-blurPoint.x, blurPoint.y);
	offset[7] = vec2(0.0, blurPoint.y);
	offset[8] = vec2(blurPoint.x, blurPoint.y);

	kernel[0] = 1.0 / 16.0;
	kernel[1] = 2.0 / 16.0;
	kernel[2] = 1.0 / 16.0;

	kernel[3] = 2.0 / 16.0;
	kernel[4] = 4.0 / 16.0;
	kernel[5] = 2.0 / 16.0;

	kernel[6] = 1.0 / 16.0;
	kernel[7] = 2.0 / 16.0;
	kernel[8] = 1.0 / 16.0;

	for (int index = 0; index < 9; ++index)
	{
		float offsetValue = texture(g_DepthStencil, textureCoordinates + offset[index]).r;
		blurredValue += offsetValue * kernel[index];
	}

	return blurredValue;
}



vec3 ProcessSample(vec2 textureCoordinates, float blurValue)
{
	vec3 processedSample = vec3(0.0);

	processedSample.r = texture(g_ColorTargets[0], textureCoordinates + vec2(0.0, 1.0) * renderTexel * g_BokehFringe * blurValue).r;
	processedSample.g = texture(g_ColorTargets[0], textureCoordinates + vec2(-0.866, -0.5) * renderTexel * g_BokehFringe * blurValue).g;
	processedSample.b = texture(g_ColorTargets[0], textureCoordinates + vec2(0.866, -0.5) * renderTexel * g_BokehFringe * blurValue).b;

	return processedSample;
}



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



vec3 ApplySSAO(vec3 finalColor)
{
	vec2 ditherNoise = GenerateDither(passTextureCoordinates);
	ditherNoise *= g_NoiseFactor;

	float depthValue = texture(g_DepthStencil, passTextureCoordinates).r;
	depthValue = LinearizeDepth(depthValue);

	float width = (1.0 / g_RenderWidth) / clamp(depthValue, g_MinimumOcclusion, 1.0) + (ditherNoise.x * (1.0 - ditherNoise.x));
	float height = (1.0 / g_RenderHeight) / clamp(depthValue, g_MinimumOcclusion, 1.0) + (ditherNoise.y * (1.0 - ditherNoise.y));

	float deltaAngle = PI_VALUE * (3.0 - sqrt(5.0));
	float deltaDistance = 1.0 / float(g_NumberOfOcclusionSamples);

	float currentAngle = 0.0;
	float currentDistance = 1.0 - deltaDistance / 2.0;

	float finalOcclusion = 0.0;
	for (int sampleIndex = 0; sampleIndex < g_NumberOfOcclusionSamples; ++sampleIndex)
	{
		float radius = sqrt(1.0 - currentDistance);
		float widthRatio = cos(currentAngle) * radius;
		float heightRatio = sin(currentAngle) * radius;

		finalOcclusion += CalculateAmbientOcclusion(depthValue, width * widthRatio, height * heightRatio);

		currentDistance -= deltaDistance;
		currentAngle += deltaAngle;
	}

	finalOcclusion /= float(g_NumberOfOcclusionSamples);
	finalOcclusion = 1.0 - finalOcclusion;
	vec3 finalOcclusionColor = mix(vec3(finalOcclusion), vec3(1.0), g_Luminance);

	return finalColor * finalOcclusionColor;
}



void main(void)
{
	float depthValue = BlurDepth(passTextureCoordinates);
	depthValue = LinearizeDepth(depthValue);
	float blurValue = 0.0;

	float focalDepth = texture(g_DepthStencil, g_FocalPoint).r;
	focalDepth = LinearizeDepth(focalDepth);
	blurValue = clamp((abs(depthValue - focalDepth) / g_FocalRange) * 100.0, -g_MaximumBlur, g_MaximumBlur);

	vec2 ditherNoise = GenerateDither(passTextureCoordinates) * g_NoiseFactor * blurValue;

	float width = (1.0 / g_RenderWidth) * blurValue + ditherNoise.x;
	float height = (1.0 / g_RenderHeight) * blurValue + ditherNoise.y;

	vec3 initialColor = texture(g_ColorTargets[0], passTextureCoordinates).rgb;
	float divisionFactor = 1.0;

	int g_NumberOfRingsamples = 0;
	for (int ringIndex = 1; ringIndex <= g_NumberOfRings; ++ringIndex)
	{
		g_NumberOfRingsamples = ringIndex * g_NumberOfSamples;

		for (int ringSampleIndex = 0; ringSampleIndex < g_NumberOfRingsamples; ++ringSampleIndex)
		{
			float currentStep = (PI_VALUE * 2.0) / float(g_NumberOfRingsamples);
			float ringWidth = (cos(float(ringSampleIndex) * currentStep) * float(ringIndex));
			float ringHeight = (sin(float(ringSampleIndex) * currentStep) * float(ringIndex));

			initialColor += (ProcessSample(passTextureCoordinates + vec2(ringWidth * width, ringHeight * height), blurValue) * mix(1.0, (float(ringIndex)) / (float(g_NumberOfRings)), g_BokehEdgeBias));
			divisionFactor += mix(1.0, (float(ringIndex)) / (float(g_NumberOfRings)), g_BokehEdgeBias);
		}
	}

	vec3 finalColor = initialColor / divisionFactor;
	outColor = vec4(finalColor, 1.0);
}