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

uniform bool texture_diffuse_enabled = false;
uniform bool specular_available = false;

uniform vec4 diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform float alpha = 1.0;

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

    ////g_albedo_specular = (light * albedo_specular);

    g_albedo_specular = albedo_specular;

    // Tests:
    //g_albedo_specular = texture(directional_shadow_map, uv);
    //g_albedo_specular = vec4(directional_shadows.fragment_position.xyz, 1.0);


    //g_albedo_specular = vec4((1.0 - shadow) * albedo_specular.rgb, albedo_specular.a);

    //output_color.a *= alpha;

    //if (output_color.a < 0.01)
    //    discard;
}