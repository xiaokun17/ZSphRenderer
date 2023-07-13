#version 330 core
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_color;
layout (location = 2) in vec3 v_normal;
layout (location = 3) in vec2 v_uv;

out vec2 f_uv;
out vec3 f_normal;
out vec3 f_fragPos;

uniform mat4 u_viewMatrix;
uniform mat4 u_projectMatrix;

void main()
{
	f_uv = v_uv;
	f_normal = v_normal;
	f_fragPos = v_position;
	gl_Position = u_projectMatrix * u_viewMatrix * vec4(v_position, 1.0f);
	
}