#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D diffuse;

uniform vec4 diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);
uniform float alpha = 1.0;

void main()
{
    vec4 output_color = texture(diffuse, TexCoords);
    output_color *= diffuse_color;
    output_color.a *= alpha;

    FragColor = output_color;
}