#version 450 core

// The input primitive specification.
layout (points) in;
// The ouput primitive specification.
layout (triangle_strip, max_vertices = 4) out;

// ----------------------------------------------------------------------------
// Local Variables
// ----------------------------------------------------------------------------
const vec2 quad_tex_coords[4] = vec2[4](
	vec2(0.0, 1.0),
	vec2(0.0, 0.0),
	vec2(1.0, 1.0),
	vec2(1.0, 0.0)
);
const vec4 quad_offsets[4] = vec4[4](
	vec4(-0.5, +0.5, 0.0, 0.0),
	vec4(-0.5, -0.5, 0.0, 0.0),
	vec4(+0.5, +0.5, 0.0, 0.0),
	vec4(+0.5, -0.5, 0.0, 0.0)
);

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec3 position_ws;       // The position of the particle in word space.
	vec4 position_vs;		// The position of the particle in view space.
} in_data[1];

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
out VertexData
{
	vec2 tex_coord;
	vec3 position_ws;
} out_data;

// Data of the camera
layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 projection;		// Projection matrix
	mat4 projection_inv;	// Inverse of the projection matrix
	mat4 view;				// View matrix
	mat4 view_inv;			// Inverse of the view matrix
	mat3 view_it;			// Inverse of the transpose of the top-left part 3x3 of the view matrix
	vec3 eye_position;		// Position of the eye in world space
};

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	float particle_size_vs = 0.5;
	for (int i = 0; i < 4; i++)
	{
		out_data.tex_coord = quad_tex_coords[i];
		out_data.position_ws = in_data[0].position_ws;
		gl_Position = projection * in_data[0].position_vs + particle_size_vs * quad_offsets[i];
		
		EmitVertex();
	}
	EndPrimitive();
}
