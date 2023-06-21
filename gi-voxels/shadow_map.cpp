#include <gi-voxels/shadow_map.h>
#include <gi-voxels/util.h>
#include <gi-voxels/constants.h>
#include <icecream/icecream.hpp>

ShadowMap::ShadowMap(int resolution):
        resolution_(resolution),
        shader_("thirdparty/learnopengl/shader/3.1.3.shadow_mapping_depth.vs", "thirdparty/learnopengl/shader/3.1.3.shadow_mapping_depth.fs") {
    glGenFramebuffers(1, &fbo_);
    // create depth texture
    glGenTextures(1, &shadow_texture_);
    glBindTexture(GL_TEXTURE_2D, shadow_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_texture_, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    IC();
}

void ShadowMap::Render(Model *model, const glm::vec3& light_pos) {
    light_pos_ = light_pos;

    // 1. render depth of scene to texture (from light's perspective)
    // --------------------------------------------------------------
    glm::mat4 lightProjection, lightView;
    float bound = WORLD_SIZE * 1.4 / 2;
    lightProjection = glm::ortho(-bound, bound, -bound, bound, -bound, bound);
    lightView = glm::lookAt(light_pos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    light_space_matrix_ = lightProjection * lightView;
    // render scene from light's point of view
    shader_.use();
    shader_.setMat4("lightSpaceMatrix", light_space_matrix_);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glViewport(0, 0, resolution_, resolution_);
        glClear(GL_DEPTH_BUFFER_BIT);
        model->Draw(shader_);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    IC();
}

void ShadowMap::ConfigureLightningShader(Shader& lightning_shader) {
    lightning_shader.use();
    lightning_shader.setInt("shadowMap", G_SHADOW_MAP_SAMPLER_ID);
    lightning_shader.setVec3("lightPos", light_pos_);
    glActiveTexture(GL_TEXTURE0 + G_SHADOW_MAP_SAMPLER_ID);
    glBindTexture(GL_TEXTURE_2D, shadow_texture_);
    lightning_shader.setMat4("lightSpaceMatrix", light_space_matrix_);
}
