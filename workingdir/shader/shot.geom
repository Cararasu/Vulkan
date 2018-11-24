#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 14) out;  

layout(location = 0) in vec4 v_up[];
layout(location = 1) in vec4 v_right[];
layout(location = 2) in vec4 v_forward[];
layout(location = 3) in mat4 v_m2wMat[];

layout(location = 0) out vec4 g_eyepos;
layout(location = 1) out vec3 g_uvs;

layout (set=0, binding = 0) uniform cameraUniformBuffer {
	layout(offset = 0) mat4 v2sMatrix;
	layout(offset = 64) vec3 eyepos;
};

void main() {
	vec4 screen_pos = gl_in[0].gl_Position;
	
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = vec3(1.0, 0.0, 0.0);
	vec3 forward = vec3(0.0, 0.0, 1.0);
	
	for(int i = 0; i < 14; i++) {
		int b = 1 << i;
		
		vec3 factor = vec3((0x287a & b) != 0, (0x02af & b) != 0, (0x31e3 & b) != 0);
		factor = (factor * 2.0) - 1.0;
		
		vec4 changevec = (v_up[0] * factor.x) + (v_right[0] * factor.y) + (v_forward[0] * factor.z);
		vec4 va = screen_pos + changevec;
		gl_Position = v2sMatrix * va;
		g_uvs = factor;
		g_eyepos = inverse(v_m2wMat[0]) * vec4(0.0, 0.0, 0.0, 1.0);
		g_eyepos /= g_eyepos.w;
		EmitVertex();
	}
	EndPrimitive();
}