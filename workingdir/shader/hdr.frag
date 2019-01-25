#version 450
#extension GL_ARB_separate_shader_objects : enable

//output
layout(location = 0) out vec4 outColor;

//input
layout(location = 0) in vec3 v_position;

layout (set=0, binding=0) uniform sampler2D lightAccumulation;
	
void main() {
    const float gamma = 1.0;
	
    vec3 hdrColor = texture(lightAccumulation, (v_position.xy + vec2(1.0, -1.0)) / vec2(2.0, -2.0)).rgb;
	
    // reinhard tone mapping
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    outColor = vec4(mapped, 1.0);
}


