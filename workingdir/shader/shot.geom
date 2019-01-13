#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (points) in;
layout (triangle_strip) out;
//layout (line_strip) out;
layout (max_vertices = 14) out;  

layout(location = 0) in mat4 v_m2vMat[];
layout(location = 4) in vec4 v_umbraColor_range[];

layout(location = 0) out vec4 g_eyepos;
layout(location = 1) out vec3 g_uvs;
layout(location = 2) out vec4 g_umbraColor_range;

layout (set=1, binding = 0) uniform cameraUniformBuffer {
	layout(offset = 0) mat4 v2sMatrix;
	layout(offset = 128) vec3 eyepos;
};

void main() {
	//center in view space
	vec4 center_pos = gl_in[0].gl_Position;
	
	mat4 inv_m2vMat = inverse(v_m2vMat[0]);
	
	//transform vectors
	//w is 0.0 because otherwise we would need to subtract the center which is "v_m2vMat[0] * vec4(0.0, 0.0, 0.0, 1.0)" setting w to 0.0 makes this unnecesary
	//calculate coordinates from model space to view space
	vec4 right = v_m2vMat[0][0];
	vec4 up = v_m2vMat[0][1];
	vec4 forward = v_m2vMat[0][2];
	
	right.y = -right.y;
	up.y = -up.y;
	forward.y = -forward.y;
	
	for(int i = 0; i < 14; i++) {
		//fancy bitshift magic
		int b = 1 << i;
		vec3 factor = vec3((0x287a & b) != 0, (0x02af & b) != 0, (0x31e3 & b) != 0);
		factor = (factor * 2.0) - 1.0;
		vec4 changevec = (right * factor.x) + (up * factor.y) + (forward * factor.z);
		
		//vertex-pos in view space
		vec4 va = center_pos + changevec;
		gl_Position = v2sMatrix * va;
		
		g_uvs = factor;
		
		g_eyepos = inv_m2vMat * vec4(0.0, 0.0, 0.0, 1.0);
		g_eyepos /= g_eyepos.w;
		
		g_umbraColor_range = v_umbraColor_range[0];
		
		EmitVertex();
	}
	EndPrimitive();
}