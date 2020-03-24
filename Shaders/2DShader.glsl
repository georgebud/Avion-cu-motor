#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

uniform vec3 object_color;

// Output value to fragment shader
out vec3 color;

void main()
{
	color = object_color;
	gl_Position = Model * vec4(v_position, 1.0);
}
