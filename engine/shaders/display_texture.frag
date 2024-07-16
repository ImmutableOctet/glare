out vec4 fragment_color;
in vec2 texture_uv;
  
uniform sampler2D display;
  
void main()
{
    fragment_color = texture(display, texture_uv);
}