#version 410 core

uniform sampler2D g_DiffuseTexture;

in vec4 passColor;
in vec2 passTextureCoordinates;

out vec4 outColor;

void main(void)
{
	outColor = texture(g_DiffuseTexture, passTextureCoordinates) * passColor;
}