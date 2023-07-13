#version 330 core
layout(location = 0) in vec3 v_position;
layout(location = 2) in vec3 v_normal;

uniform mat4 u_viewMatrix;
uniform mat4 u_projectMatrix;

out vec3 f_normal;
out vec3 f_fragPos;

void main()
{
    gl_Position = u_projectMatrix * u_viewMatrix * vec4(v_position, 1.0f);
	f_fragPos = v_position;
	f_normal = v_normal;
}