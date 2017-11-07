#ifndef VINSTANCE_H
#define VINSTANCE_H

#include <vulkan/vulkan.h>
#include <vector>
#include <string.h>
#include "VHeader.h"
#include "VDevice.h"

struct VExtLayerStruct{
	std::vector<VkLayerProperties> availableLayers;
	std::vector<VkExtensionProperties> availableExtensions;
	std::vector<const char*> neededLayers;
	std::vector<const char*> neededExtensions;
	
	bool activateLayer(const char* name);
	bool activateExtension(const char* name);
};

struct VPhysDeviceProps{
	vk::PhysicalDevice physicalDevice;
	int rating = 0;
	std::vector<VkExtensionProperties> availableExtensions;
	std::vector<VkLayerProperties> availableLayers;
	std::vector<VkQueueFamilyProperties> queueFamilyProps;
	VkPhysicalDeviceProperties vkPhysDevProps;
	VkPhysicalDeviceFeatures  vkPhysDevFeaturess;
};


struct VGlobal{
	
	vk::Instance vkinstance;
	VExtLayerStruct instExtLayers;
	
	vk::PhysicalDevice physicalDevice;
	VExtLayerStruct devExtLayers;
	
	uint32_t chosenDeviceId = -1;
	
	VDevice deviceWrapper;
	
	struct {
		
	}pipelines;
	
	std::vector<VPhysDeviceProps> physicalDevices;
	
	bool preInitialize();
	bool initializeInstance(const char* appName = nullptr, const char* engineName = nullptr);
	bool choseBestDevice();
	bool choseDevice(uint32_t index);
	bool initializeDevice();
	void terminate();
	
};

extern VGlobal vGlobal;




#endif // VINSTANCE_H
