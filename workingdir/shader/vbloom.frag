#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outLightAccumulation;

//input
layout(location = 0) in vec3 v_position;

layout (set=0, binding=0) uniform texture2D tex;
layout (set=0, binding=1) uniform sampler textureSampler;

layout(push_constant) uniform LodBlock {
	int lod;
};


float intensity = 1.0;
float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
	vec2 texsize = textureSize(sampler2D(tex, textureSampler), lod);
	vec2 texeldist = 1.0 / texsize;
	vec2 texelpos = gl_FragCoord.xy / texsize;
	texeldist.y = 0.0;
	
	vec3 value = textureLod(sampler2D(tex, textureSampler), texelpos, lod).rgb * intensity * weight[0];
	
	for (int i = 1; i < 5; i++) {
		value += textureLod(sampler2D(tex, textureSampler), texelpos - (texeldist * i), lod).rgb * intensity * weight[i];
		value += textureLod(sampler2D(tex, textureSampler), texelpos + (texeldist * i), lod).rgb * intensity * weight[i];
	}
	
	outLightAccumulation = vec4(value, 1.0);
}
/*
void main() {
	vec2 texsize = textureSize(sampler2D(tex, textureSampler), lod);
	vec2 texeldist = 1.25 / texsize;
	vec2 texelpos = gl_FragCoord.xy / texsize;
	texeldist.y = 0.0;
	
	vec4 val_mid = textureLod(sampler2D(tex, textureSampler), texelpos, lod);
	vec4 val_sub = textureLod(sampler2D(tex, textureSampler), texelpos - texeldist, lod);
	vec4 val_add = textureLod(sampler2D(tex, textureSampler), texelpos + texeldist, lod);
	outLightAccumulation = (6.0 * val_mid + 5.0 * val_sub + 5.0 * val_add) / 16.0;
}
*/