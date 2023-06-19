#version 450

#define LOG2 (1.442695)

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------

// The buffer with data about camera.
layout(binding = 0, std140) uniform Camera {
	mat4 projection;
	mat4 view;
	vec3 position;
} camera;

// The representation of one cone light.
struct Light {
    vec4 position;
  
    vec4 ambient_color;
    vec4 diffuse_color;
    vec4 specular_color;

	vec4 direction;
	vec4 cut_off;
};
// The buffer with data about all cone lights.
layout(binding = 1, std430) buffer Lights {
	Light lights[];
};

// The object info
layout(binding = 2, std140) uniform Object {
	mat4 model_matrix;
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
} object;

layout(binding = 3, std140) uniform Fog {
	vec4 color;
	float density;
} fog;

// The position of the current vertex that is being processed.
layout(location = 0) in vec3 position;
// The normal of the current vertex that is being processed.
layout(location = 1) in vec3 normal;
// The coordinations in texture of the current vertex that is being processed.
layout(location = 2) in vec2 texture_coordinate;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The position forwared to fragment shader.
layout(location = 0) out vec3 fs_position;
// The normal forwared to fragment shader.
layout(location = 1) out vec3 fs_normal;
// The texture coordinations forwared to fragment shader.
layout(location = 2) out vec2 fs_texture_coordinate;
// Fog factor forwarded to fragment shader.
layout(location = 10) out float fog_factor;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	vec3 tmp_position = vec3(object.model_matrix * vec4(position, 1.f));
	fs_position = tmp_position;
	vec3 cam_pos = vec3(camera.projection * camera.view * vec4(camera.position, 1.f));

	// fog calculation
	float fog_frag_coord = length(vec3(camera.position) - tmp_position);
	float density_sq = fog.density * fog.density;
	float dist_sq = fog_frag_coord * fog_frag_coord;
	fog_factor = exp2(-density_sq * dist_sq * LOG2);

	fs_normal = transpose(inverse(mat3(object.model_matrix))) * normal;

    fs_texture_coordinate = texture_coordinate;

    gl_Position = camera.projection * camera.view * object.model_matrix * vec4(position, 1.0);
}
