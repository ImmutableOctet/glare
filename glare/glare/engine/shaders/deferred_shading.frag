#version 330 core
//#version 400

// Layer sample UVs.
in vec2 TexCoords;

// Output:
out vec4 output_color;

// World-space position of the view/camera.
uniform vec3 view_position;

// Ambient light level of the scene.
uniform vec3 ambient_light = vec3(0.1, 0.1, 0.1);

uniform sampler2D g_normal;
uniform sampler2D g_albedo_specular;

#if LAYER_DEPTH_ENABLED
    uniform sampler2D g_depth;
#endif

#if LAYER_POSITION_ENABLED
    uniform sampler2D g_position;
#else
    // These may be moved later on (e.g. for use with screen-space effects):
    uniform mat4 inv_view;
    //uniform mat4 inv_projection;

    // TODO: Determine if we need to pass only one inverse matrix.
    //uniform mat4 inv_projview;

    uniform mat4 projection;

    uniform vec2 depth_range = vec2(0.0, 1.0);
    uniform vec2 half_size_near_plane;

    // Eye-ray; used for world-position reconstruction.
    in vec3 eye_direction;
#endif

//uniform sampler2D g_render_flags; // highp
uniform usampler2D g_render_flags;

const uint FLAG_SHADOWMAP = 1u; // (1u << 0u);
const uint FLAG_LIGHTING = 2u; // (1u << 1u);

const bool render_flags_enabled = true; // uniform
//const bool render_flags_enabled = false;

//uniform float alpha = 1.0; // const
//uniform bool specular_available = false;


// Directional lights:
const int MAX_DIR_LIGHTS = 4; // 8;
uniform int directional_lights_count; // = 0;

struct DirectionalLight
{
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool use_position;
};

uniform DirectionalLight directional_lights[MAX_DIR_LIGHTS];

// Directional shadows:
const int MAX_DIR_SHADOWS = 4;
uniform int directional_shadows_count; // = 0;

uniform vec3 directional_shadow_light_position[MAX_DIR_SHADOWS];
uniform sampler2D directional_shadow_map[MAX_DIR_SHADOWS];

uniform mat4 directional_shadow_light_space_matrix[MAX_DIR_SHADOWS];

float directional_shadow_calculation(sampler2D shadow_map, vec4 fragment_position_light_space, vec3 light_position, vec3 fragment_position, vec3 fragNormal)
{
    // Perform perspective divide.
    vec3 projected_coordinates = (fragment_position_light_space.xyz / fragment_position_light_space.w);
    
    // Convert from (-1.0 to 1.0) range into (0.0 to 1.0) range.
    projected_coordinates = projected_coordinates * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closest_depth = texture(shadow_map, projected_coordinates.xy).r; 
    
    // Get the depth of the current fragment from the light's perspective.
    float currentDepth = projected_coordinates.z;
    
    // Calculate bias based on depth-map resolution and slope:
    vec3 normal = normalize(fragNormal);
    vec3 light_direction = normalize(light_position - fragment_position);
    
    float bias = max(0.05 * (1.0 - dot(normal, light_direction)), 0.005);
    
    float shadow = 0.0;

    vec2 texel_size = (1.0 / textureSize(shadow_map, 0));

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcf_depth = texture(shadow_map, projected_coordinates.xy + vec2(x, y) * texel_size).r;

            shadow += (((currentDepth - bias) > pcf_depth) ? 1.0 : 0.0);
        }    
    }

    shadow /= 9.0;
    
    // Fragments at a depth greater than 1.0 (outside of the light frustum's far-plane) should be completely in shadow.
    if (projected_coordinates.z > 1.0)
    {
        shadow = 0.0;
    }
        
    return shadow;
}


// Point-light shadows:
const int MAX_POINT_SHADOWS = 2;

uniform int point_shadows_count;

uniform vec3 point_shadow_light_position[MAX_POINT_SHADOWS]; // light_position;
uniform float point_shadow_far_plane[MAX_POINT_SHADOWS]; // far_plane;

uniform samplerCube point_shadow_cubemap[MAX_POINT_SHADOWS];

