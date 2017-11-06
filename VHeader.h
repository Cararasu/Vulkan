
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


struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
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
	uint32_t matId;
	uint32_t diffuseTexId;
	uint32_t specularTexId;
	uint32_t normalTexId;
};
struct ObjectPart{// one part of the object
	ObjectPartData data;
	uint32_t indexOffset;
	uint32_t indexCount;
	int32_t vertexOffset;
};
struct Object {// one complete object
	std::vector<ObjectPart> parts;
	std::vector<Instance> instances;
};

struct ObjectStorage {
	std::vector<Object> objects;
	std::vector<Material> materials;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	//textureview
};
	
struct PipelineInfos{
	VkPipeline pipeline;
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	std::vector<VkDescriptorSet> descriptorSets;
};

extern std::set<VkDeviceMemory> memories;

extern VkCommandPool singleTransferCommandPool;
extern VkCommandPool singleImageTransitionCommandPool;
extern VkCommandBuffer singleTransferCommandBuffer;
extern VkCommandBuffer singleImageTransitionBuffer;

VkCommandPool createCommandPool(uint32_t queueId, VkCommandPoolCreateFlags createFlags);
void destroyCommandPool(VkCommandPool commandPool);

VkCommandBuffer createCommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel bufferLevel);
void deleteCommandBuffer(VkCommandPool commandPool, VkCommandBuffer commandBuffer);

void copyData(const void* srcData, VkDeviceMemory dstMemory, VkDeviceSize offset, VkDeviceSize size);
void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size,
	VkAccessFlags inputAccessFlag, VkAccessFlags outputAccessFlag);

void transferData(const void* srcData, VkBuffer targetBuffer, VkDeviceSize offset, VkDeviceSize size, VkAccessFlags useFlag);
void transferData(const void* srcData, VkImage targetimage, VkExtent3D offset, VkExtent3D size);

VkFormat findDepthFormat();
VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
bool hasStencilComponent(VkFormat format);


uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
void destroyBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory);


VkDeviceMemory allocateMemory(VkMemoryRequirements memoryRequirement, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended = 0);
VkDeviceMemory allocateImageMemory(VkImage image, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended = 0);
VkDeviceMemory allocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended = 0);

VkImageView createImageView2D(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

void createImage(VkExtent3D size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended,  VkImage* image, VkDeviceMemory* imageMemory);
void destroyImage(VkImage image, VkDeviceMemory imageMemory);
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask);

VkPipeline createStandardPipeline(VkExtent2D viewport, VkPipelineLayout pipelineLayout, VkRenderPass renderPass);
void destroyStandardPipeline();
void destroyPipeline(VkPipeline pipeline);

VkPipelineLayout createStandardPipelineLayout(std::vector<VkDescriptorSetLayout>* descriptorSetLayouts = nullptr, std::vector<VkPushConstantRange>* pushConstRanges = nullptr);
void destroyStandardPipelineLayout();
void destroyPipelineLayout(VkPipelineLayout pipelineLayout);

VkRenderPass createStandardRenderPass(VkFormat format);
void destroyStandardRenderPass();
void destroyRenderPass(VkRenderPass renderpass);

std::vector<VkDescriptorSetLayout> createStandardDescriptorSetLayouts();

VkDescriptorPool createStandardDescriptorSetPool();
void createStandardDescriptorSet(VkDescriptorPool descriptorSetPool, std::vector<VkDescriptorSetLayout>* descriptorSetLayouts, std::vector<VkDescriptorSet>* descriptorSets);

VkShaderModule loadShaderFromFile(const char* filename);

VkSemaphore createSemaphore(VkDevice device);
void destroySemaphore(VkDevice device, VkSemaphore semaphore);


#endif