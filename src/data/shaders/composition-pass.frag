#version 330 core

#define NUM_TOTAL_LIGHTS 8
#define SHADOW_BIAS      -0.01
#define DEPTH_BIAS       0.0001

#define FIXED_SPOT

struct PointLight
{
    vec3 lightColor;
    vec3 position;
};
uniform PointLight u_light;

uniform mat4  u_viewMatrix;
uniform mat4  u_projectMatrix;
uniform mat4  u_invViewMatrix;
uniform mat4  u_invProjectMatrix;
uniform int   u_screenWidth;
uniform int   u_screenHeight;
uniform int   u_renderType;

//uniform int       u_HasSolid;
//uniform sampler2D u_SolidDepthMap;

uniform sampler2D   u_depthTex;
uniform sampler2D   u_thicknessTex;
uniform sampler2D   u_backgroundTex;
uniform sampler2D   u_backgroundDepthTex;
uniform samplerCube u_skyBoxTex;

uniform float u_reflectionConstant;
uniform float u_attennuationConstant;
uniform int   u_transparentFluid;

in vec2  f_texCoord;
out vec4 fragColor;

// const variables
const float farZ  = 1000.0f;
const float nearZ = 0.1f;
const float refractiveIndex = 1.33;
const float eta             = 1.0 / refractiveIndex; // Ratio of indices of refraction
const float fresnelPower    = 5.0;
const float F               = ((1.0 - eta) * (1.0 - eta)) / ((1.0 + eta) * (1.0 + eta));

#define RENDER_BLUE

#ifdef RENDER_BLUE
const float k_r = 0.5f;
const float k_g = 0.2f;
const float k_b = 0.05f;
#else
const float k_r = 0.8f;
const float k_g = 0.2f;
const float k_b = 0.9f;
#endif

float toNDCDepth(float eyeDepth)
{
    float zn = ((farZ + nearZ) / (farZ - nearZ) * eyeDepth + 2 * farZ * nearZ / (farZ - nearZ)) / eyeDepth;
	return zn;
}

vec3 uvToEye(vec2 texCoord, float eyeDepth)
{
    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;
    float zn = ((farZ + nearZ) / (farZ - nearZ) * eyeDepth + 2 * farZ * nearZ / (farZ - nearZ)) / eyeDepth;

    vec4 clipPos = vec4(x, y, zn, 1.0f);
    vec4 viewPos = u_invProjectMatrix * clipPos;
    return viewPos.xyz / viewPos.w;
}

vec4 uvToSpace(vec2 texCoord, float eyeDepth)
{
    float x  = texCoord.x * 2.0 - 1.0;
    float y  = texCoord.y * 2.0 - 1.0;
    float zn = ((farZ + nearZ) / (farZ - nearZ) * eyeDepth + 2 * farZ * nearZ / (farZ - nearZ)) / eyeDepth;

    vec4 clipPos = vec4(x, y, zn, 1.0f);
    vec4 viewPos = u_invProjectMatrix * clipPos;
    vec4 eyePos  = viewPos.xyzw / viewPos.w;

    vec4 worldPos = u_invViewMatrix * eyePos;
    return worldPos;
}

vec3 computeAttennuation(float thickness)
{
    return vec3(exp(-k_r * thickness), exp(-k_g * thickness), exp(-k_b * thickness));
}

vec3 computeNormal();

