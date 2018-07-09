#version 450
#extension GL_ARB_separate_shader_objects : enable

//per-vertex
layout(location = 0) in vec2 pos;

//per-instance
layout(location = 1) in vec4 dim;
layout(location = 2) in vec4 uvdim;
layout(location = 3) in vec2 depth_imgindex;

//per-frame
//layout (set=0, binding = 0) uniform cameraUniformBuffer {
//	vec4 viewport;
//};

//output
layout(location = 0) out vec2 texCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	
    gl_Position = vec4( (dim.xy + (pos.xy * dim.zw) - 0.5f) * 2.0f, depth_imgindex.x, 1.0f);
	texCoord = depth_imgindex;
}