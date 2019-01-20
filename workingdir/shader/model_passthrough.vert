#version 450
#extension GL_ARB_separate_shader_objects : enable

//per-vertex
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 uv;
layout(location = 2) in vec3 normal;

//per-instance
layout(location = 3) in mat4 m2vMatrix;
layout(location = 7) in mat4 normalMatrix;

//global-info
layout (set=0, binding = 0) uniform cameraUniformBuffer {
	layout(offset = 0) mat4 v2sMatrix;
};

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = v2sMatrix * m2vMatrix * vec4(pos, 1.0f);
	gl_Position.y = -gl_Position.y;
}