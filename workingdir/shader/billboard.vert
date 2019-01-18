#version 450
#extension GL_ARB_separate_shader_objects : enable

//per-vertex
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 uvs;
layout(location = 2) in vec3 normal;

//per-instance
layout(location = 3) in vec4 position;
layout(location = 4) in vec4 scale_index_time;

layout(location = 0) out vec4 v_uv_index_time;
layout(location = 1) out vec4 v_indices;
layout(location = 2) out vec2 v_blend_factors;

layout (set=1, binding = 0) uniform cameraUniformBuffer {
	layout(offset = 0) mat4 v2sMatrix;
};

void main() {
	vec4 adjusted_pos = position;
	adjusted_pos.xy += pos.xy * scale_index_time.xy;
    gl_Position = v2sMatrix * adjusted_pos;
	gl_Position.y = -gl_Position.y;
	
	v_uv_index_time = vec4(uvs.xy, scale_index_time.zw);
	
	float value = scale_index_time.w * 15.0;
	float flower = floor(value);
	float flarger = ceil(value);
	v_indices = vec4(mod(flower, 4.0), floor(flower / 4.0), mod(flarger, 4.0), floor(flarger / 4.0));
	
	v_blend_factors = vec2(flarger - value, value - flower);
}