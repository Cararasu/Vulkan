#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;
layout(location = 2) out vec4 outSpecular;

//input
layout(location = 0) in vec3 v_texCoord;

layout (set=0, binding=1) uniform sampler2DArray skyboxTextures;

vec4 colors[6] = {
	vec4(1.0, 0.0, 0.0, 1.0/51.0),//red
	vec4(1.0, 1.0, 0.0, 1.0/51.0),//yellow
	vec4(1.0, 0.0, 1.0, 1.0/51.0),//
	vec4(1.0, 1.0, 1.0, 1.0/51.0),
	vec4(0.0, 1.0, 0.0, 1.0/51.0),
	vec4(0.0, 1.0, 1.0, 1.0/51.0)
};

void main() {
	outColor.xyz = colors[int(v_texCoord.z + 0.5)].xyz;
	if(int(v_texCoord.z + 0.5) <= 5) {
		outColor = texture(skyboxTextures, v_texCoord);
	}
	outColor.w = 1.4/51.0;
	outNormal = vec2(0.0, 0.0);
	outSpecular = vec4(0.0, 0.0, 0.0, 0.0);
}