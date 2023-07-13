#version 330 core
layout (location = 0) in vec3 v_position;

#define DEPTH_BIAS 0.0001

uniform mat4  u_viewMatrix;
uniform mat4  u_projectMatrix;
uniform float u_pointRadius;
uniform int   u_screenWidth;
uniform int   u_screenHeight;

void computePointSizeAndPosition(mat4 T)
{
	const mat4 D = mat4(1., 0., 0., 0.,
						0., 1., 0., 0.,
						0., 0., 1., 0.,
						0., 0., 0., -1.);
    vec2 xbc;
    vec2 ybc;

    mat4  R = transpose(u_projectMatrix * u_viewMatrix * T);
    float A = dot(R[ 3 ], D * R[ 3 ]);
    float B = -2. * dot(R[ 0 ], D * R[ 3 ]);
    float C = dot(R[ 0 ], D * R[ 0 ]);
    xbc[ 0 ] = (-B - sqrt(B * B - 4. * A * C)) / (2.0 * A);
    xbc[ 1 ] = (-B + sqrt(B * B - 4. * A * C)) / (2.0 * A);
    float sx = abs(xbc[ 0 ] - xbc[ 1 ]) * .5 * u_screenWidth;

    A        = dot(R[ 3 ], D * R[ 3 ]);
    B        = -2. * dot(R[ 1 ], D * R[ 3 ]);
    C        = dot(R[ 1 ], D * R[ 1 ]);
    ybc[ 0 ] = (-B - sqrt(B * B - 4. * A * C)) / (2.0 * A);
    ybc[ 1 ] = (-B + sqrt(B * B - 4. * A * C)) / (2.0 * A);
    float sy = abs(ybc[ 0 ] - ybc[ 1 ]) * .5 * u_screenHeight;

    float pointSize = ceil(max(sx, sy));
    gl_PointSize = pointSize;
}

void main()
{
    vec4  eyeCoord = u_viewMatrix * vec4(vec3(v_position), 1.0);
    vec3  posEye   = vec3(eyeCoord);
    float dist     = length(posEye);
	
    mat4 T = mat4(u_pointRadius, 0, 0, 0,
                 0, u_pointRadius, 0, 0,
                 0, 0, u_pointRadius, 0,
                 v_position.x, v_position.y, v_position.z, 1.0);
	computePointSizeAndPosition(T);
    float u_PointScale = u_screenHeight / tan(55.0 * 0.5 * 3.1415926 / 180.0f);
    
	gl_PointSize = u_pointRadius * (u_PointScale / dist) * 4.0f;
    
	vec4 NDCoord = u_projectMatrix * eyeCoord;
    gl_Position = NDCoord;

}
