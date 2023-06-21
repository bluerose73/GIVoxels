#include <gi-voxels/renderer.h>
#include <gi-voxels/constants.h>
#include <gi-voxels/util.h>
#include <icecream/icecream.hpp>

Renderer::Renderer(Model* model, const RenderConfig& config)
        : model_(model), config_(config), prev_light_pos_(0.0f),
          shadow_map_(config.shadow_map_resolution),
          g_buffer_(config.screen_width, config.screen_height),
          direct_light_shader_("gi-voxels/shader/direct_light.vs", "gi-voxels/shader/direct_light.fs"),
          voxel_cone_tracing_shader_("gi-voxels/shader/voxel_cone_tracing.vs", "gi-voxels/shader/voxel_cone_tracing.fs"),
          voxels_(config.voxel_resolution, config.screen_width, config.screen_height) {
    IC();
}

void Renderer::Render(Camera& camera, const glm::vec3& light_pos) {
    if (light_pos == glm::vec3(0.0f))
        assertm(false, "Light Pos = (0, 0, 0)");
    bool light_changed = !(light_pos == prev_light_pos_ || light_pos == glm::vec3(0.0f));

    if (light_changed) {
        shadow_map_.Render(model_, light_pos);
        if (config_.render_mode == RenderConfig::VOXELS) {
            voxels_.Voxelize(model_);
        }
        else {
            // Cut corners by combining voxelizing and light injection in a sigle pass
            // With the cost of having to re-voxelize the model every time the light moves
            // Will degrade performance if there are too many vertices in the model
            voxels_.VoxelizeWithDirectLight(model_, shadow_map_);
        }
        prev_light_pos_ = light_pos;
    }

    switch (config_.render_mode) {
        case RenderConfig::VOXELS:
        case RenderConfig::VOXELS_W_DIRECT_LIGHT:
            voxels_.RenderVoxels(model_, camera, config_.visualize_mipmap_level);
            break;
        case RenderConfig::DIRECT_LIGHT:
            g_buffer_.RenderGBuffer(model_, camera);
            RenderDirectLight(camera);
            break;
        case RenderConfig::GI:
            g_buffer_.RenderGBuffer(model_, camera);
            RenderVoxelConeTracing(camera);
            break;
        default:
            assertm(false, "Invalid Render Mode");
    }
}


void Renderer::RenderDirectLight(Camera& camera) {
    direct_light_shader_.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)config_.screen_width / (float)config_.screen_height, 0.1f, WORLD_SIZE);
    glm::mat4 view = camera.GetViewMatrix();
    direct_light_shader_.setMat4("projection", projection);
    direct_light_shader_.setMat4("view", view);
    direct_light_shader_.setVec3("viewPos", camera.Position);
    shadow_map_.ConfigureLightningShader(direct_light_shader_);
    g_buffer_.RenderLightning(direct_light_shader_);
}

void Renderer::RenderVoxelConeTracing(Camera& camera) {
    voxel_cone_tracing_shader_.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)config_.screen_width / (float)config_.screen_height, 0.1f, WORLD_SIZE);
    glm::mat4 view = camera.GetViewMatrix();
    voxel_cone_tracing_shader_.setMat4("projection", projection);
    voxel_cone_tracing_shader_.setMat4("view", view);
    voxel_cone_tracing_shader_.setVec3("viewPos", camera.Position);
    voxel_cone_tracing_shader_.setFloat("worldSize", WORLD_SIZE);
    voxel_cone_tracing_shader_.setInt("voxelAlbedo", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, voxels_.GetAlbedoTexture());
    shadow_map_.ConfigureLightningShader(voxel_cone_tracing_shader_);
    g_buffer_.RenderLightning(voxel_cone_tracing_shader_);
}