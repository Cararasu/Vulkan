#ifndef VINSTANCE_H
#define VINSTANCE_H

#include <vulkan/vulkan.h>
#include <vector>
#include <string.h>
#include "VHeader.h"
#include "VDevice.h"

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
	
	
	std::vector<VPhysDeviceProps> physicalDevices;
	
	bool preInitialize();
	bool initializeInstance(const char* appName = nullptr, const char* engineName = nullptr);
	bool choseBestDevice();
	bool choseDevice(uint32_t index);
	bool initializeDevice();
	

	struct{
		vk::RenderPass standardRenderPass;
	} renderpass;
	
	struct{
		vk::PipelineLayout standardPipelineLayout;
	} pipelinelayout;
	
	struct{
		vk::ShaderModule standardShaderVert;
		vk::ShaderModule standardShaderFrag;
	} shadermodule;
	
	struct{
		vk::Pipeline standardPipeline;
	} pipeline;
	
	struct{
		std::vector<vk::DescriptorSetLayout> standardDescriptorSetLayouts;
	} descriptorsetlayout;
	
	void buildStandardPipeline(vk::Format format, VkExtent2D extent);
	
	void terminate();
	
};

extern VGlobal global;




#endif // VINSTANCE_H
