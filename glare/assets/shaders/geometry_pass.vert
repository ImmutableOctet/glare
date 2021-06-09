#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in vec3 a_tangent;
layout (location = 4) in vec3 a_bitangent;

//#if LAYER_POSITION_ENABLED
out vec3 fragment_position;
//#endif

out vec3 normal;
out vec2 texture_uv;
out mat3 TBN;

out vec3 tangent_fragment_position;
out vec3 tangent_view_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 view_position;

uniform bool normal_map_available = false;
uniform bool height_map_available = false;

void main()
{
    vec4 world_position = (model * vec4(a_position, 1.0)); // vec4(a_position, 1.0);

    //#if LAYER_POSITION_ENABLED
    fragment_position = world_position.xyz; // normalize(world_position.xyz); // normalize(world_position.xyz);
    //#endif

    texture_uv = a_uv;
    
    mat3 normal_matrix = mat3(model);
    //mat3 normal_matrix = transpose(mat3(model));
    //mat3 normal_matrix = transpose(inverse(mat3(model)));

    if (normal_map_available || height_map_available)
    {
        vec3 T = normalize(normal_matrix * a_tangent);
        vec3 B = normalize(normal_matrix * a_bitangent);
        vec3 N = normalize(normal_matrix * a_normal);

        //T = normalize(T - dot(T, N) * N);
        //vec3 B = cross(N, T);
        //vec3 B = cross(T, N);
    
        // Tangent-space matrix.
        TBN = transpose(mat3(T, B, N));

        tangent_view_position     = (TBN * view_position);
        tangent_fragment_position = (TBN * fragment_position);

        //normal = TBN * vec3(0.0, 1.0, 0.0);

        // Is 'normal_map_available' is 'false', we compute the normal in the fragment shader, based on the normal map.
    }

    if (!normal_map_available)
    {
        normal = normal_matrix * a_normal;
    }

    //normal = normal_matrix * (-1.0 * a_normal);

    gl_Position = projection * view * world_position;
}