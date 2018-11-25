#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (points) in;
layout (triangle_strip) out;
//layout (line_strip) out;
layout (max_vertices = 14) out;  

layout(location = 0) in mat4 v_m2wMat[];
layout(location = 4) in vec3 v_umbraColor[];
layout(location = 5) in vec3 v_spikeColor[];

layout(location = 0) out vec4 g_eyepos;
layout(location = 1) out vec3 g_uvs;
layout(location = 2) out vec3 g_umbraColor;
layout(location = 3) out vec3 g_spikeColor;

layout (set=0, binding = 0) uniform cameraUniformBuffer {
	layout(offset = 0) mat4 v2sMatrix;
	layout(offset = 64) vec3 eyepos;
};

void main() {
	vec4 screen_pos = gl_in[0].gl_Position;
	
	//transform vectors
	//w is 0.0 because otherwise we would need to subtract the center which is "v_m2wMat[0] * vec4(0.0, 0.0, 0.0, 1.0)" setting w to 0.0 makes this unnecesary
	vec4 right = v_m2wMat[0][0];
	vec4 up = v_m2wMat[0][1];
	vec4 forward = v_m2wMat[0][2];
	
	right.y = -right.y;
	up.y = -up.y;
	forward.y = -forward.y;
	
	for(int i = 0; i < 14; i++) {
		//fancy bitshift magic
		int b = 1 << i;
		vec3 factor = vec3((0x287a & b) != 0, (0x02af & b) != 0, (0x31e3 & b) != 0);
		factor = (factor * 2.0) - 1.0;
		//I have no idea why "up" needs to be multiplied by the x-factor and "right" with the y-factor but it works that way ^^
		vec4 changevec = (up * factor.x) + (right * factor.y) + (forward * factor.z);
		vec4 va = screen_pos + changevec;
		gl_Position = v2sMatrix * va;
		
		g_uvs = factor;
		
		g_eyepos = inverse(v_m2wMat[0]) * vec4(0.0, 0.0, 0.0, 1.0);
		g_eyepos /= g_eyepos.w;
		
		g_umbraColor = v_umbraColor[0];
		g_spikeColor = v_spikeColor[0];
		
		EmitVertex();
	}
	EndPrimitive();
}