#pragma once

#include "render/Window.h"
#include "VulkanHeader.h"
#include "VulkanImage.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VGlobal.h"

#define MAX_PRESENTIMAGE_COUNT (3)

struct VulkanWindow;
struct VulkanInstance;

struct VulkanWindowSection : public WindowSection {
	const VulkanInstance* m_instance;
	Viewport<f32> m_viewport;
	RenderTargetWrapper* m_target_wrapper;

	VulkanWindow* m_parent_window;
	VulkanWindowSection* m_parent;


	VulkanWindowSection ( WindowSectionType type, VulkanInstance* instance ) : WindowSection ( type ), m_instance ( instance ),
		m_viewport(), m_target_wrapper ( nullptr ), m_parent_window ( nullptr ), m_parent ( nullptr ) {}
	virtual ~VulkanWindowSection() {}

	RendResult register_parent ( VulkanWindow* parent_window, VulkanWindowSection* parent ) {
		if ( m_parent_window || m_parent )
			return RendResult::eAlreadyRegistered;
		m_parent_window = parent_window;
		m_parent = parent;
		return RendResult::eSuccess;
	}
	void unregister_parent() {
		m_parent_window = nullptr;
		m_parent = nullptr;
	}

	virtual void update_viewport ( Viewport<f32> viewport, RenderTargetWrapper* target_wrapper ) {
		m_viewport = viewport;
		m_target_wrapper = target_wrapper;
		printf ( "Viewport changed %g x %g | %g x %g | %g - %g\n", viewport.offset.x, viewport.offset.y, viewport.extend.width, viewport.extend.height, viewport.depth.min, viewport.depth.max );
	}

};
struct UIVulkanWindowSection : public VulkanWindowSection {
	//here an optimized quad renderer
	

	UIVulkanWindowSection ( VulkanInstance* instance ) : VulkanWindowSection ( WindowSectionType::eUI, instance ) {}
	virtual ~UIVulkanWindowSection() {}

	virtual void render_frame();
	virtual void set_root_node ( UINode* node ) {}
	virtual void set_stack_size ( uint32_t size ) {}
	virtual void set_layer ( u32 layer, WindowSection* section ) {}
	virtual void set_world() {}
};

struct FrameLocalData{
	bool initialized = false;
	vk::Fence image_presented_fence;
	vk::Semaphore render_ready_sem;
	vk::Semaphore present_ready_sem;
	
	Array<vk::Semaphore> generated_sems;
	Array<vk::Semaphore> sem_wait_needed;
	
	//recreate for every resize
	VulkanImageWrapper* present_image;
	vk::ImageView present_image_view;
	vk::CommandBuffer clear_command_buffer;
	vk::CommandBuffer present_command_buffer;
	
	//needs to be destroyed before the frame is started and is created when needed
	vk::CommandPool graphics_command_pool;
};

struct VulkanWindow : public Window {
	VulkanInstance* m_instance = nullptr;
	VulkanWindowSection* m_root_section = nullptr;
	Extent2D<s32> framebuffer_size;
	VulkanWindowSection* section;
	
	//glfw
	GLFWwindow* window = nullptr;
	
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
	
	vk::CommandPool window_graphics_command_pool;
	
	VulkanImageWrapper* depth_image = nullptr;
	vk::ImageView depth_image_view;


	VulkanWindow ( VulkanInstance* instance );
	virtual ~VulkanWindow();

	virtual RendResult set_root_section ( WindowSection* section );

	virtual RendResult update();
	virtual RendResult destroy();
	
	void framebuffer_size_changed(s32 x, s32 y);
	
	vk::CommandPool graphics_command_pool(){
		if(!frame_local_data[present_image_index].graphics_command_pool)
			frame_local_data[present_image_index].graphics_command_pool = vulkan_device(m_instance).createCommandPool(vk::CommandPoolCreateInfo());
		return frame_local_data[present_image_index].graphics_command_pool;
	}
	
	void initialize();
	void render_frame();
	void create_command_buffers();
	
	void create_swapchain();
	void create_empty_pipeline();
	void destroy_depth_image();
	void create_frame_local_data(std::vector<vk::Image> swapChainImages);
	void destroy_frame_local_data();
};
