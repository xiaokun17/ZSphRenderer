#version 330 core

uniform mat4  u_viewMatrix;
uniform mat4  u_projectMatrix;
uniform mat4  u_invViewMatrix;
uniform mat4  u_invProjectMatrix;
uniform float u_pointRadius;
uniform int   u_screenWidth;
uniform int   u_screenHeight;
uniform int   u_useAnisotropyKernel;

in vec3 f_viewCenter;
in mat3 f_anisotropyMatrix;

const float farZ  = 1000.0f;
const float nearZ = 0.1f;

out float outDepth;

void main()
{
    vec3 viewDir = normalize(f_viewCenter);
    vec3 normal;
    vec3 fragPos;

    if(u_useAnisotropyKernel == 0) 
	{		
        normal.xy = gl_PointCoord.xy * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
        float mag = dot(normal.xy, normal.xy);

        if(mag > 1.0) 
		{
            discard;           // kill pixels outside circle
        }
        normal.z = sqrt(1.0 - mag);
        fragPos  = f_viewCenter + normal * u_pointRadius;
    } 
	else 
	{
        vec3 fc = gl_FragCoord.xyz;
        fc.xy /= vec2(u_screenWidth, u_screenHeight);
        fc    *= 2.0;
        fc    -= 1.0;

        vec4 worldPos = u_invProjectMatrix * vec4(fc, 1.0);
        vec3 rayDir   = vec3(worldPos) / worldPos.w;

        mat3 transMatrix    = mat3(u_viewMatrix) * f_anisotropyMatrix * u_pointRadius;
        mat3 transInvMatrix = inverse(transMatrix);
        mat3 normalMatrix   = transpose(inverse((transMatrix)));

        vec3 camT    = transInvMatrix * vec3(0, 0, 0);
        vec3 centerT = transInvMatrix * f_viewCenter;
        vec3 rayDirT = normalize(transInvMatrix * rayDir);

        // solve the ray-sphere intersection
        float tmp   = dot(rayDirT, camT - centerT);
        float delta = tmp * tmp - dot(camT - centerT, camT - centerT) + 1.0;
        if(delta < 0.0) 
		{
            discard;             // kill pixels outside circle in parameter space
        }
        float d                  = -tmp - sqrt(delta);
        vec3  intersectionPointT = camT + rayDirT * d;
        fragPos = transMatrix * intersectionPointT;
    }

    //calculate depth
    vec4 clipSpacePos = u_projectMatrix * vec4(fragPos, 1.0);
	
	outDepth = fragPos.z;
}
