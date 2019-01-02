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

layout (set=1, binding = 0) uniform lightVectorBuffer {
	vec4 direction_amb;
	vec4 color;
} light;

layout (set=2, binding=0) uniform texture2D diffuseTexture;
layout (set=2, binding=1) uniform sampler textureSampler;

void main() {
	vec3 view_direction = normalize(-v_position);
	vec3 normal_direction = normalize(v_normal);
	vec3 light_direction = normalize(-light.direction_amb.xyz);
	
	float lambertian = max(dot(light_direction, normal_direction), 0.0);
	
	float specular = 0.0;
	if(lambertian > 0.0) {
		if(true) {//blinn
			vec3 halfway_direction = normalize(light_direction + view_direction);
			float angle = max(dot(halfway_direction, normal_direction), 0.0);
			specular = pow(angle, 32.0);
		} else {//phong
			vec3 reflect_direction = reflect(-light_direction, normal_direction);
			float angle = max(dot(reflect_direction, -view_direction), 0.0);
			specular = pow(angle, 8.0);
		}
	}
	vec4 ambientcolor = texture(sampler2D(diffuseTexture, textureSampler), vec2(v_texCoord.x, -1.0f * v_texCoord.y));
   
	outColor = ambientcolor * light.direction_amb.w + ambientcolor * lambertian + (vec4(1.0, 1.0, 1.0, 1.0) * specular);
	outNormal = normal_direction.xy;
	outSpecular = vec4(v_position, 0.0);
}