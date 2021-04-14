#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;

out vec3 fragment_position;
out vec2 uv;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * vec4(a_position, 1.0); // vec4(a_position, 1.0);
    fragment_position = worldPos.xyz; // normalize(worldPos.xyz); // normalize(worldPos.xyz);
    uv = a_uv;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    normal = normalMatrix * a_normal;

    gl_Position = projection * view * worldPos;
}