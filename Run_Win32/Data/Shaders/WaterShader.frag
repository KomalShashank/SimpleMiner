#version 410 core

uniform sampler2D g_ColorTargets[];
uniform sampler2D g_DiffuseTexture;
uniform sampler2D g_DepthStencil;

uniform float g_WaterLevel;
uniform vec3 g_CameraPosition;
uniform float g_WaterRipple;
uniform float g_NearDistance;
uniform float g_FarDistance;

in vec4 clipPosition;
in vec3 passPosition;
in vec4 passColor;
in vec2 passTextureCoordinates;
in vec3 passNormal;

out vec4 outColor;

const float g_DistortionFactor = 0.01;

float LinearizeDepth(float nonLinearDepth)
{
	float linearDepth = (2.0 * g_NearDistance) / (g_FarDistance + g_NearDistance - (nonLinearDepth * (g_FarDistance - g_NearDistance)));

	return linearDepth;
}

void main(void)
{
	vec2 normalizedClipPosition = (vec2(clipPosition.xy / clipPosition.w) / vec2(2.0, 2.0)) + vec2(0.5, 0.5);

	vec2 reflectionTextureCoordinates = vec2(normalizedClipPosition.x, 1.0 - normalizedClipPosition.y);
	vec2 refractionTextureCoordinates = vec2(normalizedClipPosition.x, normalizedClipPosition.y);

	float depthValue = texture(g_DepthStencil, refractionTextureCoordinates).r;
	float waterSurfaceHeight = LinearizeDepth(depthValue);

	depthValue = gl_FragCoord.z;
	float underWaterHeight = LinearizeDepth(depthValue);
	float waterDepth = (waterSurfaceHeight - underWaterHeight) * 100.0;

	vec2 firstDistortion = (texture(g_DiffuseTexture, vec2(passTextureCoordinates.x + g_WaterRipple, -passTextureCoordinates.y + g_WaterRipple)).rg * vec2(2.0, 2.0)) - vec2(1.0, 1.0);
	vec2 secondDistortion = (texture(g_DiffuseTexture, vec2(-passTextureCoordinates.x + g_WaterRipple, passTextureCoordinates.y + g_WaterRipple)).rg * vec2(2.0, 2.0)) - vec2(1.0, 1.0);
	vec2 totalDistortion = (firstDistortion + secondDistortion) * g_DistortionFactor;

	reflectionTextureCoordinates += totalDistortion;
	reflectionTextureCoordinates = clamp(reflectionTextureCoordinates, 0.001, 0.999);

	refractionTextureCoordinates += totalDistortion;
	refractionTextureCoordinates = clamp(refractionTextureCoordinates, 0.001, 0.999);
	
	vec4 reflectionTexture = texture(g_ColorTargets[0], reflectionTextureCoordinates);
	vec4 refractionTexture = texture(g_ColorTargets[1], refractionTextureCoordinates);

	bool underTheWater = (g_CameraPosition.z < g_WaterLevel);

	float reflectiveFactor = (underTheWater) ? 0.75 : 0.25;
	float refractiveFactor = (underTheWater) ? 0.25 : 0.75;

	reflectionTexture = mix(reflectionTexture, passColor, reflectiveFactor);
	refractionTexture = mix(refractionTexture, passColor, refractiveFactor);

	vec3 waterNormal = (underTheWater) ? -passNormal : passNormal;

	vec3 vectorToCamera = normalize(g_CameraPosition - passPosition);
	float interpolationFactor = dot(vectorToCamera, waterNormal);
	interpolationFactor = pow(interpolationFactor, 2.5);

	outColor = mix(reflectionTexture, refractionTexture, interpolationFactor);
	outColor.a = clamp(waterDepth, 0.0, 1.0);
}