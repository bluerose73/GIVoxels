#ifndef GIVOXELS_VOXELS_HPP
#define GIVOXELS_VOXELS_HPP

#include <vector>
#include <learnopengl/model.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>

class Voxels {
public:
	Voxels(int resolution, int screen_width, int screen_height);
	void Voxelize(Model* model);
	void RenderVoxels(Model* model, Camera& camera, int mipmap_level);
private:
	int resolution_;
	int screen_width_, screen_height_;

	// voxels

	GLuint normal_texture_, albedo_texture_;
	Shader voxelize_shader_;

	// visualization
	GLuint visualization_fbo_;
	GLuint back_face_texture_;
	Shader voxel_face_shader_, render_voxels_shader_;
	GLuint cube_vao_, quad_vao_;
};


#endif //VXGITEST_VOXELS_HPP