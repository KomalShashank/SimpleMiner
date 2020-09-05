#version 410 core

uniform mat4 g_Model;
uniform mat4 g_View;
uniform mat4 g_Projection;

in vec3 inPosition;
in vec4 inColor;
in vec2 inTextureCoordinates;

out vec4 passColor;
out vec2 passTextureCoordinates;

void main(void)
{
	passColor = inColor;
	passTextureCoordinates = inTextureCoordinates;

	vec4 position = vec4(inPosition, 1.0f);
	position = position * g_Model * g_View * g_Projection;
	
	gl_Position = position;
}