#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outLightAccumulation;

//input
layout(location = 0) in vec3 v_position;

layout (set=0, binding=0) uniform texture2D tex;
layout (set=0, binding=1) uniform sampler textureSampler;

layout(push_constant) uniform LodBlock {
	int lod;
};

void main() {
	outLightAccumulation = 
		max(
			texture(sampler2D(tex, textureSampler), v_position.xy), 
			vec4(1.0)
		) - vec4(1.0);
}