
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;

//#if LAYER_POSITION_ENABLED
out vec3 fragment_position;
//#endif

out vec3 normal;
out vec2 uv;

out DIR_SHADOWS
{
    vec4 fragment_position;
} directional_shadows;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 directional_light_space_matrix;

void main()
{
    vec4 worldPos = (model * vec4(a_position, 1.0)); // vec4(a_position, 1.0);

    #if LAYER_POSITION_ENABLED
        fragment_position = worldPos.xyz; // normalize(worldPos.xyz); // normalize(worldPos.xyz);
    #endif
    
    //directional_shadows.fragment_position = (directional_light_space_matrix * vec4(vec3(model * vec4(a_position, 1.0)), 1.0));
    directional_shadows.fragment_position = (directional_light_space_matrix * vec4(worldPos.xyz, 1.0));

    uv = a_uv;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    normal = normalMatrix * a_normal;
    //normal = normalMatrix * (-1.0 * a_normal);

    gl_Position = projection * view * worldPos;
}