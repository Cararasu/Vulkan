#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;
layout(location = 2) out vec4 outSpecular;

//input
layout(location = 0) in vec3 v_texCoord;

layout (set=0, binding=1) uniform texture2DArray skyboxTextures;
layout (set=0, binding=2) uniform sampler textureSampler;


void main() {
	outColor = texture(sampler2DArray(skyboxTextures, textureSampler), vec3(v_texCoord.x, v_texCoord.y, v_texCoord.z));
	outNormal = vec2(0.0, 0.0);
	outSpecular = vec4(0.0, 0.0, 0.0, 0.0);
}