// Array of offset directions for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float point_shadow_calculation(samplerCube shadow_map, vec3 fragment_position, vec3 light_position, vec3 view_position, float far_plane, vec3 normal) // out vec3 dbg_color
{
    vec3 fragment_to_light = (fragment_position - light_position);
    float currentDepth = length(fragment_to_light);
    float shadow = 0.0;

    vec3 light_direction = normalize(fragment_to_light);
    
    ////float bias = 0.15;

    float bias = max(0.01 * (0.2 - dot(normal, light_direction)), (length(fragment_to_light) / far_plane));
    //float bias = (length(fragment_to_light) / far_plane);

    //float bias = max(0.15 * (1.0 - dot(normal, light_direction)), 0.01);
    //float bias = max(0.05 * (1.0 - dot(normal, light_direction)), 0.005);

    int samples = 20;
    
    float view_distance = length(view_position - fragment_position);
    float disk_radius = (1.0 + (view_distance / far_plane)) / 1000.0;
    
    for(int i = 0; i < samples; ++i)
    {
        float closest_depth = texture(shadow_map, fragment_to_light + gridSamplingDisk[i] * disk_radius).r;
        closest_depth *= far_plane;   // undo mapping [0;1]

        if(currentDepth - bias > closest_depth)
        {
            shadow += 1.0;
        }
    }

    shadow /= float(samples);
        
    // display closest_depth as debug (to visualize depth cubemap)
    //dbg_color = vec3(closest_depth / far_plane);
        
    return shadow;
}

// Spotlights:
const int MAX_SPOT_LIGHTS = 16; // 8;
uniform int spot_lights_count; // = 0;

struct SpotLight
{
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float cutoff;
    float outer_cutoff;

    float constant;
    float linear;
    float quadratic;
};

uniform SpotLight spot_lights[MAX_SPOT_LIGHTS];


// Point lights:
struct PointLight
{
    vec3 position;

    //vec3 ambient;
    vec3 diffuse;
    //vec3 specular;
    
    float constant;
    float linear;
    float quadratic;

    float radius;
};

const float specular_intensity = 4.0; // 8.0;
const bool limit_to_radius = true;

const int MAX_POINT_LIGHTS = 32; // 16; // 128;

uniform int point_lights_count = 0;
uniform PointLight point_lights[MAX_POINT_LIGHTS];

vec3 compute_point_lights(in vec3 world_position, in vec3 normal, in vec3 diffuse_in, float specular_in, vec3 view_direction)
{
    vec3 lighting = vec3(0.0, 0.0, 0.0); //diffuse_in;
    vec3 diffuse = diffuse_in;

    for (int i = 0; i < point_lights_count; ++i)
    {
        // calculate distance between light source and current fragment
        float distance = length(point_lights[i].position - world_position);

        if ((!limit_to_radius) || (distance < point_lights[i].radius))
        {
            // Diffuse:
            vec3 light_direction = normalize(point_lights[i].position - world_position);
            vec3 diffuse = max(dot(normal, light_direction), 0.0) * diffuse * point_lights[i].diffuse;

            // Specular:
            vec3 mid_point = normalize(light_direction + view_direction);

            float specular_factor = pow(max(dot(normal, mid_point), 0.0), specular_intensity);
            vec3 specular = (point_lights[i].diffuse * specular_factor * specular_in);
            
            // Attenuation:
            float attenuation = (1.0 / (point_lights[i].constant + (point_lights[i].linear * distance) + (point_lights[i].quadratic * (distance * distance))));

            diffuse *= attenuation;
            specular *= attenuation;

            lighting += (diffuse + specular);
        }
    }

    return lighting;
}

vec3 calculate_directional_light(in vec3 world_position, in vec3 normal, in vec3 view_direction, in DirectionalLight light, vec3 diffuse_in, float lighting)
{
    vec3 light_direction = ((light.use_position) ? normalize(light.position - world_position) : normalize(-light.direction));

    // Diffuse:
    float diffuse_factor = max(dot(light_direction, normal), 0.0);
    
    vec3 diffuse = diffuse_factor * light.diffuse;
    //vec3 diffuse = (light.diffuse * diffuse_factor * diffuse_in);

    vec3 reflect_direction = reflect(-light_direction, normal);

    
    // Specular:
    vec3 mid_point = normalize(light_direction + view_direction);

    float specular_factor = pow(max(dot(normal, mid_point), 0.0), 64.0);
    //float specular_factor = pow(max(dot(view_direction, reflect_direction), 0.0), 16.0); // 32.0 // shininess

    vec3 specular = (specular_factor * light.specular * light.diffuse);

    // Combine results:
    vec3 ambient = (light.ambient * diffuse_in);
    
    //vec3 specular = (light.specular * specular_factor * specular_in);
    //return (ambient + diffuse + specular);

    return (ambient + lighting * (diffuse + specular)) * diffuse_in;
}

