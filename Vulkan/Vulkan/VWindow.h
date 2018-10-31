#pragma once

#include "render/Window.h"
#include "VHeader.h"
#include "VImage.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <functional>

#define MAX_PRESENTIMAGE_COUNT (3)

struct VWindowImage;

struct FrameLocalData {
	bool initialized = false;
	u64 frame_index;
	vk::Fence image_presented_fence;
	vk::Semaphore render_ready_sem;
	vk::Semaphore present_ready_sem;

	//recreate for every resize
	bool createcommandbuffer;
	vk::CommandBuffer clear_command_buffer;
	vk::CommandBuffer present_command_buffer;

	//needs to be destroyed before the frame is started and is created when needed
	vk::CommandPool graphics_command_pool;
};

struct VQuadRenderer;
struct VInstance;
struct VBaseImage;

struct VRenderTarget : public RenderTarget {
	Array<VBaseImage*> images;
	VBaseImage* depth_image;
	u32 target_count;
};


struct VWindow : public Window {
	VInstance* m_instance = nullptr;

	//glfw
	GLFWwindow* window = nullptr;

	VQuadRenderer* quad_renderer;
	VWindowImage* present_image = nullptr;
	VBaseImage* depth_image = nullptr;
	
	VRenderTarget v_render_target_wrapper;
	
	//vulkan
	vk::SurfaceKHR surface;
	vk::Semaphore image_available_guard_sem;
	vk::SurfaceFormatKHR present_swap_format;
	vk::PresentModeKHR chosen_presentation_mode;
	vk::SurfaceCapabilitiesKHR capabilities;
	vk::SwapchainKHR swap_chain;
	Extent2D<u32> swap_chain_extend;
	u32 present_image_index = 0;
	DynArray<FrameLocalData> frame_local_data;
	DynArray<vk::Semaphore> active_sems;

	vk::CommandPool window_graphics_command_pool;

	VWindow ( VInstance* instance );
	virtual ~VWindow();
	
	virtual Image* backed_image ();

	virtual RendResult update();
	RendResult v_update();
	virtual RendResult destroy();

	void framebuffer_size_changed ( Extent2D<s32> extend );

	vk::CommandPool graphics_command_pool();

	inline FrameLocalData* current_framelocal_data() {
		return &frame_local_data[present_image_index];
	}
	
	void prepare_frame();
	
	void initialize();
	void render_frame();
	void create_command_buffers();
	void create_command_buffer ( u32 index );

	void create_swapchain();
	void create_empty_pipeline();
	void create_frame_local_data ( u32 count );
	void destroy_frame_local_data();
};

struct SubmitInfo {
	vk::PipelineStageFlags wait_dst_stage_mask;
	u32 need_sem_index, need_sem_count;
	u32 comm_buff_index, comm_buff_count;
	u32 sig_sem_index, sig_sem_count;
};
struct SubmitStore {
	DynArray<vk::Semaphore> semaphores;
	DynArray<vk::CommandBuffer> commandbuffers;
	DynArray<SubmitInfo> submitinfos;
	vk::Fence signal_fence;
};
