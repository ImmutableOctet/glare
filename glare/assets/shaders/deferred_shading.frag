#version 330 core

out vec4 output_color;

in vec2 TexCoords;

#if LAYER_DEPTH_ENABLED
    uniform sampler2D g_depth;
#endif

uniform sampler2D g_normal;
uniform sampler2D g_albedo_specular;

#if LAYER_POSITION_ENABLED
    uniform sampler2D g_position;
#else
    uniform mat4 inv_view;
    uniform mat4 inv_projection;

    // TODO: Determine if we need to pass only one inverse matrix.
    uniform mat4 inv_projview;

    uniform mat4 projection;

    uniform vec2 depth_range;
    uniform vec2 half_size_near_plane;

    in vec3 eye_direction;
#endif

struct PointLight
{
    vec3 position;
    vec3 color;
    
    float linear;
    float quadratic;
    float radius;
};

const float specular_intensity = 16.0; // 8.0;
const bool limit_to_radius = false; // true;

const int MAX_POINT_LIGHTS = 32; // 16; // 128;

uniform int point_lights_count = 0;
uniform PointLight point_lights[MAX_POINT_LIGHTS];
uniform vec3 view_position;

//uniform float ambient_light = 0.8; // 0.1;
uniform vec3 ambient_light = vec3(0.1, 0.1, 0.1);

//uniform float alpha = 1.0; // const
//uniform bool specular_available = false;

