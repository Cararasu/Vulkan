#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;
layout(location = 2) out vec4 outSpecular;

//input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_texCoord;
layout(location = 2) in vec3 v_normal;

void main() {
	outColor = vec4(0.5, 0.5, 0.5, 1/255.0);
	outNormal = normalize(v_normal).xy;
	outSpecular = gl_FragCoord / vec4(500.0, 500.0, 1.0, 1.0);
}