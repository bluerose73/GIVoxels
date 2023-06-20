#ifndef GIVOXELS_CONSTANTS_H
#define GIVOXELS_CONSTANTS_H

#include <glm/glm.hpp>

// Assume the world space is a cube of size [-50, 50]
// Transform the model to fit in the cube
// This convention will be used in
// - Projection box of projection transform
// - Range of voxelization
constexpr float WORLD_SIZE = 100.0;

constexpr glm::vec3 UP_VEC(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 FRONT_VEC(0.0f, 0.0f, -1.0f);

// Texture sampler id of the final lightning pass shader
// 0 ~ 3 are reserved for diffuse, specular, normal mapping, etc, set by LearnOpenGL Model.
// See Mesh::Draw
constexpr int G_POSITION_SAMPLER_ID   = 4;
constexpr int G_NORMAL_SAMPLER_ID     = 5;
constexpr int G_ALBEDO_SAMPLER_ID     = 6;
constexpr int G_SHADOW_MAP_SAMPLER_ID = 7;

#endif