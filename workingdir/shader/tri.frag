#version 450
#extension GL_ARB_separate_shader_objects : enable

//input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_texCoord;
layout(location = 2) in vec3 v_normal;

layout (set=1, binding = 0) uniform lightVectorBuffer {
	vec3 lightvector;
};
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

const float kPi = 3.14159265;
const float kShininess = 16.0;

void main() {
	vec3 view_direction = normalize(v_position);//0 0 -1
	vec3 normal_direction = normalize(v_normal);//0 0 1
	vec3 light_direction = normalize(lightvector);//0 0 -1
	vec3 halfway_direction = normalize(light_direction + view_direction);
	
	//float lambertian = max(dot(light_direction, normal_direction), 0.0);
	//outColor = vec4(1.0, 0.0, 0.0, 1.0) * lambertian;
   
	float angle = max(dot(halfway_direction, normal_direction), 0.0);
	outColor = vec4(1.0, 0.0, 0.0, 1.0) * pow(angle, 16.0 * 4.0);
	//outColor = textureLod(sampler2DArray(diffuseTexture, sampler), vec3(v_texCoord.x, -1.0f * v_texCoord.y, float(diffuseTexId)), 0.0f);
}