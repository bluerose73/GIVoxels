#version 450 core

in vec3 worldPos;
out vec4 color;

void main()
{
	color = vec4(worldPos, 1.0);
}