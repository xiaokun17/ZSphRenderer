#version 330 core

uniform sampler2D u_targetTex;
uniform float     u_particleRadius;
uniform int       u_filterSize;
uniform int 	  u_screenWidth;
uniform int 	  u_screenHeight;
uniform int       horizontal;

in vec2   f_texCoord;
out float outDepth;

const float weight[8] = float[] (0.197448, 0.174697, 0.120999, 0.065602, 0.02784, 0.009246, 0.002403, 0.000489);

void main(){
	// gets size of single texel.
    float pixelDepth = texture(u_targetTex, f_texCoord).r;
    float finalDepth;
    if(pixelDepth >= 0.0f || pixelDepth < -1000.0f) 
	{
        finalDepth = pixelDepth;
    }
    else
    {
        vec2 tex_offset = 1.0 / textureSize(u_targetTex, 0);
        float result = pixelDepth * weight[0];
        result += texture(u_targetTex, f_texCoord + vec2(tex_offset.x * 1 * horizontal, tex_offset.y * 1 * (1-horizontal))).r * weight[1];
        result += texture(u_targetTex, f_texCoord - vec2(tex_offset.x * 1 * horizontal, tex_offset.y * 1 * (1-horizontal))).r * weight[1];

        result += texture(u_targetTex, f_texCoord + vec2(tex_offset.x * 2 * horizontal, tex_offset.y * 2 * (1-horizontal))).r * weight[2];
        result += texture(u_targetTex, f_texCoord - vec2(tex_offset.x * 2 * horizontal, tex_offset.y * 2 * (1-horizontal))).r * weight[2];

        result += texture(u_targetTex, f_texCoord + vec2(tex_offset.x * 3 * horizontal, tex_offset.y * 3 * (1-horizontal))).r * weight[3];
        result += texture(u_targetTex, f_texCoord - vec2(tex_offset.x * 3 * horizontal, tex_offset.y * 3 * (1-horizontal))).r * weight[3];

        result += texture(u_targetTex, f_texCoord + vec2(tex_offset.x * 4 * horizontal, tex_offset.y * 4 * (1-horizontal))).r * weight[4];
        result += texture(u_targetTex, f_texCoord - vec2(tex_offset.x * 4 * horizontal, tex_offset.y * 4 * (1-horizontal))).r * weight[4];

        result += texture(u_targetTex, f_texCoord + vec2(tex_offset.x * 5 * horizontal, tex_offset.y * 5 * (1-horizontal))).r * weight[5];
        result += texture(u_targetTex, f_texCoord - vec2(tex_offset.x * 5 * horizontal, tex_offset.y * 5 * (1-horizontal))).r * weight[5];

        result += texture(u_targetTex, f_texCoord + vec2(tex_offset.x * 6 * horizontal, tex_offset.y * 6 * (1-horizontal))).r * weight[6];
        result += texture(u_targetTex, f_texCoord - vec2(tex_offset.x * 6 * horizontal, tex_offset.y * 6 * (1-horizontal))).r * weight[6];

        result += texture(u_targetTex, f_texCoord + vec2(tex_offset.x * 7 * horizontal, tex_offset.y * 7 * (1-horizontal))).r * weight[7];
        result += texture(u_targetTex, f_texCoord - vec2(tex_offset.x * 7 * horizontal, tex_offset.y * 7 * (1-horizontal))).r * weight[7];
        finalDepth = result;
    } 

	outDepth = finalDepth;
}