#version 330 core
out vec4 output_color;

in vec2 TexCoords;

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo_specular;

struct PointLight
{
    vec3 position;
    vec3 color;
    
    float linear;
    float quadratic;
    float radius;
};

const int point_lights_count = 32; // 16; // 128;

const float specular_intensity = 16.0; // 8.0;
const bool limit_to_radius = false; // true;

uniform PointLight point_lights[point_lights_count];
uniform vec3 view_position;

//uniform float ambient_light = 0.8; // 0.1;
uniform vec3 ambient_light = vec3(0.8, 0.8, 0.8);

//uniform float alpha = 1.0; // const
//uniform bool specular_available = false;

void main()
{             
    // G-Buffer Layers:
    vec3  fragment_position  = texture(g_position, TexCoords).rgb;
    vec3  normal   = texture(g_normal, TexCoords).rgb;
    vec3  diffuse  = texture(g_albedo_specular, TexCoords).rgb;
    float specular = texture(g_albedo_specular, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 lighting  = diffuse * ambient_light;
    vec3 view_direction  = normalize(view_position - fragment_position); // normalize(view_position) - normalize(fragment_position);

    for (int i = 0; i < point_lights_count; ++i)
    {
        // calculate distance between light source and current fragment
        float distance = length(point_lights[i].position - fragment_position);

        if ((!limit_to_radius) || (distance < point_lights[i].radius))
        {
            // diffuse
            vec3 lightDir = normalize(point_lights[i].position - fragment_position);
            vec3 diffuse = max(dot(normal, lightDir), 0.0) * diffuse * point_lights[i].color;

            // specular
            vec3 halfwayDir = normalize(lightDir + view_direction);  
            float spec = pow(max(dot(normal, halfwayDir), 0.0), specular_intensity);
            vec3 specular = point_lights[i].color * spec * specular;
            
            // attenuation
            float attenuation = 1.0 / (1.0 + point_lights[i].linear * distance + point_lights[i].quadratic * distance * distance);
            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular;
        }
    }


    output_color = vec4(lighting, 1.0); // alpha
}
