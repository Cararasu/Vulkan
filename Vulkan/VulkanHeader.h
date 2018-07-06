#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>
#include <render/Header.h>
#include <vulkan/vulkan.hpp>

struct PGCQueueWrapper{
	u32 graphics_queue_id = -1;
	vk::Queue graphics_queue;
	
	bool combined_graphics_present_queue = false;
	u32 present_queue_id = -1;
	vk::Queue present_queue;
	
	bool combined_graphics_compute_queue = false;
	u32 compute_queue_id = -1;
	vk::Queue compute_queue;
};

struct QueueWrapper{
	Array<PGCQueueWrapper> pgc;
	u32 index = 0;
	
	bool dedicated_transfer_queue = false;
	u32 transfer_queue_id = -1;
	vk::Queue transfer_queue;
};

struct FrameRenderContext{
	u32 max_buffer_frames;
	u32 buffer_frame_index;
	
	QueueWrapper* queue_wrapper;
	u32 pgcqueue_index;
};


struct GPUMemory {
	vk::DeviceMemory memory;
	vk::DeviceSize size;
	u32 heap_index;
	vk::MemoryPropertyFlags property_flags;
};

struct VulkanInstance;

RendResult submit_command( std::function<RendResult ( vk::CommandBuffer ) > do_command, VulkanInstance* instance, vk::CommandPool commandPool, vk::Queue submitQueue );