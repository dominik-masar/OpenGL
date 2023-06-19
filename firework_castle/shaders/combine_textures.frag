#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;  // The vertex texture coordinates.
} in_data;

// The texture to display.
layout (binding = 0) uniform sampler2D final_tex;
layout (binding = 1) uniform sampler2D mask_tex;
layout (binding = 2) uniform sampler2D reflection_tex;

uniform float mirror_factor;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final output color.
layout (location = 0) out vec4 final_color;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	vec3 final_light = texture(final_tex, in_data.tex_coord).rgb;
	vec4 mask_color = texture(mask_tex, in_data.tex_coord);

	if (mask_color.r > 0.5) {
		vec3 reflection = texture(reflection_tex, in_data.tex_coord).rgb;
		final_light = mix(final_light, reflection, mirror_factor);
	}

	final_color = vec4(final_light, 1.0);
}