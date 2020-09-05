#version 410 core

uniform sampler2D g_DiffuseTexture;
uniform sampler2D g_NormalTexture;
uniform sampler2D g_SpecularTexture;

uniform vec3 g_CameraPosition;
uniform float g_SpecularPower;



const uint MAXIMUM_NUMBER_OF_LIGHTS = 16;

uniform vec3 g_LightPosition[MAXIMUM_NUMBER_OF_LIGHTS];
uniform vec3 g_LightColor[MAXIMUM_NUMBER_OF_LIGHTS];

uniform float g_NearDistance[MAXIMUM_NUMBER_OF_LIGHTS];
uniform float g_FarDistance[MAXIMUM_NUMBER_OF_LIGHTS];
uniform float g_NearFactor[MAXIMUM_NUMBER_OF_LIGHTS];
uniform float g_FarFactor[MAXIMUM_NUMBER_OF_LIGHTS];

uniform vec3 g_LightDirection[MAXIMUM_NUMBER_OF_LIGHTS];
uniform float g_DirectionalFactor[MAXIMUM_NUMBER_OF_LIGHTS];

uniform float g_InnerAngle[MAXIMUM_NUMBER_OF_LIGHTS];
uniform float g_OuterAngle[MAXIMUM_NUMBER_OF_LIGHTS];
uniform float g_InnerFactor[MAXIMUM_NUMBER_OF_LIGHTS];
uniform float g_OuterFactor[MAXIMUM_NUMBER_OF_LIGHTS];



in vec3 passPosition;
in vec4 passColor;
in vec2 passTextureCoordinates;
in vec3 passTangent;
in vec3 passBitangent;
in vec3 passNormal;

out vec4 outColor;



struct TotalLight
{
	vec3 surfaceLight;
	vec3 specularLight;
};

TotalLight CalculateTotalLight(	vec3 surfacePosition,
								vec3 normalColor,
								vec3 directionToCamera,
								float specularColor,
								float specularPower,
								vec3 lightPosition,
								vec3 lightColor,
								float nearDistance,
								float farDistance,
								float nearFactor,
								float farFactor,
								vec3 lightDirection,
								float directionalFactor,
								float innerAngle,
								float outerAngle,
								float innerFactor,
								float outerFactor )
{
	TotalLight totalLight;

	vec3 vectorToLight = lightPosition - surfacePosition;
	float distanceToLight = length(vectorToLight);

	vec3 directionToLight = mix(vectorToLight / distanceToLight, -lightDirection, directionalFactor);
	distanceToLight = mix(distanceToLight, dot(vectorToLight, -lightDirection), directionalFactor);

	vec3 halfwayDirection = normalize(directionToLight + directionToCamera);
	float halfwayAngle = dot(lightDirection, -directionToLight);

	float linearAttenuation = mix(nearFactor, farFactor, smoothstep(nearDistance, farDistance, distanceToLight));
	float angularAttenuation = mix(innerFactor, outerFactor, smoothstep(innerAngle, outerAngle, halfwayAngle));
	float totalAttenuation = linearAttenuation * angularAttenuation;

	float surfaceLightFactor = max(dot(normalColor, directionToLight), 0.0) * totalAttenuation;
	totalLight.surfaceLight = lightColor * surfaceLightFactor;

	float specularLightFactor = max(dot(normalColor, halfwayDirection), 0.0);
	specularLightFactor = pow(specularLightFactor, specularPower) * specularColor * totalAttenuation;
	totalLight.specularLight = lightColor * specularLightFactor;

	return totalLight;
}



vec4 ApplyFog(vec4 finalColor)
{
	vec3 viewDirection = normalize(passPosition - g_CameraPosition);
	float viewDistance = length(passPosition - g_CameraPosition);

	vec3 sunlightDirection = normalize(g_CameraPosition - g_LightPosition[2]);
	vec3 sunlightColor = g_LightColor[2];

	float fogAmount = 0.75 * exp2(-g_CameraPosition.z * 0.01) * (1.0 - exp2(-viewDistance * viewDirection.z * 0.01)) / viewDirection.z;
	float sunlightFactor = max(dot(viewDirection, sunlightDirection), 0.0);

	vec4 fogColor = vec4(0.5, 0.6, 0.7, 1.0);
	vec4 finalFogColor = mix(fogColor, vec4(sunlightColor, 1.0), pow(sunlightFactor, 8.0));

	return mix(finalColor, finalFogColor, fogAmount);
}



void main(void)
{
	vec4 diffuseColor = texture(g_DiffuseTexture, passTextureCoordinates);
	vec3 normalColor = texture(g_NormalTexture, passTextureCoordinates).rgb;
	float specularColor = texture(g_SpecularTexture, passTextureCoordinates).r;

	// TODO: Multiply specular intensity here if needed.

	vec3 surfaceTangent = normalize(passTangent);
	vec3 surfaceBitangent = normalize(passBitangent);
	vec3 surfaceNormal = normalize(passNormal);

	mat3 TBN_Matrix = mat3(surfaceTangent, surfaceBitangent, surfaceNormal);
	TBN_Matrix = transpose(TBN_Matrix);

	normalColor = (normalColor * vec3(2.0, 2.0, 1.0)) - vec3(1.0, 1.0, 0.0);
	normalColor = normalize(normalColor);
	normalColor = normalColor * TBN_Matrix;

	vec3 surfaceLight = vec3(0.0);
	vec3 specularLight = vec3(0.0);

	vec3 directionToCamera = normalize(g_CameraPosition - passPosition);
	for(uint lightIndex = 0; lightIndex < MAXIMUM_NUMBER_OF_LIGHTS; ++lightIndex)
	{
		TotalLight totalLight = CalculateTotalLight(passPosition,
													normalColor,
													directionToCamera,
													specularColor,
													g_SpecularPower,
													g_LightPosition[lightIndex],
													g_LightColor[lightIndex],
													g_NearDistance[lightIndex],
													g_FarDistance[lightIndex],
													g_NearFactor[lightIndex],
													g_FarFactor[lightIndex],
													g_LightDirection[lightIndex],
													g_DirectionalFactor[lightIndex],
													g_InnerAngle[lightIndex],
													g_OuterAngle[lightIndex],
													g_InnerFactor[lightIndex],
													g_OuterFactor[lightIndex]);

		surfaceLight += totalLight.surfaceLight;
		specularLight += totalLight.specularLight;
	}

	surfaceLight = clamp(surfaceLight, vec3(0.0), vec3(1.0));

	vec4 finalColor = diffuseColor * vec4(surfaceLight, 1.0) + vec4(specularLight, 0.0);
	finalColor = clamp(finalColor, vec4(0.0), vec4(1.0));

	outColor = finalColor * passColor;
	//outColor = ApplyFog(outColor);
}