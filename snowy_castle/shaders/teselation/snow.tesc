#version 450 core

layout (vertices = 3) out;

in VertexData
{
	vec3 position_ws;	  // The vertex position in world space.
	vec3 normal_ws;		  // The vertex normal in world space.
	vec2 tex_coord;		  // The vertex texture coordinates.
} in_data[];

out VertexData
{
	vec3 position_ws;	  // The vertex position in world space.
	vec3 normal_ws;		  // The vertex normal in world space.
	vec2 tex_coord;		  // The vertex texture coordinates.
} out_data[];

uniform float tessLevelInner = 128.0;
uniform float tessLevelOuter = 128.0;

void main()
{
    out_data[gl_InvocationID].position_ws = in_data[gl_InvocationID].position_ws;
    out_data[gl_InvocationID].normal_ws = in_data[gl_InvocationID].normal_ws;
    out_data[gl_InvocationID].tex_coord = in_data[gl_InvocationID].tex_coord;

    gl_TessLevelOuter[0] = tessLevelOuter;
    gl_TessLevelOuter[1] = tessLevelOuter;
    gl_TessLevelOuter[2] = tessLevelOuter;

    gl_TessLevelInner[0] = tessLevelInner;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}