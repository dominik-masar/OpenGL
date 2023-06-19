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
layout (binding = 0) uniform sampler2D accumulated_snow_old;


// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final output color.
layout (location = 0) out float accumulated_snow_new;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// The size of one texel when texture coordinates are in range (0,1).
	vec2 texel_size = 1.0 / textureSize(accumulated_snow_old, 0);		
	float original = texture(accumulated_snow_old, in_data.tex_coord).r;

	accumulated_snow_new = original + 0.0005;
	
	// We use Gaussian blur.
	//vec3 sum = vec3(0.f);
	//for (int x = 0; x < gauss5.length(); x++)
	//for (int y = 0; y < gauss5.length(); y++)
	//{
	//	// Offset from the center texel.
	//	vec2 offset = texel_size * (vec2(x, y) - vec2(gauss5.length()/2));
	//	// Adding weighted value.
	//	sum += gauss5[x] * gauss5[y] * textureLod(highlights_texture, in_data.tex_coord + offset, 0).rgb;
	//}
	//vec3 result = sum / gauss5x5_sum;
	//accumulated_snow_new = vec4(max(result, original), 1.f);
}