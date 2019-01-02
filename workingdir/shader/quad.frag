#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;
layout(location = 2) out vec4 outSpecular;

//input
layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec4 color;

//per-material


void main() {
	outColor = color;
	outNormal = vec2(0.0, 0.0);
	outSpecular = vec4(0.0, 0.0, 0.0, 0.0);
}