vec4 exec(vec3 world_position)
{
    // G-Buffer Layers:
    vec3  normal   = texture(g_normal, TexCoords).rgb;
    vec3  diffuse  = texture(g_albedo_specular, TexCoords).rgb;
    float specular = texture(g_albedo_specular, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 lighting  = diffuse * ambient_light;
    vec3 view_direction  = normalize(view_position - world_position); // normalize(view_position) - normalize(world_position);

    for (int i = 0; i < point_lights_count; ++i)
    {
        // calculate distance between light source and current fragment
        float distance = length(point_lights[i].position - world_position);

        if ((!limit_to_radius) || (distance < point_lights[i].radius))
        {
            // diffuse
            vec3 lightDir = normalize(point_lights[i].position - world_position);
            vec3 diffuse = max(dot(normal, lightDir), 0.0) * diffuse * point_lights[i].color;

            // specular
            vec3 halfwayDir = normalize(lightDir + view_direction);  
            float spec = pow(max(dot(normal, halfwayDir), 0.0), specular_intensity);
            vec3 specular = point_lights[i].color * spec * specular;
            
            // attenuation
            float attenuation = 1.0 / (1.0 + point_lights[i].linear * distance + point_lights[i].quadratic * distance * distance);
            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular;
        }
    }

    return vec4(lighting, 1.0); // alpha
}

vec3 calculate_view_position(vec2 texture_coordinate, float depth, vec2 scale_factor)  // "scale_factor" is "v_fov_scale".
{
    vec2 half_ndc_position = vec2(0.5) - texture_coordinate;    // No need to multiply by two, because we already baked that into "v_tan_fov.xy".
    vec3 view_space_position = vec3(half_ndc_position * scale_factor.xy * -depth, -depth); // "-depth" because in OpenGL the camera is staring down the -z axis (and we're storing the unsigned depth).
    return(view_space_position);
}

float get_depth(float z_ndc)
{
    float farZ = depth_range.y;
    float nearZ = depth_range.x;
    
    return (((farZ-nearZ) * z_ndc) + nearZ + farZ) / 2.0;
}

vec4 clip_from_depth(float depth, in vec2 tex_coord)
{
    float z = (depth * 2.0 - 1.0);
    
    return vec4(tex_coord * 2.0 - 1.0, z, 1.0);
}

vec4 view_from_depth(float depth, in vec2 tex_coord, in mat4 inv_proj)
{
    vec4 clip = clip_from_depth(depth, tex_coord);
    vec4 view = inv_proj * clip;

    // Inverse perspective division. (Same as regular division)
    view /= view.w;

    return view;
}

// Per-pixel matrix calculation.
vec3 world_from_depth(float depth, in vec2 tex_coord, in mat4 inv_proj, in mat4 inv_view)
{
    vec4 view = view_from_depth(depth, tex_coord, inv_proj);
    vec4 world = inv_view * view;

    return world.xyz;
}

vec4 CalcEyeFromWindow(in float windowZ, in vec3 eyeDirection, in mat4 perspective)
{
    //windowZ = windowZ * 2.0 - 1.0;

    float ndcZ = (2.0 * windowZ - depth_range.x - depth_range.y) / (depth_range.y - depth_range.x);

    //float ndcZ = windowZ;
    //float ndcZ = ((2.0 + depth_range.x) / (depth_range.y + depth_range.x - windowZ * (depth_range.y - depth_range.x)));
    //float ndcZ = get_depth(windowZ);
    
    float eyeZ = perspective[3][2] / ((perspective[2][3] * ndcZ) - perspective[2][2]);
    
    return vec4(eyeDirection * eyeZ, 1);
    //return vec4(TexCoords * 2.0 - 1.0, eyeZ, 1.0);
}

void main()
{
    #if LAYER_POSITION_ENABLED
        vec3 world_position = texture(g_position, TexCoords).rgb;
        output_color = exec(world_position);
        //output_color = vec4(1.0, 1.0, 1.0, 1.0);
    #else
        #if LAYER_DEPTH_ENABLED
            float depth = texture(g_depth, TexCoords).x; // .r;

            //depth = depth*2.0-1.0;

            // Slow, but stable method.
            ////vec3 world_position = world_from_depth(depth, TexCoords, inv_projection, inv_view);

            // Linearize depth:
            //depth = ((2.0 + 0.1) / (1000.0 + 0.1 - depth * (1000.0 - 0.1)));
            //depth = (2.0 * depth - depth_range.x - depth_range.y) / (depth_range.y - depth_range.x);
            //depth = ((2.0 + depth_range.x) / (depth_range.y + depth_range.x - depth * (depth_range.y - depth_range.x)));
            //depth = 2.0 * depth - 1.0;

            /*

            vec4 position = vec4(TexCoords*2.0 - 1.0, depth, 1.0);

            //position = ((inv_view*inv_projection)*position);
            position = (inv_projview * position);

            position /= position.w;

            vec3 world_position = (position.xyz);
            */

            ///*
            vec3 eye_ray = eye_direction;

            //vec3 eye_ray = (eye_direction + 1.0) * 0.5;

            vec4 view_position = CalcEyeFromWindow(depth, eye_ray, projection);

            vec4 wp = (inv_view * view_position);

            //vec3 world_position = view_position.xyz;
            vec3 world_position = wp.xyz;

            //world_position = world_from_depth(depth, TexCoords, inv_projection, inv_view);

            //vec3 world_position = (world_position / world_position.w).xyz;
            //*/

            //vec4 eye_position = CalcEyeFromWindow((texture(g_depth, TexCoords).x), eye_direction); // vec3((eye_direction * 0.5) + 1.0)
            //vec4 eye_position = vec4(eye_direction, 1.0);
            //vec4 eye_position = vec4(calculate_view_position(TexCoords, texture(g_depth, TexCoords).x, half_size_near_plane * (depth_range.y - depth_range.x)), 1.0);
            //vec4 test = ((inv_view * eye_position));
            //vec3 world_position = (test.xyz / test.w);
            
            //vec3 world_position = ((inv_view * eye_position)).xyz;
            //vec3 world_position = (inv_projection * inv_view * eye_position).xyz;
            //vec3 world_position = (inv_projection * eye_position).xyz;
            //vec3 world_position = eye_position.xyz;

            //float depth = texture(g_depth, TexCoords).r;
            //output_color = vec4(depth, 0.0, 0.0, 1.0);
            //output_color = vec4(depth, depth, depth, 1.0);

            //output_color = eye_position;
            //output_color = vec4(eye_direction * (texture(g_depth, TexCoords).x), 1.0);
            //output_color = vec4(eye_position.xyz, 1.0);
            
            //output_color = vec4((eye_direction * 0.5) + 1.0, 1.0);


            //output_color = test;
            //output_color = vec4(1.0, 1.0, 0.0, 1.0);
            //output_color = vec4(gl_FragCoord.xyz, 1.0);
            //output_color = vec4(_test.x, _test.y, 0.0, 1.0);
            //output_color = vec4(TexCoords.x, TexCoords.y, 0.0, 1.0);
            //output_color = vec4(eye_direction.xyz, 1.0);
            
            //output_color = vec4(depth_range.x, depth_range.y / 8000.0, 0.0, 1.0);
            //output_color = vec4((eye_direction + 1.0) * 0.5, 1.0);
            
            output_color = vec4(world_position, 1.0);

            //output_color = exec(world_position);
        #else
            discard;
        #endif
    #endif
}