#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <string.h>
#include "VHeader.h"

struct VExtLayerStruct{
	std::vector<vk::LayerProperties> availableLayers;
	std::vector<vk::ExtensionProperties> availableExtensions;
	std::vector<const char*> neededLayers;
	std::vector<const char*> neededExtensions;
	
	bool activateLayer(const char* name);
	bool activateExtension(const char* name);
};

struct VPhysDeviceProps{
	vk::PhysicalDevice physicalDevice;
	int rating = 0;
	std::vector<vk::ExtensionProperties> availableExtensions;
	std::vector<vk::LayerProperties> availableLayers;
	std::vector<vk::QueueFamilyProperties> queueFamilyProps;
	vk::PhysicalDeviceProperties vkPhysDevProps;
	vk::PhysicalDeviceFeatures  vkPhysDevFeatures;
};

struct VInstance;

struct VGlobal{
	
	vk::Instance vkinstance;
	VExtLayerStruct extLayers;
	
	vk::DebugReportCallbackEXT debugReportCallbackEXT;
	
	struct{
		vk::ShaderModule standardShaderVert;
		vk::ShaderModule standardShaderFrag;
	} shadermodule;
	
	std::vector<VPhysDeviceProps> physicalDevices;
	
	bool preInitialize();
	bool initializeInstance(const char* appName = nullptr, const char* engineName = nullptr);
	
	//List getGPUs
	VInstance* createInstance(u32 GPUIndex = 0);
	bool initializeDevice(VInstance* instance);
	
	void terminate();
	
};

extern VGlobal global;

