#version 450
#extension GL_ARB_separate_shader_objects : enable

//per-vertex
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;
layout(location = 2) in vec3 uv;
layout(location = 3) in vec3 normal;

//per-instance
layout(location = 4) in mat4 m2wMatrix;

out vec3 texCoord;
out vec3 fragColor;

layout (set=0, binding = 0) uniform cameraUniformBuffer {
	mat4 w2sMatrix;
};

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = w2sMatrix * m2wMatrix * vec4(pos, 1.0f);
	texCoord = uv;
	fragColor = normal;
}