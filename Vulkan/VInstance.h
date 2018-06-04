#ifndef VINSTANCE_H
#define VINSTANCE_H

#include "VDevice.h"
#include "VGlobal.h"
#include "PipelineModule.h"

struct VInstance {
	VExtLayerStruct extLayers;
	vk::PhysicalDevice physicalDevice;
	vk::Device device;
	u32 deviceId;
	size_t pgcIndex = 0;
	std::vector<VPGCQueue*> pgcQueues;
	VTQueue* tqueue = nullptr;
	
	u32 compQId = -1, graphQId = -1, presentQId = -1, transfQId = -1;
	
	struct{
		StandardPipelineModuleBuilder standard;
	}pipeline_module_builders;
	struct{
		PipelineModuleLayout standard;
	}pipeline_module_layouts;
	
	VInstance(vk::PhysicalDevice physicalDevice, u32 deviceId);
	~VInstance();

	VTQueue* requestTransferQueue(){
		return tqueue;
	}
	VPGCQueue* requestPGCQueue(){//TODO synch
		VPGCQueue* queue = pgcQueues[pgcIndex];
		pgcIndex = (pgcIndex + 1) % pgcQueues.size();
		return queue;
	}
	VPGCQueue* getPGCQueue(){//TODO synch
		return pgcQueues[pgcIndex];
	}
	
	void createBuffer ( vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended, vk::Buffer* buffer, vk::DeviceMemory* bufferMemory );
	void destroyBuffer ( vk::Buffer buffer, vk::DeviceMemory bufferMemory );
	
	vk::ImageView createImageView2D (vk::Image image, u32 mipBase, u32 mipOffset, vk::Format format, vk::ImageAspectFlags aspectFlags);
	vk::ImageView createImageView2DArray (vk::Image image, u32 mipBase, u32 mipOffset, u32 arrayOffset, u32 arraySize, vk::Format format, vk::ImageAspectFlags aspectFlags);
	
	vk::CommandPool createTransferCommandPool(vk::CommandPoolCreateFlags createFlags);
	vk::CommandPool createGraphicsCommandPool(vk::CommandPoolCreateFlags createFlags);
	
	void copyBufferToImage (vk::Buffer srcBuffer, vk::Image dstImage, vk::DeviceSize srcOffset, vk::Offset3D dstOffset, vk::Extent3D extent, u32 index,
                        vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag,
                        vk::CommandPool commandPool, vk::Queue submitQueue);
		
	void transferData(const void* srcData, vk::Image targetImage, vk::Offset3D offset, vk::Extent3D extent, u32 index, vk::DeviceSize size, vk::PipelineStageFlags usePipelineFlags, vk::AccessFlags useFlag,
		vk::CommandPool commandPool, vk::Queue submitQueue);
	
	void transitionImageLayout (vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask,
                            vk::CommandPool commandPool, vk::Queue submitQueue);
							
	void copyData(const void* srcData, vk::DeviceMemory dstMemory, vk::DeviceSize offset, vk::DeviceSize size);

	vk::Format findDepthFormat();
	u32 findMemoryType(u32 typeFilter, vk::MemoryPropertyFlags properties);

	vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
	
	vk::ShaderModule loadShaderFromFile(const char* filename);

	std::vector<vk::DescriptorSet> createDescriptorSets(vk::DescriptorPool descriptorSetPool, std::vector<vk::DescriptorSetLayout>* descriptorSetLayouts);

	vk::CommandBuffer createCommandBuffer(vk::CommandPool commandPool, vk::CommandBufferLevel bufferLevel);
	void deleteCommandBuffer(vk::CommandPool commandPool, vk::CommandBuffer commandBuffer);
	
	vk::DeviceMemory allocateMemory(vk::MemoryRequirements memoryRequirement, vk::MemoryPropertyFlags properties);
	void destroyCommandPool(vk::CommandPool commandPool);
	
	vk::Semaphore createSemaphore();
	void destroySemaphore(vk::Semaphore semaphore);
	
	void destroyPipelineLayout (vk::PipelineLayout pipelineLayout) {
		vkDestroyPipelineLayout (device, pipelineLayout, nullptr);
	}
	void destroyDescriptorSetLayout(vk::DescriptorSetLayout layout);

	void destroyPipeline (vk::Pipeline pipeline) {
		vkDestroyPipeline (device, pipeline, nullptr);
	}
	void destroyRenderPass(vk::RenderPass renderpass){
		vkDestroyRenderPass(device, renderpass, nullptr);
	}
};
struct BufferWrapper;

void transferData(const void* srcData, BufferWrapper* targetBuffer, vk::DeviceSize offset, vk::DeviceSize size, vk::PipelineStageFlags usePipelineFlags, vk::AccessFlags useFlag,
	vk::CommandPool commandPool, vk::Queue submitQueue);
void transferData(const void* srcData, BufferWrapper* targetBuffer, vk::DeviceSize offset, vk::DeviceSize size, vk::PipelineStageFlags usePipelineFlags, vk::AccessFlags useFlag, vk::CommandBuffer commandBuffer);

void copyBuffer (BufferWrapper* srcBuffer, BufferWrapper* dstBuffer, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset, vk::DeviceSize size,
		vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag,
		vk::CommandPool commandPool, vk::Queue submitQueue);
void copyBuffer (BufferWrapper* srcBuffer, BufferWrapper* dstBuffer, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset, vk::DeviceSize size,
	vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag,
	vk::CommandBuffer commandBuffer);

vk::DescriptorPool createStandardDescriptorSetPool(VInstance* instance);

#endif // VINSTANCE_H