void main()
{

    float eyeDepth  = texture(u_depthTex, f_texCoord).r;
	float bgDepth	= texture(u_backgroundDepthTex, f_texCoord).r;
	vec3  backColor = texture(u_backgroundTex, f_texCoord).xyz;

    vec3  N         = normalize(computeNormal());
    float thickness = texture(u_thicknessTex, f_texCoord).r;

    vec3 position = uvToEye(f_texCoord, eyeDepth);
    vec3 viewer   = normalize(-position.xyz);

    vec3  lightDir = normalize(vec3(u_viewMatrix * vec4(u_light.position, 1.0f)) - position);
    vec3  H        = normalize(lightDir + viewer);
    float specular = pow(max(0.0f, dot(H, N)), 128);
    float diffuse  = max(0.0f, dot(lightDir, N)) * 1.0f;

    float lastDepth = toNDCDepth(eyeDepth) * 0.5f + 0.5f;
    //Background Only Pixels
    if(eyeDepth > 0.0f || eyeDepth < -1000.0f || lastDepth >= bgDepth) 
	{
        if (u_renderType == 1)
        {
            float z = bgDepth * 2.0 - 1.0; // back to NDC 
            float linearDepth = (2.0 * nearZ * farZ) / (farZ + nearZ - z * (farZ - nearZ));  
            fragColor = vec4(vec3(linearDepth / 4), 1.0f);
        }
        else
        {
            fragColor = vec4(backColor, 1.0f);
        }
        
        return;
    }

    if(u_transparentFluid == 1) 
	{
		vec3 f_color = vec3(0.4196, 0.4980, 1.0);
        fragColor = vec4(vec3(f_color * 0.1f + f_color * diffuse + vec3(0, 0, 0.2) + f_color * specular), 1.0);
        return;
    }

    //Fresnel Reflection
    float fresnelRatio    = clamp(F + (1.0 - F) * pow((1.0f - dot(H, N)), fresnelPower), 0, 1);
    vec3  reflectionDir   = reflect(-viewer, N);
    vec3  reflectionColor = texture(u_skyBoxTex, reflectionDir).xyz;

    //Color Attenuation from Thickness (Beer's Law)
    vec3 colorAttennuation = computeAttennuation(thickness * 5.0f);
    colorAttennuation = mix(vec3(1, 1, 1), colorAttennuation, u_attennuationConstant);

    vec3 refractionDir   = refract(-viewer, N, 1.0 / refractiveIndex);
    vec3 refractionColor = colorAttennuation * texture(u_backgroundTex, f_texCoord + refractionDir.xy * thickness * u_attennuationConstant * 0.1f).xyz;

    fresnelRatio = mix(fresnelRatio, 1.0, u_reflectionConstant);
    vec3 finalColor = (mix(refractionColor, reflectionColor, fresnelRatio) + specular * vec3(1.0));

    if (u_renderType == 1)
    {
        fragColor = vec4(vec3(-eyeDepth / 4), 1.0f);
    }
    else if (u_renderType == 2)
    {
         fragColor = vec4(vec3(thickness), 1.0f);
    }
    else if (u_renderType == 3)
    {
         fragColor = vec4(N, 1.0f);
    }
    else
    {
        fragColor = vec4(finalColor, 1.0f);
    }
}

vec3 computeNormal()
{
	vec3 outNormal;
	float pixelWidth  = 1 / float(u_screenWidth);
    float pixelHeight = 1 / float(u_screenHeight);
    float x           = f_texCoord.x;
    float xp          = f_texCoord.x + pixelWidth;
    float xn          = f_texCoord.x - pixelWidth;
    float y           = f_texCoord.y;
    float yp          = f_texCoord.y + pixelHeight;
    float yn          = f_texCoord.y - pixelHeight;

    float depth   = texture(u_depthTex, vec2(x, y)).r;
    float depthxp = texture(u_depthTex, vec2(xp, y)).r;
    float depthxn = texture(u_depthTex, vec2(xn, y)).r;
    float depthyp = texture(u_depthTex, vec2(x, yp)).r;
    float depthyn = texture(u_depthTex, vec2(x, yn)).r;

    if(depth < -999.0f) {
        outNormal = vec3(0, 1, 0);
        return outNormal;
    }

    vec3 position   = uvToEye(vec2(x, y), depth);
    vec3 positionxp = uvToEye(vec2(xp, y), depthxp);
    vec3 positionxn = uvToEye(vec2(xn, y), depthxn);
    vec3 dxl        = position - positionxn;
    vec3 dxr        = positionxp - position;

    vec3 dx = (abs(dxr.z) < abs(dxl.z)) ? dxr : dxl;

    vec3 positionyp = uvToEye(vec2(x, yp), depthyp);
    vec3 positionyn = uvToEye(vec2(x, yn), depthyn);
    vec3 dyb        = position - positionyn;
    vec3 dyt        = positionyp - position;

    vec3 dy = (abs(dyt.z) < abs(dyb.z)) ? dyt : dyb;

    //Compute Gradients of Depth and Cross Product Them to Get Normal
    vec3 N = normalize(cross(dx, dy));
    if(isnan(N.x) || isnan(N.y) || isnan(N.y) ||
       isinf(N.x) || isinf(N.y) || isinf(N.z)) {
        N = vec3(0, 0, 1);
    }

    outNormal = N;

	return outNormal;
}
