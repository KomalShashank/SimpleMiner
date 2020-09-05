#version 410 core

uniform mat4 g_Model;
uniform mat4 g_View;
uniform mat4 g_Projection;

in vec3 inPosition;
in vec4 inColor;
in vec3 inNormal;

out vec4 clipPosition;
out vec3 passPosition;
out vec4 passColor;
out vec2 passTextureCoordinates;
out vec3 passNormal;

const float g_TilingFactor = 0.1;

void main(void)
{
	passColor = inColor;
	passTextureCoordinates = (inPosition.xy / vec2(2.0, 2.0)) + vec2(0.5, 0.5);
	passTextureCoordinates *= g_TilingFactor;

	vec4 position = vec4(inPosition, 1.0);
	passPosition = vec4(position * g_Model).xyz;

	vec4 normal = vec4(inNormal, 0.0);
	passNormal = vec4(normal * g_Model).xyz;

	position = position * g_Model * g_View * g_Projection;
	gl_Position = position;
	clipPosition = position;
}