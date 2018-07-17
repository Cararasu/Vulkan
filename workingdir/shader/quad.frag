#version 450
#extension GL_ARB_separate_shader_objects : enable

//input
layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec4 color;

//per-material

//output
layout(location = 0) out vec4 outColor;

void main() {
	outColor = color;
}