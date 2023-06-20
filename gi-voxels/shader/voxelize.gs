#version 450 core

// We use an improved voxelization algorithm different from the GIVoxels paper.
// It does only 1 pass, while the original GIVoxels voxelization requires 3 passes, 1 from each axis.
// See OpenGL Insights, pages 303-318

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 gWorldPos[];
in vec2 gTexCoords[];
in vec3 gNormal[];

out vec3 fWorldPos;
out vec2 fTexCoords;
out vec3 fNormal;

vec2 ProjectAlongDominantAxis(in vec3 pos, in int axis) {
	if(axis == 0) return pos.yz;
	else if(axis == 1) return pos.xz;
	else return pos.xy;
}

void main() {
    vec3 edge1 = normalize(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz);
    vec3 edge2 = normalize(gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz);
	vec3 triangle_normal = cross(edge1, edge2);
    vec3 normal_abs = abs(triangle_normal);

    int dominant_axis;
	if(normal_abs.x >= normal_abs.y && normal_abs.x > normal_abs.z) dominant_axis = 0;
	else if(normal_abs.y >= normal_abs.z) dominant_axis = 1;
	else dominant_axis = 2;

	vec3 triangle_centroid = (gl_in[0].gl_Position.xyz + gl_in[1].gl_Position.xyz + gl_in[2].gl_Position.xyz) / 3.0f;
    vec2 screen_triangle_centroid = ProjectAlongDominantAxis(triangle_centroid, dominant_axis);

    for (int i = 0; i < 3; i++) {
        fWorldPos = gWorldPos[i];
        fTexCoords = gTexCoords[i];
        fNormal = gNormal[i];

        vec2 screen_pos = ProjectAlongDominantAxis(gl_in[i].gl_Position.xyz, dominant_axis);
        // Conservertive rasterization. Make the triangle a little bigger 
        // to ensure a fragment will be generated for each pixel that touches the triangle
        screen_pos += normalize(screen_pos - screen_triangle_centroid) * 0.05f;

        gl_Position = vec4(screen_pos, 0.0f, 1.0f);
        EmitVertex();
    }
	EndPrimitive();
}