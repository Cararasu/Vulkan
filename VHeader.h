#pragma once

#define V_MAX_STAGINGBUFFER_SIZE (16*1024*1024)

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtx/normal.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include <vulkan/vulkan.hpp>
#include <set>
#include <vector>
#include <stdio.h>

void printError(VkResult res);


#define VCHECKCALL(call, errorcode) {\
	if(VkResult res = call){\
		errorcode;\
		printError(res);\
	}\
}

#define V_CHECKCALL(call, errorcode) {\
	vk::Result res = call;\
	if(res != vk::Result::eSuccess){\
		printf("Error %s\n", vk::to_string(res).c_str());\
		errorcode;\
	}\
}
#ifdef VULKAN_HPP_DISABLE_ENHANCED_MODE
#define V_CHECKCALL_MAYBE(call, errorcode) {\
	vk::Result res = call;\
	if(res != vk::Result::eSuccess){\
		printf("Error %s\n", vk::to_string(res).c_str());\
		errorcode;\
	}\
}
#else
#define V_CHECKCALL_MAYBE(call, errorcode) {\
	call;\
}
#endif


struct Vertex {
	glm::vec3 pos;
	glm::vec3 uv;
	glm::vec3 normal;
};
struct Instance {
	glm::mat4 m2wMatrix;
};
struct Camera {
	glm::mat4 w2sMatrix;
};
struct Material {
	
};
struct ObjectPartData{// the data
	uint32_t objectId;
	uint32_t diffuseTexId;
	uint32_t specularTexId;
	uint32_t normalTexId;
};

struct ImageWrapper;

extern std::set<vk::DeviceMemory> memories;

void copyData(const void* srcData, vk::DeviceMemory dstMemory, vk::DeviceSize offset, vk::DeviceSize size);
	
void copyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, vk::DeviceSize srcOffset, vk::Offset3D dstOffset, vk::Extent3D extent, uint32_t index,
	vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag,
	vk::CommandPool commandPool, vk::Queue submitQueue);

void generateMipmaps(ImageWrapper* image, uint32_t baseLevel, uint32_t arrayIndex, uint32_t generateLevels, vk::CommandPool commandPool, vk::Queue submitQueue);

bool hasStencilComponent(vk::Format format);

std::vector<vk::DescriptorSetLayout> createStandardDescriptorSetLayouts();
