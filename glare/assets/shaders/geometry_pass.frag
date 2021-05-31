#version 330 core

#if LAYER_POSITION_ENABLED
    layout (location = 0) out vec3 g_position;
    layout (location = 1) out vec3 g_normal;
    layout (location = 2) out vec4 g_albedo_specular;
#else
    layout (location = 0) out vec3 g_normal;
    layout (location = 1) out vec4 g_albedo_specular;
#endif

in vec2 uv;
in vec3 fragment_position;
in vec3 normal;

uniform sampler2D diffuse;
uniform sampler2D specular;

uniform vec3 view_position;


// Point shadows:
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


uniform bool texture_diffuse_enabled = false;
uniform bool specular_available = false;

uniform vec4 diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform float alpha = 1.0;


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

void main()
{
    #if LAYER_POSITION_ENABLED
        // store the fragment position vector in the first gbuffer texture
        g_position = fragment_position; // vec3(1.0, 0.0, 0.0);
    #endif

    // also store the per-fragment normals into the gbuffer
    g_normal = normalize(normal);

    vec4 albedo_specular = vec4(1.0, 1.0, 1.0, 1.0);

    // and the diffuse per-fragment color
    if (texture_diffuse_enabled)
    {
        albedo_specular.rgb = (texture(diffuse, uv).rgb * diffuse_color.rgb);
    }
    else
    {
        albedo_specular.rgb = (diffuse_color.rgb);
    }

    if (specular_available)
    {
        // store specular intensity in g_albedo_specular's alpha component
        albedo_specular.a = texture(specular, uv).r;
    }
    else
    {
        albedo_specular.a = 1.0; //0.5;
    }
    
    float shadow = 0.0;

    if (point_shadows_count > 0)
    {
        for (int layer = 0; layer < point_shadows_count; layer++)
        {
            shadow += point_shadow_calculation(point_shadow_cubemap[layer], fragment_position, point_shadow_light_position[layer], view_position, point_shadow_far_plane[layer]); // g_position
        }
    }

    float light = (1.0 - min(shadow, 0.85));
    g_albedo_specular = vec4((light * albedo_specular.rgb), (pow(light, 2) * albedo_specular.a));

    ////g_albedo_specular = (light * albedo_specular);

    // Tests:
    //g_albedo_specular = texture(directional_shadow_map, uv);
    //g_albedo_specular = vec4(directional_shadows.fragment_position.xyz, 1.0);


    //g_albedo_specular = vec4((1.0 - shadow) * albedo_specular.rgb, albedo_specular.a);

    //output_color.a *= alpha;

    //if (output_color.a < 0.01)
    //    discard;
}