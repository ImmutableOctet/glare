
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

in DIR_SHADOWS
{
    vec4 fragment_position;
} directional_shadows;

uniform sampler2D diffuse;
uniform sampler2D specular;
uniform samplerCube shadow_cubemap;
uniform sampler2D directional_shadow_map;

uniform bool texture_diffuse_enabled = false;
uniform bool specular_available = false;

uniform bool directional_shadows_enabled = true; // false;
uniform bool point_shadows_enabled = false;

uniform vec3 directional_shadow_light_position;

uniform vec3 point_shadow_light_position; // lightPos;
uniform vec3 view_position;

uniform float point_shadow_far_plane; // far_plane;

uniform vec4 diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform float alpha = 1.0;

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
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }

    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
        shadow = 0.0;
        
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

    if (point_shadows_enabled)
    {
        shadow += point_shadow_calculation(shadow_cubemap, fragment_position, point_shadow_light_position, view_position, point_shadow_far_plane); // g_position
    }

    if (directional_shadows_enabled)
    {
        shadow += directional_shadow_calculation(directional_shadow_map, directional_shadows.fragment_position, directional_shadow_light_position, fragment_position, normal);
    }

    float light = (1.0 - min(shadow, 0.85));

    ////g_albedo_specular = (light * albedo_specular);
    g_albedo_specular = vec4((light * albedo_specular.rgb), (pow(light, 2) * albedo_specular.a));

    // Tests:
    //g_albedo_specular = texture(directional_shadow_map, uv);
    //g_albedo_specular = vec4(directional_shadows.fragment_position.xyz, 1.0);


    //g_albedo_specular = vec4((1.0 - shadow) * albedo_specular.rgb, albedo_specular.a);

    //output_color.a *= alpha;

    //if (output_color.a < 0.01)
    //    discard;
}