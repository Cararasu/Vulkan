#version 450
#extension GL_ARB_separate_shader_objects : enable

in vec3 fragColor;
in vec3 texCoord;

layout(location = 0) out vec4 outColor;

layout (set=1, binding=0) uniform sampler sampler;
layout (set=1, binding=1) uniform texture2D diffuseTexture;


void main() {
	outColor = texture(sampler2D(diffuseTexture, sampler), texCoord.xy * vec2(1.0f, -1.0f));
	//outColor = vec4(texCoord, 0.0f);
}