#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outLightAccumulation;

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputDiffuse;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputNormal;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput inputSpecular;
layout (input_attachment_index = 3, set = 0, binding = 3) uniform subpassInput inputDepth;

//input
layout(location = 0) in vec3 v_position;

layout (set=1, binding = 0) uniform camera_struct {
	layout(offset = 0) mat4 v2sMatrix;
	layout(offset = 64) mat4 inv_v2sMatrix;
	layout(offset = 128) vec4 pos;
} camera;

layout (set=2, binding = 0) uniform lightVectorBuffer {
	vec4 direction_amb;
	vec4 color;
} light;

vec3 reconstruct_pos_from_depth() {
	float z = subpassLoad(inputDepth).x;
    float x = v_position.x;
    float y = v_position.y;
	vec4 pos = camera.inv_v2sMatrix * vec4(x, y, z, 1.0);
	return pos.xyz / pos.w;
}

void main() {
	vec3 pos = reconstruct_pos_from_depth();
	
	vec3 view_direction = normalize(-pos);
	vec2 normal_temp = subpassLoad(inputNormal).rg;
	vec3 normal_direction = vec3(normal_temp, sqrt(1 - normal_temp.x*normal_temp.x - normal_temp.y*normal_temp.y));
	vec3 light_direction = normalize(-light.direction_amb.xyz);
	
	float lambertian = max(dot(light_direction, normal_direction), 0.0);
	
	float specular = 0.0;
	if(lambertian > 0.0) {
		if(true) {//blinn
			vec3 halfway_direction = normalize(light_direction + view_direction);
			float angle = max(dot(halfway_direction, normal_direction), 0.0);
			specular = pow(angle, 64.0);
		} else {//phong
			vec3 reflect_direction = reflect(-light_direction, normal_direction);
			float angle = max(dot(reflect_direction, -view_direction), 0.0);
			specular = pow(angle, 16.0);
		}
	}
	vec4 diffuseColor = subpassLoad(inputDiffuse);
	diffuseColor = diffuseColor * (diffuseColor.w * 255);
   
	outLightAccumulation = vec4(diffuseColor.rgb * light.direction_amb.w + diffuseColor.rgb * lambertian + (vec3(1.0, 1.0, 1.0) * specular), 1.0);
}
