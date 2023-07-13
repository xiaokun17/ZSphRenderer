#version 330 core
layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_color;
layout (location = 4) in vec3 v_anisotropyMatrix0;
layout (location = 5) in vec3 v_anisotropyMatrix1;
layout (location = 6) in vec3 v_anisotropyMatrix2;

uniform mat4 u_viewMatrix;
uniform mat4 u_projectMatrix;
uniform mat4 u_invViewMatrix;
uniform mat4 u_invProjectMatrix;
uniform float u_pointRadius;
uniform int   u_screenWidth;
uniform int   u_screenHeight;
uniform int   u_useAnisotropyKernel;

out vec3 f_viewCenter;
out vec3 f_color;
out mat3 f_anisotropyMatrix;

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

void computePointSizeAndPosition_iso(mat4 T)
{
	const mat4 D = mat4(1., 0., 0., 0.,
						0., 1., 0., 0.,
						0., 0., 1., 0.,
						0., 0., 0., -1.);
    vec2 xbc;

    mat4  R = transpose(u_projectMatrix * u_viewMatrix * T);
    float A = dot(R[ 3 ], D * R[ 3 ]);
    float B = -2. * dot(R[ 0 ], D * R[ 3 ]);
    float C = dot(R[ 0 ], D * R[ 0 ]);
    xbc[ 0 ] = sqrt(B * B - 4. * A * C) / A;
    float sx = abs(xbc[ 0 ]) * .5 * u_screenWidth;
    gl_PointSize = sx;

}

void main()
{
    vec4  eyeCoord = u_viewMatrix * vec4(v_position, 1.0);
    vec3  posEye   = vec3(eyeCoord);
    float dist     = length(posEye);

    // output
    f_viewCenter       = posEye;	
	f_color 		   = v_color;
	f_anisotropyMatrix = (u_useAnisotropyKernel == 0) ? mat3(0) : mat3(v_anisotropyMatrix0, v_anisotropyMatrix1, v_anisotropyMatrix2);

    if(u_useAnisotropyKernel == 0){
        mat4 T = mat4(v_anisotropyMatrix0[0] * u_pointRadius, v_anisotropyMatrix0[1] * u_pointRadius, v_anisotropyMatrix0[2] * u_pointRadius, 0,
                      v_anisotropyMatrix1[0] * u_pointRadius, v_anisotropyMatrix1[1] * u_pointRadius, v_anisotropyMatrix1[2] * u_pointRadius, 0,
                      v_anisotropyMatrix2[0] * u_pointRadius, v_anisotropyMatrix2[1] * u_pointRadius, v_anisotropyMatrix2[2] * u_pointRadius, 0,
                      v_position.x, v_position.y, v_position.z, 1.0);
        
        float sx = length(v_anisotropyMatrix0);
        float sy = length(v_anisotropyMatrix1);
        float sz = length(v_anisotropyMatrix2);

        if(abs(sx - sy) < 1e-2 && abs(sy - sz) < 1e-2 && abs(sz - sx) < 1e-2) 
        {
            T = mat4(u_pointRadius, 0, 0, 0,
                     0, u_pointRadius, 0, 0,
                     0, 0, u_pointRadius, 0,
                     v_position.x, v_position.y, v_position.z, 1.0);
            f_anisotropyMatrix = mat3(1);
        }

        computePointSizeAndPosition(T);
    }
    else{
        mat4 T = mat4(u_pointRadius, 0, 0, 0,
                      0, u_pointRadius, 0, 0,
                      0, 0, u_pointRadius, 0,
                      v_position.x, v_position.y, v_position.z, 1.0);
        computePointSizeAndPosition_iso(T);
    }

    gl_Position = u_projectMatrix * eyeCoord;

}