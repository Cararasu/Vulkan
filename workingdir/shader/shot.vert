#version 450
#extension GL_ARB_separate_shader_objects : enable

//per-vertex
layout(location = 0) in vec3 vertex_pos;

//per-instance
layout(location = 1) in mat4 m2vMatrix;

layout(location = 0) out vec4 v_up;
layout(location = 1) out vec4 v_right;
layout(location = 2) out vec4 v_forward;
layout(location = 3) out mat4 v_m2wMat;

void main() {
    gl_Position = m2vMatrix * vec4(vertex_pos, 1.0);
	v_up = m2vMatrix * vec4(0.0, 1.0, 0.0, 0.0);
	v_right = m2vMatrix * vec4(1.0, 0.0, 0.0, 0.0);
	v_forward = m2vMatrix * vec4(0.0, 0.0, 1.0, 0.0);
	
	gl_Position.y = -gl_Position.y;
	v_up.y = -v_up.y;
	v_right.y = -v_right.y;
	v_forward.y = -v_forward.y;
	v_m2wMat = m2vMatrix;
}