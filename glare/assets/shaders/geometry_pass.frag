#version 330 core

#if LAYER_POSITION_ENABLED
    layout (location = 0) out vec3 g_position;
    layout (location = 1) out vec3 g_normal;
    layout (location = 2) out vec4 g_albedo_specular;
    //layout (location = 3) out float g_render_flags; // half // uint
    layout (location = 3) out uint g_render_flags;
#else
    layout (location = 0) out vec3 g_normal;
    layout (location = 1) out vec4 g_albedo_specular;
    //layout (location = 2) out float g_render_flags;
    layout (location = 2) out uint g_render_flags;
#endif

in vec2 texture_uv;
in vec3 fragment_position;
in vec3 normal;
in mat3 TBN;

in vec3 tangent_fragment_position;
in vec3 tangent_view_position;

uniform sampler2D diffuse;
uniform sampler2D specular;

uniform sampler2D normal_map;
uniform sampler2D height_map;

uniform vec3 view_position;

uniform bool texture_diffuse_enabled = false;
uniform bool specular_available = false;
uniform bool normal_map_available = false;
uniform bool height_map_available = false;

uniform float height_map_scale = 0.1;
uniform float height_map_min_layers = 8.0; //16.0;
uniform float height_map_max_layers = 32.0;

uniform vec4 diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform float alpha = 1.0;
//uniform float shininess = 16.0;

uniform uint render_flags = 255u; // = 0u; // 0xFFu; // 0u;

// Debugging related:
///*
#if ANIMATION_ENABLED
    uniform bool animated = false;
    //flat in ivec4 dbg_bone_ids;
#endif
//*/

vec2 parallax_mapping(sampler2D depthMap, vec2 texCoords, vec3 viewDir, float heightScale, float minLayers, float maxLayers) // float minLayers=8, float maxLayers=32
{
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));

    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    
    // depth of current layer
    float currentLayerDepth = 0.0;
    
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
      
    while (currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main()
{
    #if LAYER_POSITION_ENABLED
        // store the fragment position vector in the first gbuffer texture
        g_position = fragment_position; // vec3(1.0, 0.0, 0.0);
    #endif

    vec2 uv;

    if (height_map_available)
    //if (false)
    {
        //vec3 tangent_view_direction = normalize(view_position - tangent_fragment_position);
        //vec3 tangent_view_direction = normalize(view_position - fragment_position);

        vec3 tangent_view_direction = normalize(tangent_view_position - tangent_fragment_position);
        //vec3 tangent_view_direction = normalize(tangent_fragment_position - tangent_view_position);
        
        uv = parallax_mapping(height_map, texture_uv, tangent_view_direction, height_map_scale, height_map_min_layers, height_map_max_layers);

        vec2 test = uv; // vec2(1.0-uv.x, uv.y);

        //if ((test.x > 1.0) || (test.x < 0.0))
        //    discard;

        //if ((test.y < 1.0) || (test.y > 1.0))
        //    discard;

        /*
        if ((test.x > 1.0 || test.y > 1.0 || test.x < 0.0 || test.y < 0.0))
            discard;
        */

        //vec3 up = vec3(0.0, 1.0, 0.0);
        //g_albedo_specular = vec4((TBN * up), 1.0); return;

        //if (texture_uv.y > 1.0)
        //    discard;

        //g_albedo_specular = vec4(texture_uv.x, 0.0, 0.0, 1.0); return;
        //g_albedo_specular = vec4(texture_uv.y, 0.0, 0.0, 1.0); return;

        //if (uv.y > 1.0)
        //    g_albedo_specular = vec4(uv.y-1.0, 0.0, 0.0, 1.0); return;

        //g_albedo_specular = vec4(0.0, uv.y, 0.0, 1.0); return;
        //g_albedo_specular = vec4(uv.x, uv.y/10.0, 0.0, 1.0); return;
        //g_albedo_specular = vec4(tangent_view_direction.xyz, 1.0); return;
    }
    else
    {
        uv = texture_uv;
    }

    if (normal_map_available)
    {
        vec3 N = normalize(texture(normal_map, uv).rgb * 2.0 - 1.0);

        //g_normal = normalize(normal);
        //g_normal = normalize(N);
        g_normal = normalize(TBN * N);
    }
    else
    {
        g_normal = normalize(normal);
    }

    vec4 albedo_specular = vec4(1.0, 1.0, 1.0, 1.0);

    // and the diffuse per-fragment color
    if (texture_diffuse_enabled)
    {
        albedo_specular.rgb = (texture(diffuse, uv).rgb * diffuse_color.rgb); // texture_uv
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

    ////g_albedo_specular = (light * albedo_specular);

    g_albedo_specular = albedo_specular;

    // Debugging related:
    /*
    #if ANIMATION_ENABLED
        if (animated)
        {
            float r = 0.0;
            float g = 0.0;
            float b = 0.0;

            if (dbg_bone_ids[0] != -1)
                r = 1.0;

            if (dbg_bone_ids[1] != -1)
                //r = 0.0;
                g = 1.0;

            if (dbg_bone_ids[2] != -1)
                //g = 0.0;
                b = 1.0;

            //g_albedo_specular = vec4(1.0, 1.0, 1.0, 1.0);
            g_albedo_specular = vec4(r, g, b, 1.0);
        }
    #endif
    */

    g_render_flags = render_flags;
    //g_render_flags = uintBitsToFloat(render_flags);

    // Tests:
    //g_albedo_specular = texture(directional_shadow_map, uv);
    //g_albedo_specular = vec4(directional_shadows.fragment_position.xyz, 1.0);


    //g_albedo_specular = vec4((1.0 - shadow) * albedo_specular.rgb, albedo_specular.a);

    //output_color.a *= alpha;

    //if (output_color.a < 0.01)
    //    discard;
}