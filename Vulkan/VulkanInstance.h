#pragma once

#include "render/Instance.h"
#include "render/Window.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VGlobal.h"
#include "VulkanWindow.h"

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
	Map<GLFWmonitor*, VulkanMonitor*> monitor_map;
	Map<GLFWwindow*, VulkanWindow*> window_map;
	Array<Monitor*> monitors;
	Array<Device*> devices;
	Set<VulkanWindow*> windows;
	Set<VulkanWindowSection*> window_sections;
	
	vk::Instance m_instance;
	VulkanDevice* vulkan_device;
	vk::Device m_device;
	vk::DebugReportCallbackEXT debugReportCallbackEXT;
	
	QueueWrapper queues;
	
	bool initialized = false;


	VulkanInstance();
	virtual ~VulkanInstance();

	virtual bool initialize(Device* device = nullptr);
	
	virtual void process_events();
	virtual bool is_window_open();

	virtual Window* create_window();
	virtual bool destroy_window (Window* window);
	
	virtual WindowSection* create_window_section(WindowSectionType type);
	virtual bool destroy_window_section(WindowSection* window_section);
	
	virtual Array<Monitor*>& get_monitors();
	virtual Array<Device*>& get_devices();
	VulkanMonitor* get_primary_monitor_vulkan();
	virtual Monitor* get_primary_monitor();

};

inline vk::Device vulkan_device(VulkanInstance* instance){
	return instance->m_device;
}
inline vk::PhysicalDevice vulkan_physical_device(VulkanInstance* instance){
	return instance->vulkan_device->physical_device;
}
inline vk::Semaphore create_semaphore(VulkanInstance* instance){
	return instance->m_device.createSemaphore(vk::SemaphoreCreateInfo(), nullptr);
}
inline RendResult destroy_semaphore(VulkanInstance* instance, vk::Semaphore sem){
	instance->m_device.destroySemaphore(sem, nullptr);
	return RendResult::eSuccess;
}