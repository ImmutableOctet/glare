#version 330 core

uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_rgb;

out vec3 color;

void main()
{
	vec4 local_position = vec4(a_position, 1.0);

	color = a_rgb;

	gl_Position = projection * view * local_position;
}