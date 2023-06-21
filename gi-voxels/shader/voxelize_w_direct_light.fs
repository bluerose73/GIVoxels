#version 450 core

in vec3 fWorldPos;
in vec2 fTexCoords;
in vec3 fNormal;

uniform sampler2D texture_diffuse1;
// layout (r32ui) uniform coherent volatile uimage3D voxelAlbedo;
uniform writeonly image3D voxelAlbedo;
uniform float worldSize;

// shadows
uniform sampler2D shadowMap;
uniform vec3 lightPos;
uniform mat4 lightSpaceMatrix;

// Atomic store functions provided by https://xeolabs.com/pdfs/OpenGLInsights.pdf
// vec4 convRGBA8ToVec4( uint val){
//     return vec4 ( float ((val&0x000000FF)), float ((val&0x0000FF00)>>8U), float ((val&0x00FF0000)>>16U), float ((val&0xFF000000)>>24U) );
// }
// uint convVec4ToRGBA8( vec4 val){
//     return ( uint (val.w)&0x000000FF)<<24U | ( uint(val.z)&0x000000FF)<<16U | ( uint(val.y)&0x000000FF)<<8U | ( uint(val.x)&0x000000FF);
// }

// LearnOpenGL shadow calculation
float ShadowCalculation(vec3 normal, vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 lightDir = normalize(lightPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main() {
	ivec3 voxelRes = imageSize(voxelAlbedo);
	vec3 clipPos = fWorldPos / worldSize * 2.0f;
	ivec3 voxelPos = ivec3((clipPos * 0.5f + 0.5f) * voxelRes);

    vec3 normal = normalize(fNormal);
    vec3 lightColor = vec3(1.0);
    vec3 lightDir = normalize(lightPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fWorldPos, 1.0);
    float shadow = ShadowCalculation(normal, fragPosLightSpace);

	vec4 albedo = texture(texture_diffuse1, fTexCoords);
    vec4 color  = vec4((1.0 - shadow) * diffuse * albedo.rgb, albedo.a);
	imageStore(voxelAlbedo, voxelPos, color);

    // The Atomic Avg won't compile on Nvidia, so we inlined it
    // See this issue: https://github.com/KhronosGroup/GLSL/issues/57
    // vec4 val = color;
    // ivec3 coords = voxelPos;
    // val.rgb*=255.0f; //Optimise following calculations
    // uint newVal = convVec4ToRGBA8(val);
    // uint prevStoredVal = 0; uint curStoredVal;
    // //Loop as long as destination value gets changed by other threads
    // while ( (curStoredVal = imageAtomicCompSwap(voxelAlbedo , coords, prevStoredVal , newVal)) != prevStoredVal) {
    //     prevStoredVal = curStoredVal;
    //     vec4 rval=convRGBA8ToVec4(curStoredVal);
    //     rval.xyz=(rval.xyz*rval.w); //Denormalize
    //     vec4 curValF=rval+val; //Add new value
    //     curValF.xyz/=(curValF .w); //Renormalize
    //     newVal = convVec4ToRGBA8( curValF);
    // }
}