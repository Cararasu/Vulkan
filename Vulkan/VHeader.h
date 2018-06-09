#pragma once

#define V_MAX_STAGINGBUFFER_SIZE (16*1024*1024)

#include <render/Header.h>
#include <vulkan/vulkan.hpp>
#include <inttypes.h>

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
struct InstanceObj {
	glm::mat4 m2wMatrix;
};
struct Camera {
	glm::mat4 w2sMatrix;
};
struct Material {
	
};
struct ObjectPartData{// the data
	u32 objectId;
	u32 diffuseTexId;
	u32 specularTexId;
	u32 normalTexId;
};

struct ImageWrapper;

extern std::set<vk::DeviceMemory> memories;

void copyData(const void* srcData, vk::DeviceMemory dstMemory, vk::DeviceSize offset, vk::DeviceSize size);
	
void copyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, vk::DeviceSize srcOffset, vk::Offset3D dstOffset, vk::Extent3D extent, u32 index,
	vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag,
	vk::CommandPool commandPool, vk::Queue submitQueue);

void generateMipmaps(ImageWrapper* image, u32 baseLevel, u32 arrayIndex, u32 generateLevels, vk::CommandPool commandPool, vk::Queue submitQueue);

bool hasStencilComponent(vk::Format format);

std::vector<vk::DescriptorSetLayout> createStandardDescriptorSetLayouts();
