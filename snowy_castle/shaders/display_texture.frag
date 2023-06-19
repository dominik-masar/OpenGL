#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;  // The vertex texture coordinates.
} in_data;

// The texture to display.
layout (binding = 0) uniform sampler2D input_tex;
uniform bool is_depth_tex = false;

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
	// We applie the transformation to the color from the texture using the uniform matrix.
	if (is_depth_tex) {
		final_color = vec4(texture(input_tex, in_data.tex_coord).rrr, 1.0);
	} else {
		final_color = texture(input_tex, in_data.tex_coord);
	}
}