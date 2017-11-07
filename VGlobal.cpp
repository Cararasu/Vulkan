#include "VGlobal.h"
#include "VHeader.h"
#include <stdio.h>
#include <algorithm>
#include <assert.h>
#include <algorithm>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>

VGlobal vGlobal;


bool operator==(VkLayerProperties& lhs, VkLayerProperties& rhs){
	return !strcmp(lhs.layerName, rhs.layerName);
}
bool operator==(VkExtensionProperties& lhs, VkExtensionProperties& rhs){
	return !strcmp(lhs.extensionName, rhs.extensionName);
}

void addExtension(std::vector<VkExtensionProperties>* propList, VkExtensionProperties prop){
	bool found = false;
	for(VkExtensionProperties& it : *propList){
		if(it == prop){
			found = true;
			break;
		}
	}
	if(!found){
		propList->push_back(prop);
	}
}

void gatherExtLayer(VkPhysicalDevice device, std::vector<VkLayerProperties>* layers, std::vector<VkExtensionProperties>* extensions){
	
	uint32_t count;
	if(device == VK_NULL_HANDLE){
		VCHECKCALL(vkEnumerateInstanceExtensionProperties (nullptr, &count, nullptr), {
			printf("Could not get Extension-count");
		});
	}else{
		VCHECKCALL(vkEnumerateDeviceExtensionProperties (device, nullptr, &count, nullptr), {
			printf("Could not get Extension-count");
		});
	}
	extensions->resize(count);
	if(device == VK_NULL_HANDLE){
		VCHECKCALL(vkEnumerateInstanceExtensionProperties (nullptr, &count, extensions->data()), {
			printf("Could not get Extensions");
		});
	}else{
		VCHECKCALL(vkEnumerateDeviceExtensionProperties (device, nullptr, &count, extensions->data()), {
			printf("Could not get Extensions");
		});
	}
	
	if(device == VK_NULL_HANDLE){
		VCHECKCALL(vkEnumerateInstanceLayerProperties (&count, nullptr), {
			printf("Could not get Layer-count");
		});
	}else{
		VCHECKCALL(vkEnumerateDeviceLayerProperties (device, &count, nullptr), {
			printf("Could not get Layer-count");
		});
	}
	layers->resize(count);
	if(device == VK_NULL_HANDLE){
		VCHECKCALL(vkEnumerateInstanceLayerProperties (&count, layers->data()), {
			printf("Could not get Layers");
		});
	}else{
		VCHECKCALL(vkEnumerateDeviceLayerProperties (device, &count, layers->data()), {
			printf("Could not get Layers");
		});
	}
	for(VkLayerProperties& layerProp : *layers){
		
		if(device == VK_NULL_HANDLE){
			VCHECKCALL(vkEnumerateInstanceExtensionProperties (layerProp.layerName, &count, nullptr), {
				printf("Could not get Extension-count");
			});
		}else{
			VCHECKCALL(vkEnumerateDeviceExtensionProperties (device, layerProp.layerName, &count, nullptr), {
				printf("Could not get Extension-count");
			});
		}
		VkExtensionProperties extensionArray[count];
		if(device == VK_NULL_HANDLE){
			VCHECKCALL(vkEnumerateInstanceExtensionProperties (layerProp.layerName, &count, extensionArray), {
				printf("Could not get Extensions");
			});
		}else{
			VCHECKCALL(vkEnumerateDeviceExtensionProperties (device, layerProp.layerName, &count, extensionArray), {
				printf("Could not get Extensions");
			});
		}
		for(size_t i = 0; i < count; ++i){
			addExtension(extensions, extensionArray[i]);
		}
	}
}

