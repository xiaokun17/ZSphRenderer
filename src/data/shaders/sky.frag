#version 330 core

uniform samplerCube cubemap;

in vec3 f_uv;

out vec4 fragColor;

void main()
{
	fragColor = texture(cubemap, f_uv);
}