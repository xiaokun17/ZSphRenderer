#version 330 core
layout (location = 0) in vec3 v_position;

out vec3 f_uv;

uniform mat4 u_viewMatrix;
uniform mat4 u_projectMatrix;

void main()
{
	f_uv = normalize(v_position);
	vec4 pos = u_projectMatrix * u_viewMatrix * vec4(v_position, 1.0f);
	gl_Position = pos.xyww;
}