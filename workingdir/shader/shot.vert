#version 450
#extension GL_ARB_separate_shader_objects : enable

//per-vertex
layout(location = 0) in vec3 vertex_pos;

//per-instance
layout(location = 1) in mat4 m2vMatrix;
layout(location = 5) in vec3 umbraColor;

layout(location = 0) out mat4 v_m2wMat;
layout(location = 4) out vec3 v_umbraColor;

void main() {
    gl_Position = m2vMatrix * vec4(vertex_pos, 1.0);
	
	gl_Position.y = -gl_Position.y;
	v_m2wMat = m2vMatrix;
	v_umbraColor = umbraColor;
}