#version 450 core

//----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec3 position_ws;	  // The vertex position in world space.
	vec3 normal_ws;		  // The vertex normal in world space.
	vec2 tex_coord;		  // The vertex texture coordinates.
} in_data;

uniform bool is_water;

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
	final_color = vec4((is_water) ? 1.0 : 0.0, 0.0, 0.0, 1.0);
}	