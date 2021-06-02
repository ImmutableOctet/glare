#version 330 core

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

float directional_shadow_calculation(sampler2D shadow_map, vec4 fragPosLightSpace, vec3 lightPos, vec3 fragPos, vec3 fragNormal)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadow_map, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadow_map, 0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadow_map, projCoords.xy + vec2(x, y) * texelSize).r;

            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }

    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}


// Point-light shadows:
const int MAX_POINT_SHADOWS = 1;

uniform int point_shadows_count;

uniform vec3 point_shadow_light_position[MAX_POINT_SHADOWS]; // lightPos;
uniform float point_shadow_far_plane[MAX_POINT_SHADOWS]; // far_plane;

uniform samplerCube point_shadow_cubemap[MAX_POINT_SHADOWS];

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float point_shadow_calculation(samplerCube shadow_map, vec3 fragPos, vec3 lightPos, vec3 viewPos, float far_plane)
{
    // get vector between fragment position and light position
    vec3 fragToLight = (fragPos - lightPos);
    // use the fragment to light vector to sample from the depth map    
    // float closestDepth = texture(shadow_map, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    // closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    // float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    // float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;
    // PCF
    // float shadow = 0.0;
    // float bias = 0.05; 
    // float samples = 4.0;
    // float offset = 0.1;
    // for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    // {
        // for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        // {
            // for(float z = -offset; z < offset; z += offset / (samples * 0.5))
            // {
                // float closestDepth = texture(shadow_map, fragToLight + vec3(x, y, z)).r; // use lightdir to lookup cubemap
                // closestDepth *= far_plane;   // Undo mapping [0;1]
                // if(currentDepth - bias > closestDepth)
                    // shadow += 1.0;
            // }
        // }
    // }
    // shadow /= (samples * samples * samples);
    float shadow = 0.0;
    float bias = 0.15; // 0.15;

    //vec3 lightDir = normalize(fragToLight);
    //float bias = max(0.15 * (1.0 - dot(normal, lightDir)), 0.01);
    //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  

    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 1000.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(shadow_map, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
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
    vec3 lighting = diffuse_in;
    vec3 diffuse = diffuse_in;

    for (int i = 0; i < point_lights_count; ++i)
    {
        // calculate distance between light source and current fragment
        float distance = length(point_lights[i].position - world_position);

        if ((!limit_to_radius) || (distance < point_lights[i].radius))
        {
            // diffuse
            vec3 lightDir = normalize(point_lights[i].position - world_position);
            vec3 diffuse = max(dot(normal, lightDir), 0.0) * diffuse * point_lights[i].diffuse;

            // specular
            vec3 halfwayDir = normalize(lightDir + view_direction);  
            float spec = pow(max(dot(normal, halfwayDir), 0.0), specular_intensity);
            vec3 specular = point_lights[i].diffuse * spec * specular_in;
            
            // attenuation
            float attenuation = 1.0 / (1.0 + point_lights[i].linear * distance + point_lights[i].quadratic * distance * distance);

            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular;
        }
    }

    return lighting;
}

vec3 calculate_directional_light(in vec3 world_position, in vec3 normal, in vec3 view_direction, in DirectionalLight light, vec3 diffuse_in, float lighting)
{
    vec3 light_direction = ((light.use_position) ? normalize(light.position - world_position) : normalize(-light.direction));

    // Diffuse shading:
    float diff = max(dot(light_direction, normal), 0.0);
    
    vec3 diffuse = diff * light.diffuse;
    //vec3 diffuse = (light.diffuse * diff * diffuse_in);

    // Specular shading:
    vec3 reflect_direction = reflect(-light_direction, normal);

    vec3 halfway_dir = normalize(light_direction + view_direction);
    float spec = pow(max(dot(normal, halfway_dir), 0.0), 64.0);
    //float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 16.0); // 32.0 // shininess

    vec3 specular = (spec * light.specular * light.diffuse);

    // Combine results:
    vec3 ambient = (light.ambient * diffuse_in);
    
    //vec3 specular = light.specular * spec * specular_in;
    //return (ambient + diffuse + specular);

    return (ambient + lighting * (diffuse + specular)) * diffuse_in;
}

// Old:
/*
vec3 calculate_directional_light(in vec3 lighting, in vec3 world_position, in vec3 normal, in vec3 view_direction, in DirectionalLight light, float base_dist=1000.0) // vec4
{
    vec3 delta = (light.position - world_position);

    float dist = abs(length(delta));
    vec3 light_dir = normalize(delta);

    float diff = max(dot(light_dir, normal), 0.0);
    vec3 diffuse = diff * light.color;

    vec3 reflectDir = reflect(-light_dir, normal);
    
    vec3 halfwayDir = normalize(light_dir + view_direction);

    float specular = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    
    float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
	float linear = 0.1; // 0.000005f; // 0.7f; // 0.005f;
	float quadratic = 1.0 / 2000.0; // 2000.0; // 1.8f; // 0.0005f;

    float attenuation = 1.0 / (1.0 + linear * dist + quadratic * dist * dist);

    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;

    //return vec4((diffuse * light.intensity * ((base_dist - dist) / base_dist)), specular);

    return lighting;
}
*/

const float max_shadow = 0.95; // 0.85;

vec3 shade_pixel(in vec3 world_position, in vec3 view_position, in vec3 normal, vec3 diffuse_in, float specular)
{
    vec3 diffuse = diffuse_in; // (diffuse_in * ambient_light);

    //diffuse *= 1.0 - ambient_light;

    vec3 view_direction = normalize(view_position - world_position); // normalize(view_position) - normalize(world_position);

    float shadow = 0.0;

    for (int layer = 0; layer < directional_shadows_count; layer++)
    {
        vec4 dir_shadow_frag_position = (directional_shadow_light_space_matrix[layer] * vec4(world_position.xyz, 1.0));
        //vec4 dir_shadow_frag_position = (directional_shadow_light_space_matrix[layer] * vec4(vec3(model * vec4(a_position, 1.0)), 1.0));

        shadow += directional_shadow_calculation(directional_shadow_map[layer], dir_shadow_frag_position, directional_shadow_light_position[layer], world_position, normal);
    }

    for (int layer = 0; layer < point_shadows_count; layer++)
    {
        shadow += point_shadow_calculation(point_shadow_cubemap[layer], world_position, point_shadow_light_position[layer], view_position, point_shadow_far_plane[layer]);
    }

    //float light = (1.0 - min(shadow, max_shadow));
    float ambient_strength = min(length(ambient_light), max_shadow);
    float light =  (1.0 - min(shadow, (1.0 - ambient_strength)));

    //float dir_light_intensity = 0.5; // 1.0; //(ambient * 2.0);

    for (int i = 0; i < directional_lights_count; i++)
    {
        diffuse = calculate_directional_light(world_position, normal, view_direction, directional_lights[i], diffuse_in, light);
    }

    diffuse = (light * diffuse);
    //diffuse = (light * diffuse);

    //vec3 lighting =
    diffuse += compute_point_lights(world_position, normal, diffuse_in, specular, view_direction); // , diffuse


    specular = (pow(light, 2) * specular);
    
    //lighting = (light * lighting);

    //return lighting;

    //return vec4(0.0, 0.0, diffuse.b, 1.0);
    //return vec4(diffuse, 1.0);
    
    //return vec4(lighting, 1.0); // alpha

    //vec3 p = lighting;

    //return vec4(p, 1.0);

    //float ambient_strength = length(ambient_light);

    //specular = pow(light, 2);

    vec3 ambient = (ambient_light * diffuse_in);

    vec3 lighting = ambient + (light * (diffuse));


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

vec3 get_world_position()
{
    #if LAYER_POSITION_ENABLED
        return texture(g_position, TexCoords).rgb;
    #else
        #if LAYER_DEPTH_ENABLED
            float depth = texture(g_depth, TexCoords).x; // .r;

            // Slow, but stable method.
            ////vec3 world_position = world_from_depth(depth, TexCoords, inv_projection, inv_view);

            vec3 eye_ray = eye_direction;

            vec4 view_position = CalcEyeFromWindow(depth, eye_ray, projection);
            return (inv_view * view_position).xyz;
        #else
            return vec3(0.0, 0.0, 0.0);
        #endif
    #endif
}

void main()
{
    vec3 world_position = get_world_position();

    // G-Buffer Layers:
    vec3  normal   = texture(g_normal, TexCoords).rgb;
    vec3  diffuse  = texture(g_albedo_specular, TexCoords).rgb;
    float specular = texture(g_albedo_specular, TexCoords).a;

    output_color = vec4(shade_pixel(world_position, view_position, normal, diffuse, specular), 1.0);
    //output_color = vec4(1.0, 1.0, 1.0, 1.0);
}