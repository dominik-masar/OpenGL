#version 450

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
// The intpolated position from the vertex shader.
layout(location = 0) in vec3 fs_position;
// The interpolated color from the vertex shader.
layout(location = 1) in vec3 fs_color;
// Fog factor for computing fog.
layout(location = 10) in float fog_factor;

layout(location = 3) uniform bool fog_on = false;

layout(binding = 3, std140) uniform Fog {
	vec4 color;
	float density;
} fog;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------

// The final output color.
layout(location = 0) out vec4 final_color;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
    vec3 result = fs_color / (fs_color + 1.0); // tone mapping
    result = pow(result, vec3(1.0 / 2.2));     // gamma correction

    if (fog_on) {
        result = mix(vec3(fog.color), result, fog_factor);
    }

	final_color = vec4(result, 1.0);
}
