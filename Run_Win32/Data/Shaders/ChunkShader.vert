#version 410 core

uniform mat4 g_Model;
uniform mat4 g_View;
uniform mat4 g_Projection;

uniform vec4 g_ClippingPlane;

in vec3 inPosition;
in vec4 inColor;
in vec2 inTextureCoordinates;
in vec3 inTangent;
in vec3 inBitangent;
in vec3 inNormal;

out vec3 passPosition;
out vec4 passColor;
out vec2 passTextureCoordinates;
out vec3 passTangent;
out vec3 passBitangent;
out vec3 passNormal;

void main(void)
{
	passColor = inColor;
	passTextureCoordinates = inTextureCoordinates;

	vec4 position = vec4(inPosition, 1.0);
	passPosition = vec4(position * g_Model).xyz;

	vec4 tangent = vec4(inTangent, 0.0);
	passTangent = vec4(tangent * g_Model).xyz;

	vec4 bitangent = vec4(inBitangent, 0.0);
	passBitangent = vec4(bitangent * g_Model).xyz;

	vec4 normal = vec4(inNormal, 0.0);
	passNormal = vec4(normal * g_Model).xyz;

	position = position * g_Model * g_View * g_Projection;
	gl_Position = position;

	gl_ClipDistance[0] = dot(vec4(passPosition, 1.0), g_ClippingPlane);
}