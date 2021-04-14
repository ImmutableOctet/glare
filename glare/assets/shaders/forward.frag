#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D diffuse;
uniform bool texture_diffuse_enabled;

uniform vec4 diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform float alpha = 1.0;

void main()
{
    vec4 output_color;
    
    if (texture_diffuse_enabled)
    {
        output_color = texture(diffuse, TexCoords);
        output_color *= diffuse_color;
    }
    else
    {
        output_color = diffuse_color;
    }

    output_color.a *= alpha;

    if (output_color.a < 0.01)
        discard;

    FragColor = output_color;
}