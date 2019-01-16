#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outLightAccumulation;

layout(location = 0) in vec4 v_uv_index_time;
layout(location = 1) in vec4 v_indices;
layout(location = 2) in vec2 v_blend_factors;

layout (set=2, binding=0) uniform texture2DArray explosionTextures;
layout (set=2, binding=1) uniform sampler textureSampler;


vec4 getColor(vec2 image_position, float factor) {
	return texture(
			sampler2DArray(explosionTextures, textureSampler),
			vec3((v_uv_index_time.xy + image_position) * 0.25, v_uv_index_time.z)
		) * factor;
}

void main() {
	
	outLightAccumulation = getColor(v_indices.xy, v_blend_factors.x) + getColor(v_indices.zw, v_blend_factors.y);
	
	outLightAccumulation.a = 1.0 - (max(abs(v_uv_index_time.w - 0.5) - 0.45, 0.0) * 20.0);
}
