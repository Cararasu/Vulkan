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

layout (set=1, binding = 0) uniform cameraUniformBuffer {
	layout(offset = 64) mat4 inv_v2sMatrix;
} camera;

layout (set=2, binding = 0) uniform lightVectorBuffer {
	vec4 direction_amb;
	vec4 color;
} light;

layout (set=3, binding = 0) uniform shadowMapBuffer {
	mat4 v2ls_mat[3];
	vec4 drawrange[3];
} shadowMap;

layout (set=3, binding = 1) uniform texture2DArray shadowTexture;
layout (set=3, binding = 2) uniform sampler sampl; 

const mat4 screenspace_to_coords_mat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );
	
vec3 reconstruct_pos_from_depth() {
	vec4 screenpos = vec4(v_position.x, v_position.y, subpassLoad(inputDepth).x, 1.0);
	vec4 view_pos = camera.inv_v2sMatrix * screenpos;
	return view_pos.xyz / view_pos.w;
}

	
void main() {
	//this is in view space
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
	vec4 diffuseInput = subpassLoad(inputDiffuse);
	vec3 diffuseColor = diffuseInput.rgb * (diffuseInput.w * 51.0);
	
	
	outLightAccumulation = vec4(0.0, 0.0, 0.0, 1.0);
	float depth = -pos.z;
	float inlight = 1.0;
	for(int i = 0; i < 3; i++) {
		if(depth >= shadowMap.drawrange[i].x && depth < shadowMap.drawrange[i].y) {
			vec4 pos_in_shadowmap = shadowMap.v2ls_mat[i] * vec4(pos, 1.0f);//from view space directly into light-screen space
			pos_in_shadowmap.y = -pos_in_shadowmap.y;//flip y because reasons
			pos_in_shadowmap = screenspace_to_coords_mat * pos_in_shadowmap;//screen space -> coordinates of texture
			pos_in_shadowmap /= pos_in_shadowmap.w;
			inlight = texture( sampler2DArrayShadow(shadowTexture, sampl), vec4(pos_in_shadowmap.x, pos_in_shadowmap.y, float(i), pos_in_shadowmap.z) );
			//outLightAccumulation = pos_in_shadowmap;
			//return;
			break;
		}
	}
	outLightAccumulation = vec4(diffuseColor * (((lambertian + specular) * inlight) + light.direction_amb.w), 1.0);
}


