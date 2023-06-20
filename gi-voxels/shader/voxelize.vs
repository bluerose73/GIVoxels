#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 gWorldPos;
out vec2 gTexCoords;
out vec3 gNormal;

uniform mat4 model;
uniform float worldSize;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    gWorldPos = worldPos.xyz / worldPos.w;
    gTexCoords = aTexCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    gNormal = normalMatrix * aNormal;

    gl_Position = vec4(gWorldPos * 2.0f / worldSize, 1.0f);
}