#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;
layout(location = 2) out vec4 outSpecular;

//input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_texCoord;
layout(location = 2) in vec3 v_normal;

layout (set=1, binding=0) uniform texture2D diffuseTexture;
layout (set=1, binding=1) uniform sampler textureSampler;

void main() {
	vec3 normal_direction = normalize(v_normal);
	
	vec4 ambientcolor = texture(sampler2D(diffuseTexture, textureSampler), vec2(v_texCoord.x, -1.0f * v_texCoord.y));
   
	outColor = ambientcolor;
	outColor.w = 1/255.0;
	outNormal = normal_direction.xy;
	outSpecular = vec4(v_position, 1.0);//gl_FragCoord / vec4(500.0, 500.0, 1.0, 1.0);
}