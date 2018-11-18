#version 450
#extension GL_ARB_separate_shader_objects : enable

//input
layout(location = 0) in vec3 texCoord;

//texture-aggregates
//layout (set=1, binding=0) sampler sampler;
//layout (set=1, binding=1) texture2DArray diffuseTexture;

//per-material
//layout(push_constant) uniform Data {
//	uint matId;
//	uint diffuseTexId;
//	uint specularTexId;
//	uint normalTexId;
//};

//output
layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	//outColor = textureLod(sampler2DArray(diffuseTexture, sampler), vec3(texCoord.x, -1.0f * texCoord.y, float(diffuseTexId)), 0.0f);
}