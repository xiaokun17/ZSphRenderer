#version 330 core
layout (location = 0) in vec3 v_position;

out vec2 f_texCoord;

void main()
{
	f_texCoord = v_position.xy * 0.5f + 0.5f;
	gl_Position = vec4(v_position, 1.0f);
}