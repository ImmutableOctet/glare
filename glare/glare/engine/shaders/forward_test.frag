out vec4 FragColor;

uniform vec4 diffuse_color = vec4(0.0, 1.0, 0.0, 1.0);
uniform float alpha = 1.0;

void main()
{
    vec4 output_color;
    
    output_color = diffuse_color;
    output_color.a *= alpha;

    if (output_color.a < 0.01)
        discard;

    FragColor = output_color;
}