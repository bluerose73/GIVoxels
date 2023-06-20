#ifndef GIVOXELS_SHADOW_MAP_H
#define GIVOXELS_SHADOW_MAP_H

#include <glm/glm.hpp>
#include <learnopengl/model.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>

// Creates shadow map for a single directional light.
class ShadowMap {
public:
    ShadowMap(int resolution);
    void Render(Model* model, const glm::vec3& light_pos);
    void ConfigureLightningShader(Shader& lightning_shader);
    GLuint GetShadowTexture() const {
        return shadow_texture_;
    }
    const glm::mat4& GetLightSpaceMatrix() const {
        return light_space_matrix_;
    }
private:
    int resolution_;
    Model* model_;
    Shader shader_;
    GLuint fbo_;
    GLuint shadow_texture_;
    glm::mat4 light_space_matrix_;
};

#endif