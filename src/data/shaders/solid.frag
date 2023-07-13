#version 330 core

struct PointLight
{
    vec3 lightColor;
    vec3 position;
};
uniform PointLight u_light;

uniform mat4 u_viewMatrix;
uniform mat4 u_projectMatrix;
uniform vec3 u_cameraPos;
uniform vec3 f_color;

in vec3 f_normal;
in vec3 f_fragPos;

out vec4 fragColor;

void main()
{
	vec3 normal 	   = normalize(f_normal);
	vec3 viewDir 	   = normalize(u_cameraPos - f_fragPos);
	vec3 lightDir      = normalize(u_light.position - f_fragPos);
	vec3 halfDir  	   = normalize(lightDir + viewDir);

	// shading
	// const vec3 f_color = vec3(0.925f, 0.654f, 0.145f);
	
	vec3 ambientColor  = u_light.lightColor * f_color * 0.3f;
	vec3 diffuseColor  = u_light.lightColor * f_color * max(dot(normal, lightDir), 0.0f);
	vec3 specularColor = u_light.lightColor * pow(max(dot(halfDir, normal), 0.0), 32);
	
	fragColor = vec4(ambientColor + diffuseColor + specularColor, 1.0f);
}
