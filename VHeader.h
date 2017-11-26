
#ifndef VHEADER_H
#define VHEADER_H

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
#include "DataWrapper.h"

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

vk::CommandPool createTransferCommandPool(vk::CommandPoolCreateFlags createFlags);
vk::CommandPool createGraphicsCommandPool(vk::CommandPoolCreateFlags createFlags);
void destroyCommandPool(vk::CommandPool commandPool);

vk::CommandBuffer createCommandBuffer(vk::CommandPool commandPool, vk::CommandBufferLevel bufferLevel);
void deleteCommandBuffer(vk::CommandPool commandPool, vk::CommandBuffer commandBuffer);

void copyData(const void* srcData, vk::DeviceMemory dstMemory, vk::DeviceSize offset, vk::DeviceSize size);
void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset, vk::DeviceSize size,
	vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag,
	vk::CommandPool commandPool, vk::Queue submitQueue);
void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset, vk::DeviceSize size,
	vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag,
	vk::CommandBuffer commandBuffer);
	
void copyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, vk::DeviceSize srcOffset, vk::Offset3D dstOffset, vk::Extent3D extent, uint32_t index,
	vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag,
	vk::CommandPool commandPool, vk::Queue submitQueue);

void generateMipmaps(ImageWrapper* image, uint32_t baseLevel, uint32_t arrayIndex, uint32_t generateLevels, vk::CommandPool commandPool, vk::Queue submitQueue);
	
void transferData(const void* srcData, vk::Buffer targetBuffer, vk::DeviceSize offset, vk::DeviceSize size, vk::PipelineStageFlags usePipelineFlags, vk::AccessFlags useFlag,
	vk::CommandPool commandPool, vk::Queue submitQueue);
void transferData(const void* srcData, vk::Buffer targetBuffer, vk::DeviceSize offset, vk::DeviceSize size, vk::PipelineStageFlags usePipelineFlags, vk::AccessFlags useFlag, vk::CommandBuffer commandBuffer);
void transferData(const void* srcData, vk::Image targetImage, vk::Offset3D offset, vk::Extent3D extent, uint32_t index, vk::DeviceSize size, vk::PipelineStageFlags usePipelineFlags, vk::AccessFlags useFlag,
	vk::CommandPool commandPool, vk::Queue submitQueue);

vk::Format findDepthFormat();
vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
bool hasStencilComponent(vk::Format format);


uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended, vk::Buffer* buffer, vk::DeviceMemory* bufferMemory);
void destroyBuffer(vk::Buffer buffer, vk::DeviceMemory bufferMemory);


vk::DeviceMemory allocateMemory(vk::MemoryRequirements memoryRequirement, vk::MemoryPropertyFlags properties);

vk::ImageView createImageView2D(vk::Image image, uint32_t mipBase, uint32_t mipOffset, vk::Format format, vk::ImageAspectFlags aspectFlags);
vk::ImageView createImageView2DArray(vk::Image images, uint32_t mipBase, uint32_t mipOffset, uint32_t arrayOffset, uint32_t arraySize, vk::Format format, vk::ImageAspectFlags aspectFlags);

void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask,
	vk::CommandPool commandPool, vk::Queue submitQueue);

vk::Pipeline createStandardPipeline(vk::Extent2D viewport, vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass);
void destroyPipeline(vk::Pipeline pipeline);

vk::PipelineLayout createStandardPipelineLayout(std::vector<vk::DescriptorSetLayout>* descriptorSetLayouts = nullptr, std::vector<vk::PushConstantRange>* pushConstRanges = nullptr);
void destroyPipelineLayout(vk::PipelineLayout pipelineLayout);

vk::RenderPass createStandardRenderPass(vk::Format format);
void destroyRenderPass(vk::RenderPass renderpass);

std::vector<vk::DescriptorSetLayout> createStandardDescriptorSetLayouts();

vk::DescriptorPool createStandardDescriptorSetPool();
std::vector<vk::DescriptorSet> createDescriptorSets(vk::DescriptorPool descriptorSetPool, std::vector<vk::DescriptorSetLayout>* descriptorSetLayouts);

vk::ShaderModule loadShaderFromFile(const char* filename);

vk::Semaphore createSemaphore();
void destroySemaphore(vk::Semaphore semaphore);


#endif