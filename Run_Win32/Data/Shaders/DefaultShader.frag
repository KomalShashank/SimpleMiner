#version 410 core

uniform sampler2D g_DiffuseTexture;
uniform sampler2D g_NormalTexture;
uniform sampler2D g_SpecularTexture;
uniform sampler2D g_AmbientTexture;
uniform sampler2D g_EmissiveTexture;

in vec3 passPosition;
in vec4 passColor;
in vec2 passTextureCoordinates;
in vec3 passTangent;
in vec3 passBitangent;
in vec3 passNormal;

out vec4 outColor;

void main(void)
{
	outColor = texture(g_DiffuseTexture, passTextureCoordinates) * passColor;
}