	#version 450
#extension GL_ARB_separate_shader_objects : enable

//per-vertex
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 uv;

//global-info
layout (set=0, binding = 0) uniform cameraUniformBuffer {
	layout(offset = 0) mat4 m2sMatrix;
};

//output
layout(location = 0) out vec3 v_texCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = m2sMatrix * vec4(pos, 1.0f);
	gl_Position.y = -gl_Position.y;
	v_texCoord = uv;
}