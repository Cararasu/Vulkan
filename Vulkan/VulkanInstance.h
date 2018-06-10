#pragma once

#include "render/Instance.h"
#include "render/Window.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VGlobal.h"

#define V_MAX_PGCQUEUE_COUNT (2)

struct VulkanDevice : public Device{
	vk::PhysicalDevice physical_device;
	
	std::vector<vk::ExtensionProperties> availableExtensions;
	std::vector<vk::LayerProperties> availableLayers;
	std::vector<vk::QueueFamilyProperties> queueFamilyProps;
	vk::PhysicalDeviceProperties vkPhysDevProps;
	vk::PhysicalDeviceFeatures  vkPhysDevFeatures;
	
	virtual ~VulkanDevice(){}
};

struct VulkanMonitor : public Monitor {
	GLFWmonitor* monitor;

	VulkanMonitor (GLFWmonitor*);
	virtual ~VulkanMonitor();

	virtual VideoMode current_mode();
};

enum class WindowState {
	eUninitialized,
	eInitialized,
};
enum class FrameState {
	eUninitialized,
	eFramePrepared,
	eFramePresented,
	eNotVisible,
	eResized
};

struct VulkanInstance;

class VulkanWindow : public Window {

	WindowState windowstate = WindowState::eUninitialized;
	FrameState framestate = FrameState::eUninitialized;
	VulkanInstance* m_instance;
	GLFWwindow* window;
	vk::SurfaceKHR surface;
public:
	VulkanWindow (VulkanInstance* instance);
	virtual ~VulkanWindow();

	virtual void set_root_section (WindowSection* section);

	virtual RendResult update();
	virtual RendResult destroy();
};

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
	
	bool dedicated_transfer_queue = false;
	u32 transfer_queue_id = -1;
	vk::Queue transfer_queue;
};

struct VulkanInstance : public Instance {
	VExtLayerStruct extLayers;
	Map<GLFWmonitor*, VulkanMonitor*> monitormap;
	Array<Monitor*> monitors;
	Array<Device*> devices;
	Set<VulkanWindow*> windows;
	
	vk::Instance m_instance;
	VulkanDevice* vulkan_device;
	vk::Device m_device;
	vk::DebugReportCallbackEXT debugReportCallbackEXT;
	
	QueueWrapper queues;
	
	bool initialized = false;


	VulkanInstance();
	virtual ~VulkanInstance();

	virtual bool initialize(Device* device = nullptr);

	virtual Window* create_window();
	virtual Array<Monitor*>& get_monitors();
	virtual Array<Device*>& get_devices();
	VulkanMonitor* get_primary_monitor_vulkan();
	virtual Monitor* get_primary_monitor();
	virtual bool destroy_window (Window* window);

};
