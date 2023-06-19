#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
layout (location = 0) in vec4 position;

// The elapsed time in seconds.
uniform float elapsed_time_s;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
out VertexData
{
	vec3 position_ws;       // The position of the particle in word space.
	vec4 position_vs;		// The position of the particle in view space.
} out_data;

// The UBO with camera data.	
layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 projection;		// The projection matrix.
	mat4 projection_inv;	// The inverse of the projection matrix.
	mat4 view;				// The view matrix
	mat4 view_inv;			// The inverse of the view matrix.
	mat3 view_it;			// The inverse of the transpose of the top-left part 3x3 of the view matrix
	vec3 eye_position;		// The position of the eye in world space.
};

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	vec4 p = position;
	p.y = mod(p.y - elapsed_time_s * 3.0, 50.0);
	out_data.position_vs = view * p;
}
