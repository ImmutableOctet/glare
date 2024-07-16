#version 330 core
//#version 400

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

#if !LAYER_POSITION_ENABLED
    uniform vec2 half_size_near_plane;

    out vec3 eye_direction;
#endif

void main()
{
    TexCoords = aTexCoords;

    #if !LAYER_POSITION_ENABLED
        vec2 clipPos = aPos.xy; // xz;

        vec2 tc = aTexCoords;

        eye_direction = -vec3(((2.0 * half_size_near_plane * tc) - half_size_near_plane), -1.0);
    #endif

    //gl_Position = vec4(aPos, 1.0);
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}