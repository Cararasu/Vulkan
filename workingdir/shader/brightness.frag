#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outLightAccumulation;

//input
layout(location = 0) in vec3 v_position;

const mat4 screenspace_to_coords_mat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );
	
layout (set=0, binding=0) uniform sampler2D tex;

layout(push_constant) uniform LodBlock {
	int lod;
};

const float brightness_cutoff = 1.1;

void main() {
	outLightAccumulation = 
		max(
			texture(tex, (screenspace_to_coords_mat * vec4(v_position, 1.0)).xy), 
			vec4(brightness_cutoff)
		) - vec4(brightness_cutoff);
}