#version 450
#extension GL_ARB_separate_shader_objects : enable

//input
layout(location = 0) in vec2 texCoord;

//per-material

//output
layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(0.0, 0.0, texCoord.y, 0.5);
}