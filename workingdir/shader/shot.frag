#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outColor;

layout(location = 0) in vec4 g_eyepos;
layout(location = 1) in vec3 g_uvs;

layout (set=0, binding = 0) uniform cameraUniformBuffer {
	layout(offset = 0) mat4 v2sMatrix;
	layout(offset = 64) vec3 eyepos;
};


vec3 spikecolor = vec3(1.0, 1.0, 1.0);
vec3 umbracolor = vec3(1.0, 0.0, 0.0);

void main() {
	vec3 n_eyepos = normalize(g_eyepos.yxz - g_uvs);
	vec3 n_uv = normalize(g_uvs);
	float l1 = length(g_uvs) * dot(n_eyepos, n_uv);
	
	float distance = 1.0 - clamp(length(g_uvs - (n_eyepos * l1)), 0.0, 1.0);
	outColor = vec4(spikecolor * pow(distance, 4) + umbracolor * pow(distance, 0.25), distance);
}