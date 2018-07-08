#version 450
#extension GL_ARB_separate_shader_objects : enable

//per-vertex
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;

//output
layout(location = 0) out vec2 texCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = vec4(pos, 1.0f);
	texCoord = uv;
}