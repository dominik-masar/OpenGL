#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;    // The texture coordinates for the particle.
	vec4 color;        // The particle color.
	flat int id;	   // The particle id.
} in_data;

// The particle texture.
layout (binding = 0) uniform sampler2D particle_texture;

uniform bool use_black;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final fragment color.
layout (location = 0) out vec4 final_color;
      
// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	float intensity = texture(particle_texture, in_data.tex_coord).r;

	final_color = (use_black) ? vec4(0.0, 0.0, 0.0, intensity) : vec4(in_data.color.rgb * intensity, intensity);
}