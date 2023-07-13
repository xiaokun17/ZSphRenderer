#version 330 core

uniform sampler2D u_targetTex;

in vec2 f_texCoord;

out float outDepth;

void main()
{
	outDepth = texture(u_targetTex, f_texCoord).r;
}