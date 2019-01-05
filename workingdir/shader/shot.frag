#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;
layout(location = 2) out vec4 outSpecular;

layout(location = 0) in vec4 g_eyepos;
layout(location = 1) in vec3 g_uvs;
layout(location = 2) in vec3 g_umbraColor;
layout(location = 3) in vec3 g_spikeColor;

layout (set=0, binding = 0) uniform cameraUniformBuffer {
	layout(offset = 0) mat4 v2sMatrix;
	layout(offset = 64) vec3 eyepos;
};

void main() {
	vec3 n_eyepos = normalize(g_eyepos.xyz - g_uvs);
	vec3 n_uv = normalize(g_uvs);
	float l1 = length(g_uvs) * dot(n_eyepos, n_uv);
	
	vec3 distance_vec = g_uvs - (n_eyepos * l1);
	float distance = 1.0 - clamp(length(distance_vec), 0.0, 1.0);
	if(distance < 0.5) discard;
	
	/*float curve = pow(distance, 2) * 2.0;
	outColor = vec4(vec3(1.0, 1.0, 1.0) * curve + g_umbraColor, curve);
	outNormal = vec2(0.0, 0.0);
	outSpecular = vec4(0.0, 0.0, 0.0, 0.0);*/
	
	outColor = vec4(g_umbraColor, 100.0/255.0);
	outNormal = vec2(0.0, 0.0);
	outSpecular = vec4(0.0, 0.0, 0.0, 0.0);
}