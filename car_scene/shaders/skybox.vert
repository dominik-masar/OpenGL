#version 450

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------

// The buffer with data about camera.
layout(binding = 0, std140) uniform Camera {
	mat4 projection;
	mat4 view;
	vec3 position;
} camera;

layout(location = 0) in vec3 position;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------

layout(location = 0) out vec3 tex_coords;

void main()
{
    mat4 tmp_view = mat4(mat3(camera.view));
    tex_coords = position;
    vec4 pos = camera.projection * tmp_view * vec4(position, 1.0);
    gl_Position = pos.xyww;
} 