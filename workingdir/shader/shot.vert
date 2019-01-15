#version 450
#extension GL_ARB_separate_shader_objects : enable

//per-vertex
layout(location = 0) in vec3 vertex_pos;

//per-instance
layout(location = 1) in mat4 m2vMatrix;
layout(location = 5) in vec4 umbraColor_range;

layout(location = 0) out mat4 v_m2vMat;
layout(location = 4) out vec4 v_umbraColor_range;
//in view space
layout(location = 5) out vec4 v_light_position;

layout(location = 6) out vec3 right;
layout(location = 7) out vec3 up;
layout(location = 8) out vec3 forward;
layout(location = 9) out vec4 v_0[14];

layout (set=1, binding = 0) uniform cameraUniformBuffer {
	layout(offset = 0) mat4 v2sMatrix;
	layout(offset = 128) vec3 eyepos;
};

void main() {
	v_m2vMat = m2vMatrix;
	v_umbraColor_range = umbraColor_range;
    v_light_position = m2vMatrix * vec4(vertex_pos, 1.0);
	v_light_position /= v_light_position.w;
	v_light_position.y = -v_light_position.y;
	gl_Position = v_light_position;
	
	
	{
		//center in view space
		vec3 center_pos = gl_Position.xyz;
		
		mat4 inv_m2vMat = inverse(v_m2vMat);
		
		//transform vectors
		//w is 0.0 because otherwise we would need to subtract the center which is "v_m2vMat[0] * vec4(0.0, 0.0, 0.0, 1.0)" setting w to 0.0 makes this unnecesary
		//calculate coordinates from model space to view space
		
		right = normalize(v_m2vMat[0].xyz) * v_umbraColor_range.w;
		up = normalize(v_m2vMat[1].xyz) * v_umbraColor_range.w;
		forward = normalize(v_m2vMat[2].xyz) * v_umbraColor_range.w;
		
		right.y = -right.y;
		up.y = -up.y;
		forward.y = -forward.y;
		
		for(int i = 0; i < 7; i++) {
			//fancy bitshift magic
			int b = 1 << i;
			vec3 factor = vec3((0x287a & b) != 0, (0x02af & b) != 0, (0x31e3 & b) != 0);
			factor = (factor * 2.0) - 1.0;
			vec3 changevec = (right * factor.x) + (up * factor.y) + (forward * factor.z);
			
			//vertex-pos in view space
			v_0[2*i] = vec4(center_pos + changevec, 1.0);
			v_0[2*i + 1] = v2sMatrix * v_0[2*i];
			v_0[2*i + 1] /= v_0[2*i + 1].w;
		}
	}
	
	
}