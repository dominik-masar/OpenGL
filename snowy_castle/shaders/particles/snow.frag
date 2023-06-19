#version 430 core

// The UBO with camera data.	
layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 projection;	  // The projection matrix.
	mat4 projection_inv;  // The inverse of the projection matrix.
	mat4 view;			  // The view matrix
	mat4 view_inv;		  // The inverse of the view matrix.
	mat3 view_it;		  // The inverse of the transpose of the top-left part 3x3 of the view matrix
	vec3 eye_position;	  // The position of the eye in world space.
};

// The material data.
layout (std140, binding = 3) uniform PhongMaterialBuffer
{
    vec3 ambient;     // The ambient part of the material.
    vec3 diffuse;     // The diffuse part of the material.
    float alpha;      // The alpha (transparency) of the material.
    vec3 specular;    // The specular part of the material.
    float shininess;  // The shininess of the material.
} material;

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;
	vec3 position_ws;
} in_data;

layout (binding = 0) uniform sampler2D particle_texture;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
layout (location = 0) out vec4 final_color;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	float intensity = texture(particle_texture, in_data.tex_coord).r;
	final_color = vec4(1.0f, 1.0f, 1.0f, 0.2 * intensity);
}