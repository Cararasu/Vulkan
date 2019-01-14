#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (points) in;
layout (triangle_strip) out;
//layout (line_strip) out;
layout (max_vertices = 14) out;  

layout(location = 0) in mat4 v_m2vMat[];
layout(location = 4) in vec4 v_umbraColor_range[];
layout(location = 5) in vec4 v_light_position[];

layout(location = 0) out vec3 g_position;
layout(location = 1) out vec3 g_lightpos;
layout(location = 2) out vec4 g_umbraColor_range;

layout (set=1, binding = 0) uniform cameraUniformBuffer {
	layout(offset = 0) mat4 v2sMatrix;
	layout(offset = 128) vec3 eyepos;
};

void main() {
	//center in view space
	vec3 center_pos = gl_in[0].gl_Position.xyz;
	
	mat4 inv_m2vMat = inverse(v_m2vMat[0]);
	
	//transform vectors
	//w is 0.0 because otherwise we would need to subtract the center which is "v_m2vMat[0] * vec4(0.0, 0.0, 0.0, 1.0)" setting w to 0.0 makes this unnecesary
	vec3 right = normalize(v_m2vMat[0][0].xyz) * v_umbraColor_range[0].w;
	vec3 up = normalize(v_m2vMat[0][1].xyz) * v_umbraColor_range[0].w;
	vec3 forward = normalize(v_m2vMat[0][2].xyz) * v_umbraColor_range[0].w;
	
	right.y = -right.y;
	up.y = -up.y;
	forward.y = -forward.y;
	
	g_lightpos = v_light_position[0].xyz * vec3(1.0, -1.0, 1.0);
	g_umbraColor_range = v_umbraColor_range[0];
	
	for(int i = 0; i < 14; i++) {
		//fancy bitshift magic
		int b = 1 << i;
		vec3 factor = vec3((0x287a & b) != 0, (0x02af & b) != 0, (0x31e3 & b) != 0);
		factor = (factor * 2.0) - 1.0;
		
		vec3 changevec = (right * factor.x) + (up * factor.y) + (forward * factor.z);
		
		//vertex-pos in view space
		vec3 va = center_pos + changevec;
		//and in screen space
		gl_Position = v2sMatrix * vec4(va, 1.0);
		gl_Position /= gl_Position.w;
		
		g_position = gl_Position.xyz;
		
		EmitVertex();
	}
	EndPrimitive();
}