#version 450
#extension GL_ARB_separate_shader_objects : enable

//input
in vec3 texCoord;

//texture-aggregates
layout (set=1, binding=0) uniform sampler sampler;
layout (set=1, binding=1) uniform texture2DArray diffuseTexture;

//per-material
layout(push_constant) uniform Data {
	uint matId;
	uint diffuseTexId;
	uint specularTexId;
	uint normalTexId;
};

//output
layout(location = 0) out vec4 outColor;

void main() {
	outColor = textureLod(sampler2DArray(diffuseTexture, sampler), vec3(texCoord.x, -1.0f * texCoord.y, float(diffuseTexId)), 0.0f);
}