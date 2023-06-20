#version 450 core

in vec3 fWorldPos;
in vec2 fTexCoords;
in vec3 fNormal;

uniform sampler2D texture_diffuse1;
uniform writeonly image3D voxelAlbedo;
// uniform writeonly image3D voxelNormal;
uniform float worldSize;

void main() {
	ivec3 voxelRes = imageSize(voxelAlbedo);
	vec3 clipPos = fWorldPos / worldSize * 2.0f;
	ivec3 voxelPos = ivec3((clipPos * 0.5f + 0.5f) * voxelRes);

	vec4 albedo = texture(texture_diffuse1, fTexCoords);
	imageStore(voxelAlbedo, voxelPos, albedo);

	// vec4 normal = vec4(normalize(fNormal), 1.0f);
	// imageStore(voxelNormal, voxelPos, normal);
}