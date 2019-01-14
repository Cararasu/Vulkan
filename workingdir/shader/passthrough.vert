#version 450
#extension GL_ARB_separate_shader_objects : enable

//per-vertex
layout(location = 0) in vec3 pos;

//output
layout(location = 0) out vec3 v_position;
//layout(location = 1) out vec3 v_viewvec;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	gl_Position = vec4((pos * vec3(2.0, 2.0, 1.0)) + vec3(-1.0, -1.0, 0.0), 1.0);
	v_position = vec3(pos.xy, -1.0);
}