#ifndef VINSTANCE_H
#define VINSTANCE_H

#include <vulkan/vulkan.hpp>
#include <vector>
#include <string.h>
#include "VHeader.h"
#include "VDevice.h"
#include "PipelineModule.h"

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


struct VGlobal{
	
	vk::Instance vkinstance;
	VExtLayerStruct instExtLayers;
	
	vk::PhysicalDevice physicalDevice;
	VExtLayerStruct devExtLayers;
	
	vk::DebugReportCallbackEXT debugReportCallbackEXT;
	
	uint32_t chosenDeviceId = -1;
	
	VDevice deviceWrapper;
	
	struct{
		vk::ShaderModule standardShaderVert;
		vk::ShaderModule standardShaderFrag;
	} shadermodule;
	
	std::vector<VPhysDeviceProps> physicalDevices;
	
	bool preInitialize();
	bool initializeInstance(const char* appName = nullptr, const char* engineName = nullptr);
	bool choseBestDevice();
	bool choseDevice(uint32_t index);
	bool initializeDevice();
	
	struct{
		StandardPipelineModuleBuilder standard;
	}pipeline_module_builders;
	struct{
		PipelineModuleLayout standard;
	}pipeline_module_layouts;
	
	void terminate();
	
};

extern VGlobal global;




#endif // VINSTANCE_H
