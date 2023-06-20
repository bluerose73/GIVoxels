// Code built on top of
// https://learnopengl.com/code_viewer.php?code=advanced-lighting/deferred_light_cube

#include <gi-voxels/g_buffer.h>
#include <gi-voxels/util.h>
#include <gi-voxels/constants.h>
#include <glm/gtc/matrix_transform.hpp>
#include <icecream/icecream.hpp>

static const float quad_vertices[] = {
    // positions        // texture Coords
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};

GBuffer::GBuffer(int screen_width, int screen_height)
        : screen_width_(screen_width), screen_height_(screen_height),
          shader_("thirdparty/learnopengl/shader/8.1.g_buffer.vs", "thirdparty/learnopengl/shader/8.1.g_buffer.fs") {
    // Set up G-Buffer
    // 3 textures:
    // 1. Positions (RGB)
    // 2. Color (RGB) + Specular (A)
    // 3. Normals (RGB) 
    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    // - Position color buffer
    glGenTextures(1, &pos_texture_);
    glBindTexture(GL_TEXTURE_2D, pos_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screen_width, screen_height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pos_texture_, 0);
    // - Normal color buffer
    glGenTextures(1, &normal_texture_);
    glBindTexture(GL_TEXTURE_2D, normal_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screen_width, screen_height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_texture_, 0);
    // - Color + Specular color buffer
    glGenTextures(1, &albedo_texture_);
    glBindTexture(GL_TEXTURE_2D, albedo_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedo_texture_, 0);
    // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // - Create and attach depth buffer (renderbuffer)
    glGenRenderbuffers(1, &depth_buffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screen_width, screen_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);
    // - Finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        assertm(false, "GBuffer: framebuffer incomplete");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // setup plane VAO
    glGenVertexArrays(1, &quad_vao_);
    GLuint quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quad_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    IC();
}

void GBuffer::RenderGBuffer(Model* model, Camera& camera) {
    // 1. geometry pass: render scene's geometry/color data into gbuffer
    // -----------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, screen_width_, screen_height_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screen_width_ / (float)screen_height_, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader_.use();
    shader_.setMat4("projection", projection);
    shader_.setMat4("view", view);
    model->Draw(shader_);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::RenderLightning(Shader& lightning_shader) {
    // 2. render scene as normal using the generated depth/shadow map  
    // --------------------------------------------------------------
    glViewport(0, 0, screen_width_, screen_height_);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // activate gbuffer
    lightning_shader.setInt("gPosition", G_POSITION_SAMPLER_ID);
    lightning_shader.setInt("gNormal", G_NORMAL_SAMPLER_ID);
    lightning_shader.setInt("gAlbedoSpec", G_ALBEDO_SAMPLER_ID);
    glActiveTexture(GL_TEXTURE0 + G_POSITION_SAMPLER_ID);
    glBindTexture(GL_TEXTURE_2D, pos_texture_);
    glActiveTexture(GL_TEXTURE0 + G_NORMAL_SAMPLER_ID);
    glBindTexture(GL_TEXTURE_2D, normal_texture_);
    glActiveTexture(GL_TEXTURE0 + G_ALBEDO_SAMPLER_ID);
    glBindTexture(GL_TEXTURE_2D, albedo_texture_);

    // render quad
    glBindVertexArray(quad_vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}