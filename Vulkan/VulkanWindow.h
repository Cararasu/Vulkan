#pragma once

#include "render/Window.h"
#include "VulkanHeader.h"
#include "VulkanImage.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VGlobal.h"

#include <functional>

#include "VulkanWindowSection.h"

#define MAX_PRESENTIMAGE_COUNT (3)


struct FrameLocalData {
	bool initialized = false;
	vk::Fence image_presented_fence;
	vk::Semaphore render_ready_sem;
	vk::Semaphore present_ready_sem;

	//recreate for every resize
	VulkanImageWrapper* present_image;
	vk::ImageView present_image_view;
	vk::CommandBuffer clear_command_buffer;
	vk::CommandBuffer present_command_buffer;

	Array<std::function<RendResult ( u32 ) >> deferred_calls;

	//needs to be destroyed before the frame is started and is created when needed
	vk::CommandPool graphics_command_pool;
};

struct VulkanWindow : public Window {
	VulkanInstance* m_instance = nullptr;
	VulkanWindowSection* v_root_section = nullptr;

	//glfw
	GLFWwindow* window = nullptr;

	VulkanQuadRenderer quad_renderer;
	VulkanSingleImage depth_image;
	
	//vulkan
	vk::SurfaceKHR surface;
	vk::Semaphore image_available_guard_sem;
	vk::SurfaceFormatKHR present_swap_format;
	vk::PresentModeKHR chosen_presentation_mode;
	vk::SurfaceCapabilitiesKHR capabilities;
	vk::SwapchainKHR swap_chain;
	Extent2D<u32> swap_chain_extend;
	u32 image_buffer_count;
	u32 queue_index = 0;
	u32 present_image_index = 0;
	Array<FrameLocalData> frame_local_data;
	Array<vk::Semaphore> active_sems;

	vk::CommandPool window_graphics_command_pool;

	VulkanRenderTarget v_render_target_wrapper;


	VulkanWindow ( VulkanInstance* instance );
	virtual ~VulkanWindow();

	virtual RendResult root_section ( WindowSection* section );
	virtual WindowSection* root_section ( );

	virtual RendResult update();
	virtual RendResult destroy();

	void framebuffer_size_changed ( Extent2D<s32> extend );

	vk::CommandPool graphics_command_pool() {
		if ( !frame_local_data[present_image_index].graphics_command_pool )
			frame_local_data[present_image_index].graphics_command_pool = vulkan_device ( m_instance ).createCommandPool ( vk::CommandPoolCreateInfo() );
		return frame_local_data[present_image_index].graphics_command_pool;
	}

	inline FrameLocalData* current_framelocal_data() {
		return &frame_local_data[present_image_index];
	}

	void initialize();
	void render_frame();
	void create_command_buffers();

	void create_swapchain();
	void create_empty_pipeline();
	void create_frame_local_data ( std::vector<vk::Image> swapChainImages );
	void destroy_frame_local_data();
};

struct SubmitInfo{
	vk::PipelineStageFlags wait_dst_stage_mask;
	u32 need_sem_index, need_sem_count;
	u32 comm_buff_index, comm_buff_count;
	u32 sig_sem_index, sig_sem_count;
};
struct SubmitStore{
	Array<vk::Semaphore> semaphores;
	Array<vk::CommandBuffer> commandbuffers;
	Array<SubmitInfo> submitinfos;
	vk::Fence signal_fence;
};