const float max_shadow = 0.99; // 0.95; // 0.85;

vec3 shade_pixel(in vec3 world_position, in vec3 view_position, in vec3 normal, vec3 diffuse_in, float specular, uint render_flags)
{
    vec3 diffuse = vec3(0.0, 0.0, 0.0); //diffuse_in; // (diffuse_in * ambient_light);

    //diffuse *= 1.0 - ambient_light;

    vec3 view_direction = normalize(view_position - world_position); // normalize(view_position) - normalize(world_position);

    float shadow = 0.0;

    if ((render_flags & FLAG_SHADOWMAP) > 0u)
    {
        for (int layer = 0; layer < directional_shadows_count; layer++)
        {
            vec4 dir_shadow_frag_position = (directional_shadow_light_space_matrix[layer] * vec4(world_position.xyz, 1.0));
            //vec4 dir_shadow_frag_position = (directional_shadow_light_space_matrix[layer] * vec4(vec3(model * vec4(a_position, 1.0)), 1.0));

            shadow += directional_shadow_calculation(directional_shadow_map[layer], dir_shadow_frag_position, directional_shadow_light_position[layer], world_position, normal);
        }

        for (int layer = 0; layer < point_shadows_count; layer++)
        {
            shadow += point_shadow_calculation(point_shadow_cubemap[layer], world_position, point_shadow_light_position[layer], view_position, point_shadow_far_plane[layer], normal);
        }
    }
    /*
    else
    {
        shadow = 1.0;
    }
    */

    float ambient_strength = min(length(ambient_light), (1.0 - max_shadow));
    //float ambient_strength = max_shadow;
    //float light = (1.0 - min(shadow, max_shadow));
    float light = (1.0 - min(shadow, (1.0 - ambient_strength)));

    specular = (pow(light, 2) * specular);
    //float dir_light_intensity = 0.5; // 1.0; //(ambient * 2.0);

    vec3 ambient;

    if ((render_flags & FLAG_LIGHTING) > 0u)
    {
        for (int i = 0; i < directional_lights_count; i++)
        {
            diffuse += calculate_directional_light(world_position, normal, view_direction, directional_lights[i], diffuse_in, light);
        }

        //diffuse = (light * diffuse);

        diffuse += compute_point_lights(world_position, normal, diffuse_in, specular, view_direction); // , diffuse

        diffuse = (light * diffuse);

        ambient = (ambient_light * diffuse_in); // diffuse
    }
    else
    {
       diffuse = diffuse_in;
       ambient = vec3(0.0, 0.0, 0.0);
    }

    
    //lighting = (light * lighting);

    //return lighting;

    //return vec4(0.0, 0.0, diffuse.b, 1.0);
    //return vec4(diffuse, 1.0);
    
    //return vec4(lighting, 1.0); // alpha

    //vec3 p = lighting;

    //return vec4(p, 1.0);

    //float ambient_strength = length(ambient_light);

    //specular = pow(light, 2);

    vec3 lighting = ambient + (light * (diffuse));


    /*
    if (render_flags > 0u) // > 0u
    //if (true)
    {
        lighting = vec3(1.0, 1.0, 1.0);
    }
    else
    {
        lighting = vec3(1.0, 0.0, 0.0);
    }
    */

    //return diffuse;
    return lighting;
}

/*
float get_linear_depth(float z_ndc)
{
    float farZ = depth_range.y;
    float nearZ = depth_range.x;
    
    return (((farZ-nearZ) * z_ndc) + nearZ + farZ) / 2.0;
}
*/

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

vec4 calculate_eye_from_window(in float window_z, in vec3 eye_direction, in mat4 perspective)
{
    //window_z = window_z * 2.0 - 1.0;

    float ndcZ = (2.0 * window_z - depth_range.x - depth_range.y) / (depth_range.y - depth_range.x);

    //float ndcZ = window_z;
    //float ndcZ = ((2.0 + depth_range.x) / (depth_range.y + depth_range.x - window_z * (depth_range.y - depth_range.x)));
    //float ndcZ = get_depth(window_z);
    
    float eyeZ = perspective[3][2] / ((perspective[2][3] * ndcZ) - perspective[2][2]);
    
    return vec4(eye_direction * eyeZ, 1);
    //return vec4(TexCoords * 2.0 - 1.0, eyeZ, 1.0);
}

