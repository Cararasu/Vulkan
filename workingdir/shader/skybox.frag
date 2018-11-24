#version 450
#extension GL_ARB_separate_shader_objects : enable

//input
layout(location = 0) in vec3 v_texCoord;

layout (set=0, binding=1) uniform texture2DArray skyboxTextures;
layout (set=0, binding=2) uniform sampler textureSampler;


//output
layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(sampler2DArray(skyboxTextures, textureSampler), vec3(v_texCoord.x, -1.0f * v_texCoord.y, v_texCoord.z));
}