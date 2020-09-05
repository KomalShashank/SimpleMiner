#version 410 core

uniform sampler2D g_DiffuseTexture;
uniform vec3 g_SkyboxTint;

in vec4 passColor;
in vec2 passTextureCoordinates;

out vec4 outColor;

void main(void)
{
	outColor = texture(g_DiffuseTexture, passTextureCoordinates) * passColor * vec4(g_SkyboxTint, 1.0);
}