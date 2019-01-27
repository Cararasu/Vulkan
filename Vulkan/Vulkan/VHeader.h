#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <render/Logger.h>
#include <functional>
#include <render/Header.h>

// save diagnostic state
#pragma GCC diagnostic push
// turn off the specific warning. Can also use "-Wall"
#pragma GCC diagnostic ignored "-Wall"
#include <vulkan/vulkan.hpp>
// turn the warnings back on
#pragma GCC diagnostic pop

constexpr u32 MAX_PRESENTIMAGE_COUNT = 2;

void printError ( VkResult res );

#define VCHECKCALL(call, errorcode) {\
		if(VkResult res = call){\
			errorcode;\
			printError(res);\
		}\
	}

#define V_CHECKCALL(call, errorcode) {\
		vk::Result res = call;\
		if(res != vk::Result::eSuccess){\
			printf("Error %s in call %s\n", vk::to_string(res).c_str(), #call);\
			errorcode;\
		}\
	}
#ifdef VULKAN_HPP_DISABLE_ENHANCED_MODE
#define V_CHECKCALL_MAYBE(call, errorcode) {\
		vk::Result res = call;\
		if(res != vk::Result::eSuccess){\
			printf("Error %s in call %s\n", vk::to_string(res).c_str(), #call);\
			errorcode;\
		}\
	}
#else
#define V_CHECKCALL_MAYBE(call, errorcode) {\
		call;\
	}
#endif

struct ResettableCommandBuffer {
	bool should_reset = true;
	vk::CommandBuffer buffer;
};

struct QueueWrapper {
	u32 graphics_queue_id = -1;
	vk::Queue graphics_queue;

	bool combined_graphics_present_queue = false;
	u32 present_queue_id = -1;
	vk::Queue present_queue;

	bool combined_graphics_compute_queue = false;
	u32 compute_queue_id = -1;
	vk::Queue compute_queue;

	bool dedicated_transfer_queue = false;
	u32 transfer_queue_id = -1;
	vk::Queue transfer_queue;
};

struct FrameRenderContext {
	u32 max_buffer_frames;
	u32 buffer_frame_index;

	QueueWrapper* queue_wrapper;
	u32 pgcqueue_index;
};


struct GPUMemory {
	vk::DeviceMemory memory;
	vk::DeviceSize size;
	u32 heap_index;
	vk::MemoryPropertyFlags property_flags, needed, recommended;

	GPUMemory() {}
	GPUMemory ( vk::DeviceSize size, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) : size ( size ), needed ( needed ), recommended ( recommended ) {}
	GPUMemory ( vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) : needed ( needed ), recommended ( recommended ) {}
};

extern const Logger v_logger;
