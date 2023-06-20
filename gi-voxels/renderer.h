#ifndef GIVOXELS_RENDERER_H
#define GIVOXELS_RENDERER_H

#include <learnopengl/model.h>
#include <learnopengl/camera.h>
#include <learnopengl/shader.h>
#include <glm/glm.hpp>
#include <gi-voxels/shadow_map.h>
#include <gi-voxels/g_buffer.h>
#include <gi-voxels/voxels.h>

struct RenderConfig {
    int shadow_map_resolution;
    int voxel_resolution;
    int screen_width;
    int screen_height;
    int visualize_mipmap_level;
    enum RenderMode {
        VOXELS,
        DIRECT_LIGHT,
        GI
    } render_mode;
};

class Renderer {
public:
    Renderer(Model* model, const RenderConfig& config);
    void Render(Camera& camera, const glm::vec3& light_pos);
private:
    Model* model_;
    RenderConfig config_;
    ShadowMap shadow_map_;
    GBuffer g_buffer_;
    Voxels voxels_;
    Shader direct_light_shader_;
    glm::vec3 prev_light_pos_;

    void RenderDirectLight(Camera& camera, const glm::vec3& light_pos);
};

#endif