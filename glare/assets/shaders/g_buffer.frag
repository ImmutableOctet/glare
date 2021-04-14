#version 330 core
layout (location = 0) out vec3 g_position;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_albedo_specular;

in vec2 uv;
in vec3 fragment_position;
in vec3 normal;

uniform sampler2D diffuse;
uniform sampler2D specular;

uniform bool texture_diffuse_enabled = false;
uniform bool specular_available = false;

uniform vec4 diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform float alpha = 1.0;

void main()
{
    // store the fragment position vector in the first gbuffer texture
    g_position = fragment_position; // vec3(1.0, 0.0, 0.0);

    // also store the per-fragment normals into the gbuffer
    g_normal = normalize(normal);

    // and the diffuse per-fragment color
    if (texture_diffuse_enabled)
    {
        g_albedo_specular.rgb = (texture(diffuse, uv).rgb * diffuse_color.rgb);
    }
    else
    {
        g_albedo_specular.rgb = (diffuse_color.rgb);
    }

    if (specular_available)
    {
        // store specular intensity in g_albedo_specular's alpha component
        g_albedo_specular.a = texture(specular, uv).r;
    }
    else
    {
        g_albedo_specular.a = 1.0; //0.5;
    }

    //output_color.a *= alpha;

    //if (output_color.a < 0.01)
    //    discard;
}