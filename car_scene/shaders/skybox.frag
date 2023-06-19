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

layout(binding = 3, std140) uniform Fog {
	vec4 color;
	float density;
} fog;

// The buffer with data about skybox texture
layout(binding = 4) uniform samplerCube skybox;

layout(location = 0) in vec3 tex_coords;

layout(location = 3) uniform bool fog_on = false;

layout(location = 15) uniform bool night_vision = false;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final output color.
layout(location = 0) out vec4 final_color;

void main()
{    
	if (fog_on) {
		final_color = vec4(fog.color);
	} else {
    	final_color = vec4(vec3(texture(skybox, tex_coords)) - (night_vision ? 0.f : .3f), 1.f);
	}
}