vec3 get_world_position(vec2 uv)
{
    #if LAYER_POSITION_ENABLED
        return texture(g_position, uv).rgb;
    #else
        #if LAYER_DEPTH_ENABLED
            float depth = texture(g_depth, uv).x; // .r;

            // Slow, but stable method.
            ////vec3 world_position = world_from_depth(depth, uv, inv_projection, inv_view);

            vec3 eye_ray = eye_direction;

            vec4 view_position = calculate_eye_from_window(depth, eye_ray, projection);
            return (inv_view * view_position).xyz;
        #else
            return vec3(0.0, 0.0, 0.0);
        #endif
    #endif
}

uint texture2DLinear(vec2 image_size, usampler2D texture_sampler, vec2 uv)
{
    vec2 pixel_offset = (vec2(0.5, 0.5) / image_size);

    vec2 uv_precision = fract(uv * image_size);
    
    if (abs(uv_precision.x - 0.5) < 0.001 || abs(uv_precision.y - 0.5) < 0.001)
    {
        pixel_offset = (pixel_offset - vec2(0.00001, 0.00001));
    }

    vec4 top_left     = vec4(texture(texture_sampler, uv + vec2(-pixel_offset.x, -pixel_offset.y))) * 255.0;
    vec4 top_right    = vec4(texture(texture_sampler, uv + vec2(pixel_offset.x,  -pixel_offset.y))) * 255.0;
    vec4 bottom_left  = vec4(texture(texture_sampler, uv + vec2(-pixel_offset.x,  pixel_offset.y))) * 255.0;
    vec4 bottom_right = vec4(texture(texture_sampler, uv + vec2(pixel_offset.x,   pixel_offset.y))) * 255.0;

    vec2 interpolation_factor = fract((uv.xy - pixel_offset) * image_size);

    vec4 top    = mix(top_left, top_right, interpolation_factor.x);
    vec4 bottom = mix(bottom_left, bottom_right, interpolation_factor.x);

    return uint((mix(top, bottom, interpolation_factor.y).r) / 255.0);
}

uint get_render_flags(vec2 uv)
{
    if (render_flags_enabled)
    {
        vec2 size = textureSize(g_render_flags, 0);

        ivec2 pixel_position = ivec2(int(uv.x * size.x), int(uv.y * size.y));
        //ivec2 pixel_position = ivec2(gl_FragCoord.xy);

        return (texelFetch(g_render_flags, pixel_position, 0).r); // floatBitsToUint
        //return (texture(g_render_flags, uv).r); // floatBitsToUint

        //return textureGather(g_render_flags, uv).r;
        //return texture2DLinear(textureSize(g_render_flags, 0), g_render_flags, uv);
        //return texelFetchOffset(g_render_flags, ivec2(int(uv.x), int(uv.y)), 0, ivec2(0, 0)).r;
    }

    return 255u;
}

void main()
{
    vec2 uv = TexCoords;
    vec3 world_position = get_world_position(uv);

    // G-Buffer Layers:
    vec3  normal       = texture(g_normal, uv).rgb;
    vec3  diffuse      = texture(g_albedo_specular, uv).rgb;
    float specular     = texture(g_albedo_specular, uv).a;
    uint  render_flags = get_render_flags(uv);

    /*
    //if (render_flags < 255u)
    if ((render_flags & FLAG_SHADOWMAP) > 0u)
    {
        diffuse = vec3(1.0, 1.0, 1.0);
        //diffuse = vec3(0.8, 0.0, 0.0);
        //diffuse = (vec3(float(render_flags)/1.0, float(render_flags)/1.0, float(render_flags)/1.0) * diffuse);
    }
    else
    {
        diffuse = vec3(0.1, 0.1, 0.1);
    }
    */

    //output_color = vec4(diffuse, 1.0); return;

    output_color = vec4(shade_pixel(world_position, view_position, normal, diffuse, specular, render_flags), 1.0);
    //output_color = vec4(1.0, 1.0, 1.0, 1.0);
}