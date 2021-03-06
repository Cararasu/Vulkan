#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outLightAccumulation;

layout(location = 0) in vec4 g_eyepos;
layout(location = 1) in vec3 g_uvs;
layout(location = 2) in vec4 g_umbraColor;


void main() {
	vec3 n_eyepos = normalize(g_eyepos.xyz - g_uvs);
	vec3 n_uv = normalize(g_uvs);
	float l1 = length(g_uvs) * dot(n_eyepos, n_uv);
	
	vec3 distance_vec = g_uvs - (n_eyepos * l1);
	float distance = 1.0 - clamp(length(distance_vec), 0.0, 1.0);
	
	float curve = pow(distance, 2) * 2.0;
	outLightAccumulation = vec4(vec3(1.0, 1.0, 1.0) * curve + g_umbraColor.rgb, curve);
}
