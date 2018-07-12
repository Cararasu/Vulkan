#pragma once

#include "render/Instance.h"
#include "render/Window.h"
#include "VGlobal.h"
#include "VulkanHeader.h"
#include "VulkanResourceManager.h"

#define V_MAX_PGCQUEUE_COUNT (2)

struct VulkanDevice : public Device {
	vk::PhysicalDevice physical_device;

	std::vector<vk::ExtensionProperties> availableExtensions;
	std::vector<vk::LayerProperties> availableLayers;
	std::vector<vk::QueueFamilyProperties> queueFamilyProps;
	vk::PhysicalDeviceProperties vkPhysDevProps;
	vk::PhysicalDeviceFeatures  vkPhysDevFeatures;

	virtual ~VulkanDevice() {}
};

struct VulkanMonitor : public Monitor {
	GLFWmonitor* monitor;

	VulkanMonitor ( GLFWmonitor* );
	virtual ~VulkanMonitor();

	virtual VideoMode current_mode();
};


struct VulkanWindow;
struct VulkanWindowSection;

struct VulkanInstance : public Instance {
	VExtLayerStruct extLayers;
	Map<GLFWmonitor*, VulkanMonitor*> monitor_map;
	Map<GLFWwindow*, VulkanWindow*> window_map;
	Array<Monitor*> monitors;
	Array<Device*> devices;
	Set<VulkanWindow*> windows;
	Set<VulkanWindowSection*> window_sections;

	vk::Instance m_instance;
	VulkanDevice* v_device;
	vk::Device m_device;
	vk::DebugReportCallbackEXT debugReportCallbackEXT;

	QueueWrapper queues;

	VulkanResourceManager* v_resource_manager;

	bool initialized = false;


	VulkanInstance();
	virtual ~VulkanInstance();

	virtual bool initialize ( Device* device = nullptr );

	virtual void process_events();
	virtual bool is_window_open();

	virtual Window* create_window();
	virtual bool destroy_window ( Window* window );

	virtual WindowSection* create_window_section ( WindowSectionType type );
	virtual bool destroy_window_section ( WindowSection* window_section );

	virtual Array<Monitor*>& get_monitors();
	virtual Array<Device*>& get_devices();
	VulkanMonitor* get_primary_monitor_vulkan();
	virtual Monitor* get_primary_monitor();

	virtual ResourceManager* resource_manager();



//------------ Instance/Device/Physicaldevice

	inline PGCQueueWrapper* vulkan_pgc_queue ( u32 index ) {
		return &queues.pgc[index];
	}


	GPUMemory allocate_gpu_memory ( vk::MemoryRequirements mem_req, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended );
	RendResult free_gpu_memory ( GPUMemory memory );

	vk::ImageView createImageView2D ( vk::Image image, u32 mipBase, u32 mipOffset, vk::Format format, vk::ImageAspectFlags aspectFlags );
	vk::ImageView createImageView2DArray ( vk::Image image, u32 mipBase, u32 mipOffset, u32 arrayOffset, u32 arraySize, vk::Format format, vk::ImageAspectFlags aspectFlags );

	vk::DeviceMemory allocateMemory ( vk::MemoryRequirements memoryRequirement, vk::MemoryPropertyFlags properties );

	vk::CommandPool createTransferCommandPool ( vk::CommandPoolCreateFlags createFlags = vk::CommandPoolCreateFlags() );
	vk::CommandPool createGraphicsCommandPool ( vk::CommandPoolCreateFlags createFlags = vk::CommandPoolCreateFlags() );
	void destroyCommandPool ( vk::CommandPool commandPool );
	vk::CommandBuffer createCommandBuffer ( vk::CommandPool commandPool, vk::CommandBufferLevel bufferLevel );
	void deleteCommandBuffer ( vk::CommandPool commandPool, vk::CommandBuffer commandBuffer );
	void copyData ( const void* srcData, vk::DeviceMemory dstMemory, vk::DeviceSize offset, vk::DeviceSize size );

	u32 findMemoryType ( u32 typeFilter, vk::MemoryPropertyFlags properties );
	vk::Format findSupportedFormat ( const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features );
	vk::Format findDepthFormat();
};

inline QueueWrapper* vulkan_queue_wrapper ( VulkanInstance* instance ) {
	return &instance->queues;
}
inline PGCQueueWrapper* vulkan_pgc_queue_wrapper ( VulkanInstance* instance, u32 pgc_index ) {
	return &instance->queues.pgc[pgc_index];
}

inline vk::Device vulkan_device ( const VulkanInstance* instance ) {
	return instance->m_device;
}
inline vk::PhysicalDevice vulkan_physical_device ( const VulkanInstance* instance ) {
	return instance->v_device->physical_device;
}
inline vk::Semaphore create_semaphore ( VulkanInstance* instance ) {
	return instance->m_device.createSemaphore ( vk::SemaphoreCreateInfo(), nullptr );
}
inline RendResult destroy_semaphore ( const VulkanInstance* const instance, vk::Semaphore sem ) {
	instance->m_device.destroySemaphore ( sem, nullptr );
	return RendResult::eSuccess;
}

inline bool hasStencilComponent ( vk::Format format ) {
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

inline RendResult wrap_command ( std::function<RendResult ( vk::CommandBuffer ) > do_command, VulkanInstance* v_instance, vk::CommandPool commandpool, vk::CommandBuffer* cmdbuffer ) {
	*cmdbuffer = v_instance->createCommandBuffer ( commandpool, vk::CommandBufferLevel::ePrimary );
	cmdbuffer->begin ( vk::CommandBufferBeginInfo ( vk::CommandBufferUsageFlagBits::eOneTimeSubmit ) );
	RendResult res = do_command ( *cmdbuffer );
	cmdbuffer->end();
	return res;
}