bool VGlobal::preInitialize(){
	
	if (glfwVulkanSupported()) {
		printf ("Vulkan supported\n");
		// Vulkan is available, at least for compute
	} else {

		printf ("Vulkan not supported\n");
		return false;
	}
	gatherExtLayer(VK_NULL_HANDLE, &instExtLayers.availableLayers, &instExtLayers.availableExtensions);
	
	return true;
}
bool VGlobal::initializeInstance(const char* appName, const char* engineName){
	
	vk::ApplicationInfo appInfo(appName, VK_MAKE_VERSION (1, 0, 0), engineName, VK_MAKE_VERSION (1, 0, 0), VK_MAKE_VERSION (1, 0, 61));
	
	vk::InstanceCreateInfo instanceCreateInfo(vk::InstanceCreateFlags(), &appInfo,
		instExtLayers.neededLayers.size(), instExtLayers.neededLayers.data(),
		instExtLayers.neededExtensions.size(), instExtLayers.neededExtensions.data());
	V_CHECKCALL(vk::createInstance (&instanceCreateInfo, nullptr, &vkinstance), printf("Instance Creation Failed\n"));
	
	uint32_t devicecount = 0;
	VCHECKCALL(vkEnumeratePhysicalDevices (vkinstance, &devicecount, nullptr), {
		printf("Get physical-device count Failed\n");
	});
	VkPhysicalDevice physDevices[devicecount];
	physicalDevices.resize(devicecount);
	VCHECKCALL(vkEnumeratePhysicalDevices (vkinstance, &devicecount, physDevices), {
		printf("Get physical-devicec Failed\n");
	});

	for (size_t i = 0; i < devicecount; i++) {
		VPhysDeviceProps& prop = physicalDevices[i];
		prop.physicalDevice = physDevices[i];
		vkGetPhysicalDeviceProperties (physDevices[i], &prop.vkPhysDevProps);
		vkGetPhysicalDeviceFeatures (physDevices[i], &prop.vkPhysDevFeaturess);
		
		gatherExtLayer(physDevices[i], &prop.availableLayers, &prop.availableExtensions);
		
		uint32_t count;
		vkGetPhysicalDeviceQueueFamilyProperties(prop.physicalDevice, &count, nullptr);
		prop.queueFamilyProps.resize(count);
		vkGetPhysicalDeviceQueueFamilyProperties(prop.physicalDevice, &count, prop.queueFamilyProps.data());
		
		{//rate a physicalDevice
			prop.rating = 0;
			if (prop.vkPhysDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				prop.rating += 1000;
			}
			// Maximum possible size of textures affects graphics quality
			prop.rating += prop.vkPhysDevProps.limits.maxImageDimension2D;
			// Application can't function without geometry shaders
			if (!prop.vkPhysDevFeaturess.geometryShader) {
				prop.rating = 0;
			}
			bool canPresent = false;
			for(size_t j = 0; j < prop.queueFamilyProps.size(); j++){
				if(glfwGetPhysicalDevicePresentationSupport (vkinstance, physDevices[i], j)){
					canPresent = true;
					break;
				}
			}
			if(!canPresent)
				prop.rating = 0;
		}
	}
	
	std::sort(physicalDevices.begin(), physicalDevices.end(), [](VPhysDeviceProps& lhs, VPhysDeviceProps& rhs){return lhs.rating > rhs.rating;});
	
	return true;
}
bool VGlobal::choseBestDevice(){
	return choseDevice(0);
}
bool VGlobal::choseDevice(uint32_t index){
	
	if(index >= physicalDevices.size())
		return false;
	chosenDeviceId = index;
	devExtLayers.availableExtensions = physicalDevices[chosenDeviceId].availableExtensions;
	devExtLayers.availableLayers = physicalDevices[chosenDeviceId].availableLayers;
	devExtLayers.neededExtensions.clear();
	devExtLayers.neededLayers.clear();
	
	physicalDevice = physicalDevices[chosenDeviceId].physicalDevice;
	return true;
}
bool VGlobal::initializeDevice(){
	
	const float priority = 1.0f;
	
	VPhysDeviceProps& devProps = physicalDevices[chosenDeviceId];
	
	uint32_t pgcId = -1;
	uint32_t pgId = -1;
	uint32_t pId = -1;
	uint32_t gId = -1;
	uint32_t cId = -1;
	uint32_t tId = -1;
	
	uint32_t highestRate = 0;
	for(size_t j = 0; j < devProps.queueFamilyProps.size(); ++j){
		uint32_t rating = 0;
		
		bool present = glfwGetPhysicalDevicePresentationSupport (vkinstance, devProps.physicalDevice, j);
		bool graphics = devProps.queueFamilyProps[j].queueFlags & VK_QUEUE_GRAPHICS_BIT;
		bool compute = devProps.queueFamilyProps[j].queueFlags & VK_QUEUE_COMPUTE_BIT;
		bool transfer = devProps.queueFamilyProps[j].queueFlags & VK_QUEUE_TRANSFER_BIT;
		bool sparse = devProps.queueFamilyProps[j].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT;
		if(present) rating++;
		if(graphics) rating++;
		if(compute) rating++;
		if(transfer) rating++;
		if(sparse) rating++;
		
		if(present && graphics && compute && (pgcId == -1 || rating > highestRate)){
			pgcId = j;
		}
		if(present && graphics && (pgId == -1 || rating > highestRate)){
			pgId = j;
		}
		if(present && (pId == -1 || rating > highestRate)){
			pId = j;
		}
		if(graphics && (gId == -1 || rating > highestRate)){
			gId = j;
		}
		if(compute && (cId == -1 || rating > highestRate)){
			cId = j;
		}
		if(transfer && (tId == -1 && rating == 1)){//for nvidia 1 transfer queue
			tId = j;
		}
		if(rating > highestRate)
			highestRate = rating;
	}
	size_t queueFamilyCount = 0;
	if(tId != -1) 
		queueFamilyCount++;
	
	if(pgcId != -1) {
		pId = pgcId;
		gId = pgcId;
		cId = pgcId;
		queueFamilyCount++;
	}else if(pgId != -1){
		pId = pgId;
		gId = pgId;
		queueFamilyCount+=2;
	}else{
		queueFamilyCount+=3;
	}
	
	vk::DeviceQueueCreateInfo deviceQueueCreateInfos[queueFamilyCount];
	size_t currentIndex = 0;
	size_t pgcCount = 0;
	bool separateTransferQueue = false;
	if(tId != -1 && devProps.queueFamilyProps[tId].queueCount <= 1){
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), tId, 1, &priority);
		devProps.queueFamilyProps[tId].queueCount--;
		currentIndex++;
		separateTransferQueue = true;
	}
	if(pgcId != -1){
		pgcCount = devProps.queueFamilyProps[pgcId].queueCount;
		pgcCount = std::min<uint32_t>(V_MAX_PGCQUEUE_COUNT, pgcCount);
		assert(pgcCount);
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), pgcId, pgcCount, &priority);
		devProps.queueFamilyProps[pgcId].queueCount -= pgcCount;
		currentIndex+=1;
	}else if(pgId != -1){
		pgcCount = std::min<uint32_t>(devProps.queueFamilyProps[pgId].queueCount, devProps.queueFamilyProps[cId].queueCount);
		pgcCount = std::min<uint32_t>(V_MAX_PGCQUEUE_COUNT, pgcCount);
		assert(pgcCount);
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), pgId, pgcCount, &priority);
		devProps.queueFamilyProps[pgId].queueCount -= pgcCount;
		currentIndex++;
		
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), cId, pgcCount, &priority);
		devProps.queueFamilyProps[cId].queueCount -= pgcCount;
		currentIndex++;
	}else{
		pgcCount = std::min<uint32_t>(devProps.queueFamilyProps[pId].queueCount, 
				std::min<uint32_t>(devProps.queueFamilyProps[gId].queueCount, devProps.queueFamilyProps[cId].queueCount));
		pgcCount = std::min<uint32_t>(V_MAX_PGCQUEUE_COUNT, pgcCount);
		assert(pgcCount);
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), pId, pgcCount, &priority);
		devProps.queueFamilyProps[pId].queueCount -= pgcCount;
		currentIndex++;
		//TODO problem if gId == cId
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), gId, pgcCount, &priority);
		devProps.queueFamilyProps[gId].queueCount -= pgcCount;
		currentIndex++;
		
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), cId, pgcCount, &priority);
		devProps.queueFamilyProps[cId].queueCount -= pgcCount;
		currentIndex++;
	}

	printf("Create %d Present-Graphics-Compute Queues\n",pgcCount);
	
	vk::PhysicalDeviceFeatures physicalDeviceFeatures;
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
	physicalDeviceFeatures.multiDrawIndirect = VK_TRUE;
	physicalDeviceFeatures.fillModeNonSolid = VK_TRUE;// for only mesh rendering
	
	vk::DeviceCreateInfo deviceCreateInfo(vk::DeviceCreateFlags(), queueFamilyCount, deviceQueueCreateInfos, 
			devExtLayers.neededLayers.size(), devExtLayers.neededLayers.data(), 
			devExtLayers.neededExtensions.size(), devExtLayers.neededExtensions.data(),
			&physicalDeviceFeatures);
	
	V_CHECKCALL (physicalDevice.createDevice(&deviceCreateInfo, nullptr, &deviceWrapper.device), printf("Device Creation Failed\n"));
	
	currentIndex = 0;
	if(separateTransferQueue){
		VkQueue tQueue;
		vkGetDeviceQueue(deviceWrapper.device, tId, 0, &tQueue);
		
		deviceWrapper.tqueue = new VTQueue(tId, tQueue);
	}
	for(size_t i = 0; i < pgcCount; ++i){
		VPGCQueue* queue;
		if(pgcId != -1){
			queue = new VCombinedPGCQueue();
			deviceWrapper.device.getQueue(pgcId, i, &queue->presentQueue);
			queue->graphicsQueue = queue->presentQueue;
			queue->computeQueue = queue->presentQueue;
		}else if(pgId != -1){
			queue = new VPartlyPGCQueue();
			deviceWrapper.device.getQueue(pgId, i, &queue->presentQueue);
			queue->graphicsQueue = queue->presentQueue;
			deviceWrapper.device.getQueue(cId, i, &queue->computeQueue);
		}else{
			queue = new VSinglePGCQueue();
			deviceWrapper.device.getQueue(pId, i, &queue->presentQueue);
			deviceWrapper.device.getQueue(gId, i, &queue->graphicsQueue);
			deviceWrapper.device.getQueue(cId, i, &queue->computeQueue);
		}
		queue->presentQId = pId;
		queue->graphicsQId = gId;
		queue->computeQId = cId;
		queue->combinedPGQ = (pId == gId);
		queue->combinedGCQ = (gId == cId);
		deviceWrapper.pgcQueues.push_back(queue);
	}
	deviceWrapper.compQId = cId;
	deviceWrapper.presentQId = pId;
	deviceWrapper.graphQId = gId;
	deviceWrapper.transfQId = tId;
	
	return true;
}
void VGlobal::terminate(){
	
	
	vkDestroyInstance(vkinstance, nullptr);
	glfwTerminate();
	return;
}
bool VExtLayerStruct::activateLayer(const char* name){
	bool found = false;
	for(VkLayerProperties& layer : availableLayers){
		if(!strcmp(layer.layerName, name)){
			found = true;
			break;
		}
	}
	if(!found)
		return false;
	for(const char* layName : neededLayers){
		if(!strcmp(layName, name)){
			return true;
		}
	}
	neededLayers.push_back(name);
	return true;
}
bool VExtLayerStruct::activateExtension(const char* name){
	bool found = false;
	for(VkExtensionProperties& ext : availableExtensions){
		if(!strcmp(ext.extensionName, name)){
			found = true;
			break;
		}
	}
	if(!found)
		return false;
	for(const char* extName : neededExtensions){
		if(!strcmp(extName, name)){
			return true;
		}
	}
	neededExtensions.push_back(name);
	return true;
}