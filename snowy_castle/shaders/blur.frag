#version 450 core

// The kernel for gaussian blur.
const float gauss5[5] = float[5](1.0, 4.0, 6.0, 4.0, 1.0);
const float gauss5x5_sum = 16.0 * 16.0;

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;  // The vertex texture coordinates.
} in_data;

// The texture to be blurred.
layout (binding = 0) uniform sampler2D snow_texture;


// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final output color.
layout (location = 0) out vec4 blured_snow;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	vec2 texel_size = 1.0 / textureSize(snow_texture, 0);
	vec3 original = texture(snow_texture, in_data.tex_coord).rgb;
	
	// We use Gaussian blur.
	vec3 sum = vec3(0.f);
	for (int x = 0; x < gauss5.length(); x++)
	for (int y = 0; y < gauss5.length(); y++)
	{
		// Offset from the center texel.
		vec2 offset = texel_size * (vec2(x, y) - vec2(gauss5.length()/2));
		// Adding weighted value.
		sum += gauss5[x] * gauss5[y] * textureLod(snow_texture, in_data.tex_coord + offset, 0).rgb;
	}
	vec3 result = sum / gauss5x5_sum;
	blured_snow = vec4(result, 1.f);
}