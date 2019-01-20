#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outLightAccumulation;

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_lightpos;
layout(location = 2) in vec4 v_umbraColor_range;

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputDiffuse;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputNormal;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput inputSpecular;
layout (input_attachment_index = 3, set = 0, binding = 3) uniform subpassInput inputDepth;

layout (set=1, binding = 0) uniform cameraUniformBuffer {
	layout(offset = 0) mat4 v2sMatrix;
	layout(offset = 64) mat4 inv_v2sMatrix;
} camera;

vec3 reconstruct_pos_from_depth() {
	vec4 screenpos = vec4(v_position.x, v_position.y, subpassLoad(inputDepth).x, 1.0);
	vec4 view_pos = camera.inv_v2sMatrix * screenpos;
	return view_pos.xyz / view_pos.w;
}

void main() {
	vec3 pos = reconstruct_pos_from_depth();
	
	vec3 view_direction = normalize(-pos);
	vec2 normal_temp = subpassLoad(inputNormal).rg;
	vec3 normal_direction = vec3(normal_temp, sqrt(1 - normal_temp.x*normal_temp.x - normal_temp.y*normal_temp.y));
	vec3 lightvec = v_lightpos - pos;
	vec3 light_direction = normalize(lightvec);
	
	float distance = length(lightvec);
	
	float intensity = 1.0 - (min(distance, v_umbraColor_range.w) / v_umbraColor_range.w);
	intensity *= intensity;
	
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
	
	vec3 reflectcolor = diffuseColor.rgb * v_umbraColor_range.rgb * (lambertian);
   
	outLightAccumulation = vec4(reflectcolor * (intensity * specular * 1.5 + intensity), 1.0);
}