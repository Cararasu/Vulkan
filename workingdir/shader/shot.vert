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

void main() {
	v_m2vMat = m2vMatrix;
	v_umbraColor_range = umbraColor_range;
    v_light_position = m2vMatrix * vec4(vertex_pos, 1.0);
	v_light_position /= v_light_position.w;
	v_light_position.y = -v_light_position.y;
	gl_Position = v_light_position;
}