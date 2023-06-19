#version 450 core

layout (triangles, equal_spacing, cw) in;

in VertexData
{
	vec3 position_ws;	  // The vertex position in world space.
	vec3 normal_ws;		  // The vertex normal in world space.
	vec2 tex_coord;		  // The vertex texture coordinates.
} in_data[];

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

out VertexData
{
	vec3 position_ws;	  // The vertex position in world space.
	vec3 normal_ws;		  // The vertex normal in world space.
	vec2 tex_coord;		  // The vertex texture coordinates.
} out_data;

layout (binding = 1) uniform sampler2D ortho_depth_tex;
layout (binding = 2) uniform sampler2D accumulated_snow_tex;
layout (binding = 3) uniform sampler2D height_tex;
uniform mat4 snow_matrix;

void main()
{
    out_data.position_ws = gl_TessCoord.x * in_data[0].position_ws + gl_TessCoord.y * in_data[1].position_ws + gl_TessCoord.z * in_data[2].position_ws;
    out_data.normal_ws = gl_TessCoord.x * in_data[0].normal_ws + gl_TessCoord.y * in_data[1].normal_ws + gl_TessCoord.z * in_data[2].normal_ws;
    out_data.tex_coord = gl_TessCoord.x * in_data[0].tex_coord + gl_TessCoord.y * in_data[1].tex_coord + gl_TessCoord.z * in_data[2].tex_coord;
	
	// Displacement
	vec4 snow_tex_coord = snow_matrix * vec4(out_data.position_ws, 1.0);
	float snow_factor = 0.0;

	float dist_in_tex = textureProj(ortho_depth_tex, snow_tex_coord).x;

	if (!(dist_in_tex + 0.00005 < snow_tex_coord.z / snow_tex_coord.w)) {
		snow_factor = textureProj(accumulated_snow_tex, snow_tex_coord).r;
	}

	float height = textureLod(height_tex, out_data.tex_coord, 0).x * snow_factor * 0.6;

	out_data.position_ws.y += height;

	vec4 height_cs = projection * view * vec4(0.0, height, 0.0, 0.0);
	vec4 pos = gl_TessCoord.x * gl_in[0].gl_Position + gl_TessCoord.y * gl_in[1].gl_Position + gl_TessCoord.z * gl_in[2].gl_Position;
	pos += height_cs;

	gl_Position = pos;
}