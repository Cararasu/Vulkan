#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outLightAccumulation;

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputDiffuse;

//input
layout(location = 0) in vec3 v_position;

void main() {
	vec4 diffvalue = subpassLoad(inputDiffuse);
	outLightAccumulation = vec4(diffvalue.rgb * diffvalue.w * 255.0, 1.0);
}