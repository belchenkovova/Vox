#version 400 core

in vec3			in_position;

uniform mat4	uniform_projection;
uniform mat4	uniform_view;
uniform mat4	uniform_transformation;

void			main()
{
	gl_Position = uniform_projection * uniform_view * uniform_transformation * vec4(in_position, 1.0);
}