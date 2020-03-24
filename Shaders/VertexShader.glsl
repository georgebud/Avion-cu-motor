#version 330

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Uniforms for light properties
uniform vec3 light_position;
uniform vec3 eye_position;
uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

uniform vec3 object_color;

// Output value to fragment shader
out vec3 color;

void main()
{
	// compute world space vectors
	vec3 world_pos = (Model * vec4(v_position,1)).xyz;
	vec3 world_normal = normalize( mat3(Model) * v_normal );

	vec3 N = normalize( world_normal );
	vec3 L = normalize( light_position - world_pos );
	vec3 V = normalize( eye_position - world_pos );
	vec3 H = normalize( L + V );

	// define ambient light component
	float ambient_light = 0.75;

	// compute diffuse light component
	float diffuse_light = material_kd * max (dot(N, L), 0);

	// compute specular light component
	float specular_light = 0;

	if (diffuse_light > 0)
	{
		specular_light = material_ks * pow(max(dot(normalize(N), H), 0), material_shininess);
	}

	// compute light
	float d	= distance(light_position, world_pos);
	float attenuation_factor = 1 / (1 + 0.14 * d + 0.007 * d * d);
	float light = ambient_light + attenuation_factor * (diffuse_light + specular_light);

	//float light = ambient_light + specular_light + diffuse_light;

	// send color light output to fragment shader
	color = object_color * light;
	//color = object_color;

	gl_Position = Projection * View * Model * vec4(v_position, 1.0);
}
