#version 450 core

#define PI 3.14159265f

const int TOTAL_DIFFUSE_CONES = 6;
const vec3 DIFFUSE_CONE_DIRECTIONS[TOTAL_DIFFUSE_CONES] = { vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.5f, 0.866025f), vec3(0.823639f, 0.5f, 0.267617f), vec3(0.509037f, 0.5f, -0.7006629f), vec3(-0.50937f, 0.5f, -0.7006629f), vec3(-0.823639f, 0.5f, 0.267617f) };
const float DIFFUSE_CONE_WEIGHTS[TOTAL_DIFFUSE_CONES] = { PI / 4.0f, 3.0f * PI / 20.0f, 3.0f * PI / 20.0f, 3.0f * PI / 20.0f,  3.0f * PI / 20.0f, 3.0f * PI / 20.0f };
const float DIFFUSE_APERTURE  = 0.577f;
const float SPECULAR_APERTURE = 0.027f;
const float AMBIENT_APERTURE  = 0.577f;
const float MAX_TRACE_DIST    = 1.0f;
const float SAMPLING_FACTOR   = 0.2f;
ivec3 voxelRes;
float startTracingOffset;

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D shadowMap;
uniform sampler3D voxelAlbedo;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform mat4 lightSpaceMatrix;
uniform float worldSize;

float ShadowCalculation(vec3 normal, vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    vec3 lightDir = normalize(lightPos);
    float bias = max(0.02 * (1.0 - dot(normal, lightDir)), 0.002);
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

bool IsInsideVoxelTexture(const vec3 p, float e) {
	return abs(p.x) < 1 + e && abs(p.y) < 1 + e && abs(p.z) < 1 + e;
}

//
// CONE TRACE FUNCTION
// note: aperture = tan(radians * 0.5)
vec4 trace_cone(const vec3 start_clip_pos, vec3 direction, float aperture, float startTracingOffset, float distance_max, float sampling_factor)
{
	aperture = max(0.1f, aperture); // inf loop if 0
	direction = normalize(direction);
	float distance = startTracingOffset; // avoid self-collision
	vec3 accumulated_color = vec3(0.0f);
	float accumulated_occlusion = 0.0f;
	
	while (distance <= distance_max && accumulated_occlusion < 1.0f)
	{
		vec3 cone_clip_pos = start_clip_pos + (direction * distance);
		vec3 cone_voxelgrid_pos = 0.5f * cone_clip_pos + vec3(0.5f); // from clipspace -1.0...1.0 to texcoords 0.0...1.0

		float diameter = 2.0f * aperture * distance; 
		float mipmap_level = log2(diameter * voxelRes.x);
		vec4 voxel_sample = textureLod(voxelAlbedo, cone_voxelgrid_pos, mipmap_level);

		// front to back composition
		accumulated_color += (1.0f - accumulated_occlusion) * voxel_sample.rgb; 
		accumulated_occlusion += (1.0f - accumulated_occlusion) * voxel_sample.a; 

		distance += diameter * sampling_factor;
	}

	accumulated_occlusion = min(accumulated_occlusion, 1.0f);
	return vec4(accumulated_color, accumulated_occlusion);
}

vec4 CalcIndirectSpecular(vec3 voxelPos, vec3 normal, vec3 worldPos, vec3 viewPos)
{
	vec3 viewDirection = normalize(worldPos - viewPos);
	vec3 coneDirection = normalize(reflect(viewDirection, normal));
	vec3 start_clip_pos = voxelPos + (normal * startTracingOffset);
	vec4 specular = trace_cone(start_clip_pos, coneDirection, SPECULAR_APERTURE,
							   startTracingOffset, MAX_TRACE_DIST, SAMPLING_FACTOR);
	return specular;
}

vec4 CalcIndirectDiffuse(vec3 voxelPos, vec3 normal) {
	vec4 accumulated_color = vec4(0.0f);

	// Arbitrarily find a vector right perpendicular to the normal
	vec3 guide = vec3(0.0f, 1.0f, 0.0f);
	if (abs(abs(dot(normal, guide)) - 1.0f) < 0.001f)
	  guide = vec3(0.0f, 0.0f, 1.0f);
	vec3 right = normalize(guide - dot(normal, guide) * normal);
	vec3 up = cross(right, normal);

	for (int i = 0; i < TOTAL_DIFFUSE_CONES; i++) {
		// make cone direction relative to normal
		vec3 coneDirection = normal;
		coneDirection += DIFFUSE_CONE_DIRECTIONS[i].x * right + DIFFUSE_CONE_DIRECTIONS[i].z * up;
		coneDirection = normalize(coneDirection);

		vec3 start_clip_pos = voxelPos + normal * startTracingOffset;
		accumulated_color += trace_cone(start_clip_pos, coneDirection, DIFFUSE_APERTURE,
										startTracingOffset, MAX_TRACE_DIST, SAMPLING_FACTOR)
							 * DIFFUSE_CONE_WEIGHTS[i];
	}

	return accumulated_color;
}

void main() {
	vec3 fragPos = texture(gPosition, TexCoords).xyz;
	vec3 voxelPos = fragPos / worldSize + 0.5f;
	if (!IsInsideVoxelTexture(voxelPos, 0.01f)) {
		discard;
	}
	FragColor = texture(voxelAlbedo, voxelPos);
	return;
	vec3 normal  = normalize(texture(gNormal, TexCoords).xyz);
	vec3 albedo  = texture(gAlbedoSpec, TexCoords).rgb;
	float specIntencity = 0.5;
	voxelRes = textureSize(voxelAlbedo, 0);
	startTracingOffset = 1.0f / voxelRes.x;

	// 1. Direct Lightning
	// -------------------
	vec3 lightDir   = normalize(lightPos);
	vec3 lightColor = vec3(1.0);
	vec3 directAmbient    = 0.2 * lightColor;
	vec3 directDiffuse    = max(dot(normal, lightDir), 0.0) * lightColor;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 directSpecular = spec * lightColor * specIntencity;

	vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
	float shadow = ShadowCalculation(normal, fragPosLightSpace);
	vec3 directLightning = (directAmbient + (1.0 - shadow) * (directDiffuse + directSpecular)) * albedo;
	// FragColor = vec4(directLightning, 1.0);
	// return;

	// 2. Indirect Lightning
	// ---------------------
	// Calculate lightning in [0, 1] cube space
	float indirectDiffuseIntensity = 0.5f;
	float indirectSpecularIntensity = 0.5f;
	float indirectAmbientIntensity = 0.5f;

	vec4 diffuseTraceResult = CalcIndirectDiffuse(voxelPos, normal);
	vec3 indirectDiffuse = diffuseTraceResult.rgb * indirectDiffuseIntensity;
	float ao = diffuseTraceResult.a;
	// vec3 indirectSpecular = CalcIndirectSpecular(voxelPos, normal, fragPos, viewPos).rgb * indirectSpecularIntensity;
	vec3 indirectSpecular = vec3(0.0f);
	// vec3 indirectAmbient = vec3(1.0 - ao) * indirectAmbientIntensity;
	vec3 indirectAmbient = vec3(0.0f);
	vec3 indirectLightning = (indirectDiffuse + indirectSpecular + indirectAmbient) * albedo;

	// FragColor = vec4(directLightning + indirectLightning, 1.0);
	FragColor = vec4(indirectDiffuse, 1.0);
}