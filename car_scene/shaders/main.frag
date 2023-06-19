#version 450

// ----------------------------------------------------------------------------
// Structs
// ----------------------------------------------------------------------------

struct Light {
    vec4 position;
  
    vec4 ambient_color;
    vec4 diffuse_color;
    vec4 specular_color;

	vec4 direction;
    vec4 cut_off;
};

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------

layout(binding = 0, std140) uniform Camera {
    mat4 projection;
    mat4 view;
    vec3 position;
} camera;

layout(binding = 1, std430) buffer Lights {
	Light lights[];
};

layout(binding = 2, std140) uniform Object {
    mat4 model_matrix;

    vec4 ambient_color;
    vec4 diffuse_color;
    vec4 specular_color;
} object;

layout(binding = 3, std140) uniform Fog {
	vec4 color;
	float density;
} fog;

// The intpolated position from the vertex shader.
layout(location = 0) in vec3 fs_position;
// The intpolated normal from the vertex shader.
layout(location = 1) in vec3 fs_normal;
// The coordinates in texture from the vertex shader.
layout(location = 2) in vec2 fs_texture_coordinate;
// Fog factor for computing fog.
layout(location = 10) in float fog_factor;

// Texture variables
layout(location = 3) uniform bool has_texture = false;
layout(location = 4) uniform vec2 texture_scale;
layout(binding = 4) uniform sampler2D albedo_texture;

// Uniform bools
layout(location = 5) uniform bool toon_shading_on = false;
layout(location = 6) uniform bool light_on = true;
layout(location = 11) uniform bool fog_on = false;

// Other variables
layout(location = 7) uniform int blinking_light = 5;
layout(location = 8) uniform int toon_levels = 10;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final output color.
layout(location = 0) out vec4 final_color;

// ----------------------------------------------------------------------------
// Functions
// ----------------------------------------------------------------------------
vec3 calc_light(Light light, vec3 N, vec3 E);

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main() {
	vec3 color_sum = vec3(0.0);
    vec3 N = normalize(fs_normal);
	vec3 E = normalize(camera.position - fs_position); 
	
    // calculate all lights
    for(int i = 0; i < lights.length(); i++) {
        if (i != blinking_light + 1 || light_on) {
	    	color_sum += calc_light(lights[i], N, E);
        }
    }

    color_sum = color_sum / (color_sum + 1.0);   // tone mapping
    color_sum = pow(color_sum, vec3(1.0 / 2.2)); // gamma correction

    if (fog_on) {
        color_sum = mix(vec3(fog.color), color_sum, fog_factor);
    }

    if (toon_shading_on) {
        int level = toon_levels;
        final_color = vec4(round(color_sum * level) / level, 1.0);
    } else {
        final_color = vec4(color_sum, 1.0);
    }
}

vec3 calc_light(Light light, vec3 N, vec3 E) {
    vec3 light_vector = light.position.xyz - fs_position * light.position.w;
	vec3 L = normalize(light_vector);
	vec3 H = normalize(L + E);

	float NdotL = max(dot(N, L), 0.0);
	float NdotH = max(dot(N, H), 0.0001);

    float intensity = 1.0;
	if (light.direction.w > 0.5) {
        float theta = dot(L, normalize(-light.direction.xyz));
        float epsilon = light.cut_off.x - light.cut_off.y;
        intensity = clamp((theta - light.cut_off.y) / epsilon, 0.0, 1.0);
    }
	
	vec3 ambient = object.ambient_color.rgb * light.ambient_color.rgb * intensity *
                   ((has_texture) ? 
                   texture(albedo_texture, vec2(fs_texture_coordinate.x * texture_scale.x, fs_texture_coordinate.y * texture_scale.y)).rgb 
                   : vec3(1.0));
	vec3 diffuse = object.diffuse_color.rgb * light.diffuse_color.rgb * intensity *
                   ((has_texture) ? 
                   texture(albedo_texture, vec2(fs_texture_coordinate.x * texture_scale.x, fs_texture_coordinate.y * texture_scale.y)).rgb 
                   : vec3(1.0));
	vec3 specular = object.specular_color.rgb * light.specular_color.rgb * intensity;
	vec3 color = ambient.rgb
		+ NdotL * diffuse.rgb
		+ pow(NdotH, object.specular_color.w) * specular;
	color /= dot(light_vector, light_vector);

    return color;
}
