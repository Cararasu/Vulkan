#version 450
#extension GL_ARB_separate_shader_objects : enable

//per-vertex
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 uvs;
layout(location = 2) in vec3 normal;

//per-instance
layout(location = 3) in mat4 m2vMatrix;
layout(location = 7) in vec4 umbraColor_range;

layout(location = 0) out vec4 v_eyepos;
layout(location = 1) out vec3 v_uvs;
layout(location = 2) out vec4 v_umbraColor_range;

layout (set=1, binding = 0) uniform cameraUniformBuffer {
	layout(offset = 0) mat4 v2sMatrix;
};

void main() {
    gl_Position = v2sMatrix * m2vMatrix * vec4(pos, 1.0);
	gl_Position.y = -gl_Position.y;
	
	v_uvs = pos;
	
	v_eyepos = inverse(m2vMatrix) * vec4(0.0, 0.0, 0.0, 1.0);
	v_eyepos /= v_eyepos.w;
	
	v_umbraColor_range = umbraColor_range;
}