#ifndef GIVOXELS_G_BUFFER_H
#define GIVOXELS_G_BUFFER_H

#include <learnopengl/model.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>

class GBuffer {
public:
    GBuffer(int screen_width, int screen_height);
    void RenderGBuffer(Model* model, Camera& camera);
    void RenderLightning(Shader& lightning_shader);

private:
    int screen_width_, screen_height_;
    Shader shader_;
    GLuint fbo_;
    GLuint pos_texture_;
    GLuint normal_texture_;
    GLuint albedo_texture_;
    GLuint depth_buffer_;

    GLuint quad_vao_;
};

#endif