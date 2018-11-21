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

//output
layout(location = 0) out vec3 v_position;
layout(location = 1) out vec3 v_texCoord;
layout(location = 2) out vec3 v_normal;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	vec4 t_position = m2vMatrix * vec4(pos, 1.0f);
    gl_Position = v2sMatrix * t_position;
	gl_Position.y *= 1.0;
	v_position = t_position.xyz / t_position.z;
	v_texCoord = uv;
	v_normal = vec3(normalMatrix * vec4(normal, 0.0));
}