#include <gi-voxels/voxels.h>
#include <gi-voxels/constants.h>
#include <gi-voxels/util.h>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <icecream/icecream.hpp>

static const float cube_vertices[] = {
	// back face
	-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
	-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
	-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
	// front face
	-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
	-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
	-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
	// left face
	-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
	-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
	-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
	-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
	// right face
		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
	// bottom face
	-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
	-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
	-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
	// top face
	-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
	-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
	-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
};

static const float quad_vertices[] = {
    // positions        // texture Coords
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};

// static float fdata[1 << 24] = {0.0f};

Voxels::Voxels(int resolution, int screen_width, int screen_height)
		: resolution_(resolution), screen_width_(screen_width), screen_height_(screen_height),
		  voxelize_shader_("gi-voxels/shader/voxelize.vs", "gi-voxels/shader/voxelize.fs", "gi-voxels/shader/voxelize.gs"),
		  voxelize_w_direct_light_shader_("gi-voxels/shader/voxelize.vs", "gi-voxels/shader/voxelize_w_direct_light.fs", "gi-voxels/shader/voxelize.gs"),
		  voxel_face_shader_("gi-voxels/shader/voxel_face.vs", "gi-voxels/shader/voxel_face.fs"),
		  render_voxels_shader_("gi-voxels/shader/render_voxels.vs", "gi-voxels/shader/render_voxels.fs") {
	// voxel properties
	// ----------------
	glGenTextures(1, &albedo_texture_);
	glBindTexture(GL_TEXTURE_3D, albedo_texture_);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexStorage3D(GL_TEXTURE_3D, (int)log2(resolution_), GL_RGBA8, resolution_, resolution_, resolution_);
	glGenerateMipmap(GL_TEXTURE_3D);

	// rendering voxels
	// ----------------
	glGenFramebuffers(1, &visualization_fbo_);
	glBindFramebuffer(GL_FRAMEBUFFER, visualization_fbo_);
	// back face
	glGenTextures(1, &back_face_texture_);
	glBindTexture(GL_TEXTURE_2D, back_face_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screen_width, screen_height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, back_face_texture_, 0);
	// check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        assertm(false, "Voxels: framebuffer incomplete");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// cube
	GLuint cube_vbo;
	glGenVertexArrays(1, &cube_vao_);
	glGenBuffers(1, &cube_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
	glBindVertexArray(cube_vao_);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
	// quad
    // setup plane VAO
    GLuint quadVBO;
    glGenVertexArrays(1, &quad_vao_);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quad_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
	// shader
	// ------
	voxelize_shader_.use();
	voxelize_shader_.setFloat("worldSize", WORLD_SIZE);
	voxelize_shader_.setInt("voxelAlbedo", 0);

	voxelize_w_direct_light_shader_.use();
	voxelize_w_direct_light_shader_.setFloat("worldSize", WORLD_SIZE);
	voxelize_w_direct_light_shader_.setInt("voxelAlbedo", 0);
	IC();
}

void Voxels::Voxelize(Model* model) {
	voxelize_shader_.use();
	voxelize_shader_.setMat4("model", model->model_transform);
	glBindImageTexture(0, albedo_texture_, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

	// We render into framebuffer 0, but it's only a dummy render target,
	// and we don't want to change things on screen.
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glm::vec4 clear_color(0.0f);
	glClearTexImage(albedo_texture_, 0, GL_RGBA, GL_FLOAT, glm::value_ptr(clear_color));
	glDisable(GL_CULL_FACE | GL_DEPTH_TEST | GL_BLEND);
	glViewport(0, 0, resolution_, resolution_);

	model->Draw(voxelize_shader_);
	glBindTexture(GL_TEXTURE_3D, albedo_texture_);
	glGenerateMipmap(GL_TEXTURE_3D);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void Voxels::VoxelizeWithDirectLight(Model* model, ShadowMap& shadow_map) {
	voxelize_w_direct_light_shader_.use();
	voxelize_w_direct_light_shader_.setMat4("model", model->model_transform);
	shadow_map.ConfigureLightningShader(voxelize_w_direct_light_shader_);
	glBindImageTexture(0, albedo_texture_, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glm::vec4 clear_color(0.0f);
	glClearTexImage(albedo_texture_, 0, GL_RGBA, GL_FLOAT, glm::value_ptr(clear_color));
	glDisable(GL_CULL_FACE | GL_DEPTH_TEST | GL_BLEND);
	glViewport(0, 0, resolution_, resolution_);

	model->Draw(voxelize_w_direct_light_shader_);
	glBindTexture(GL_TEXTURE_3D, albedo_texture_);
	glGenerateMipmap(GL_TEXTURE_3D);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void Voxels::RenderVoxels(Model* model, Camera& camera, int mipmap_level) {
	// Render world cube back face position (aka furthest depth)
	// Then sample a ray from camera to back face
	voxel_face_shader_.use();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screen_width_ / (float)screen_height_, 0.1f, 100.0f);
	voxel_face_shader_.setMat4("view", view);
	voxel_face_shader_.setMat4("projection", projection);
	voxel_face_shader_.setMat4("model", glm::scale(glm::identity<glm::mat4>(), glm::vec3(WORLD_SIZE / 2)));

	glBindFramebuffer(GL_FRAMEBUFFER, visualization_fbo_);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_CULL_FACE | GL_DEPTH_TEST);
	glViewport(0, 0, screen_width_, screen_height_);
	glBindVertexArray(cube_vao_);

	glCullFace(GL_FRONT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);

	// render voxels
	render_voxels_shader_.use();
	render_voxels_shader_.setInt("mipMapLevel", mipmap_level);
	render_voxels_shader_.setFloat("worldSize", WORLD_SIZE);
	render_voxels_shader_.setVec3("viewPos", camera.Position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, back_face_texture_);
	render_voxels_shader_.setInt("backFaceTexture", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, albedo_texture_);
	render_voxels_shader_.setInt("voxelTexture", 2);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screen_width_, screen_height_);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(quad_vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

}