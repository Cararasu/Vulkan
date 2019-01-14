#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outLightAccumulation;

//input
layout(location = 0) in vec3 v_position;

layout (set=0, binding=0) uniform texture2D bloom;
layout (set=0, binding=1) uniform sampler textureSampler;

layout(push_constant) uniform LodBlock {
	int lod;
};

void main() {
	vec4 color = vec4(0.0);
	for(int i = 0; i < 5; i++) {
		color += textureLod(sampler2D(bloom, textureSampler), v_position.xy, i);
	}
	outLightAccumulation = color;
}