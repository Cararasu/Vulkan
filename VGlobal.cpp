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
		VCHECKCALL(vkEnumerateInstanceExtensionProperties (nullptr, &count, nullptr), "Could not get Extension-count");
	}else{
		VCHECKCALL(vkEnumerateDeviceExtensionProperties (device, nullptr, &count, nullptr), "Could not get Extension-count");
	}
	extensions->resize(count);
	if(device == VK_NULL_HANDLE){
		VCHECKCALL(vkEnumerateInstanceExtensionProperties (nullptr, &count, extensions->data()), "Could not get Extensions");
	}else{
		VCHECKCALL(vkEnumerateDeviceExtensionProperties (device, nullptr, &count, extensions->data()), "Could not get Extensions");
	}
	
	if(device == VK_NULL_HANDLE){
		VCHECKCALL(vkEnumerateInstanceLayerProperties (&count, nullptr), "Could not get Layer-count");
	}else{
		VCHECKCALL(vkEnumerateDeviceLayerProperties (device, &count, nullptr), "Could not get Layer-count");
	}
	layers->resize(count);
	if(device == VK_NULL_HANDLE){
		VCHECKCALL(vkEnumerateInstanceLayerProperties (&count, layers->data()), "Could not get Layers");
	}else{
		VCHECKCALL(vkEnumerateDeviceLayerProperties (device, &count, layers->data()), "Could not get Layers");
	}
	for(VkLayerProperties& layerProp : *layers){
		
		if(device == VK_NULL_HANDLE){
			VCHECKCALL(vkEnumerateInstanceExtensionProperties (layerProp.layerName, &count, nullptr), "Could not get Layer-count");
		}else{
			VCHECKCALL(vkEnumerateDeviceExtensionProperties (device, layerProp.layerName, &count, nullptr), "Could not get Layer-count");
		}
		VkExtensionProperties extensionArray[count];
		if(device == VK_NULL_HANDLE){
			VCHECKCALL(vkEnumerateInstanceExtensionProperties (layerProp.layerName, &count, extensionArray), "Could not get Extension-count");
		}else{
			VCHECKCALL(vkEnumerateDeviceExtensionProperties (device, layerProp.layerName, &count, extensionArray), "Could not get Layer-count");
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
	
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = appName;
	appInfo.applicationVersion = VK_MAKE_VERSION (1, 0, 0);
	appInfo.pEngineName = engineName;
	appInfo.engineVersion = VK_MAKE_VERSION (1, 0, 0);
	appInfo.apiVersion = VK_MAKE_VERSION (1, 0, 61);

	VkInstanceCreateInfo instanceCreateInfo;
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &appInfo;//can be null
	instanceCreateInfo.enabledLayerCount = instExtLayers.neededLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = instExtLayers.neededLayers.data();
	instanceCreateInfo.enabledExtensionCount = instExtLayers.neededExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = instExtLayers.neededExtensions.data();//maybe we need to add some things in the future
	
	VCHECKCALL(vkCreateInstance (&instanceCreateInfo, nullptr, &vkinstance), "Instance Creation Failed\n");
	
	uint32_t devicecount = 0;
	VCHECKCALL(vkEnumeratePhysicalDevices (vkinstance, &devicecount, nullptr), "Get physical-device count Failed\n");
	VkPhysicalDevice physDevices[devicecount];
	physicalDevices.resize(devicecount);
	VCHECKCALL(vkEnumeratePhysicalDevices (vkinstance, &devicecount, physDevices), "Get physical-devicec Failed\n");

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
			for(int j = 0; j < prop.queueFamilyProps.size(); j++){
				if(glfwGetPhysicalDevicePresentationSupport (vkinstance, physDevices[i], j)){
					canPresent = true;
					break;
				}
			}
			if(!canPresent)
				prop.rating = 0;
		}
		int highestratequeue = 0;
	}
	
	std::sort(physicalDevices.begin(), physicalDevices.end(), [](VPhysDeviceProps& lhs, VPhysDeviceProps& rhs){return lhs.rating < rhs.rating;});
	
	return true;
}
bool VGlobal::choseBestDevice(){
	return choseDevice(0);
}
bool VGlobal::choseDevice(uint32_t index){
	
	if(index >= physicalDevices.size())
		return false;
	chosenDevice = index;
	devExtLayers.availableExtensions = physicalDevices[chosenDevice].availableExtensions;
	devExtLayers.availableLayers = physicalDevices[chosenDevice].availableLayers;
	devExtLayers.neededExtensions.clear();
	devExtLayers.neededLayers.clear();
	
	physicalDevice = physicalDevices[chosenDevice].physicalDevice;
	return true;
}
bool VGlobal::initializeDevice(){
	
	const float priority = 1.0f;
	
	VPhysDeviceProps& devProps = physicalDevices[chosenDevice];
	
	uint32_t pgcId = -1;
	uint32_t pgId = -1;
	uint32_t pId = -1;
	uint32_t gId = -1;
	uint32_t cId = -1;
	uint32_t tId = -1;
	
	uint32_t highestRate = 0;
	for(int j = 0; j < devProps.queueFamilyProps.size(); ++j){
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
		if(transfer && (tId == -1 || rating == 1)){//for nvidia 1 transfer queue
			tId = j;
		}
		if(rating > highestRate)
			highestRate = rating;
	}
	assert(pId != -1 && gId != -1 && cId != -1);
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
	
	VkDeviceQueueCreateInfo deviceQueueCreateInfos[queueFamilyCount];
	size_t currentIndex = 0;
	size_t pgcCount = 0;
	bool separateTransferQueue = false;;
	if(tId != -1 && !((pId == tId || gId == tId || cId == tId) && deviceQueueCreateInfos[tId].queueCount <= 1)){
		deviceQueueCreateInfos[currentIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfos[currentIndex].pNext = nullptr;
		deviceQueueCreateInfos[currentIndex].flags = 0;
		deviceQueueCreateInfos[currentIndex].queueFamilyIndex = tId;
		deviceQueueCreateInfos[currentIndex].queueCount = 1;
		deviceQueueCreateInfos[currentIndex].pQueuePriorities = &priority;
		devProps.queueFamilyProps[tId].queueCount--;
		currentIndex+=1;
		separateTransferQueue = true;
	}
	if(pgcId != -1){
		pgcCount = std::min<uint32_t>(V_MAX_PGCQUEUE_COUNT, devProps.queueFamilyProps[pgcId].queueCount);
		assert(pgcCount);
		deviceQueueCreateInfos[currentIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfos[currentIndex].pNext = nullptr;
		deviceQueueCreateInfos[currentIndex].flags = 0;
		deviceQueueCreateInfos[currentIndex].queueFamilyIndex = pgcId;
		deviceQueueCreateInfos[currentIndex].queueCount = pgcCount;
		deviceQueueCreateInfos[currentIndex].pQueuePriorities = &priority;
		devProps.queueFamilyProps[pgcId].queueCount -= pgcCount;
		currentIndex+=1;
	}else if(pgId != -1){
		pgcCount = std::min<uint32_t>(V_MAX_PGCQUEUE_COUNT, 
				std::min<uint32_t>(devProps.queueFamilyProps[pgId].queueCount, devProps.queueFamilyProps[cId].queueCount));
		assert(pgcCount);
		deviceQueueCreateInfos[currentIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfos[currentIndex].pNext = nullptr;
		deviceQueueCreateInfos[currentIndex].flags = 0;
		deviceQueueCreateInfos[currentIndex].queueFamilyIndex = pgId;
		deviceQueueCreateInfos[currentIndex].queueCount = pgcCount;
		deviceQueueCreateInfos[currentIndex].pQueuePriorities = &priority;
		devProps.queueFamilyProps[pgId].queueCount -= pgcCount;
		currentIndex+=1;
		
		deviceQueueCreateInfos[currentIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfos[currentIndex].pNext = nullptr;
		deviceQueueCreateInfos[currentIndex].flags = 0;
		deviceQueueCreateInfos[currentIndex].queueFamilyIndex = cId;
		deviceQueueCreateInfos[currentIndex].queueCount = pgcCount;
		deviceQueueCreateInfos[currentIndex].pQueuePriorities = &priority;
		devProps.queueFamilyProps[cId].queueCount -= pgcCount;
		currentIndex+=1;
	}else{
		pgcCount = std::min<uint32_t>(V_MAX_PGCQUEUE_COUNT, 
				std::min<uint32_t>(devProps.queueFamilyProps[pId].queueCount, 
				std::min<uint32_t>(devProps.queueFamilyProps[gId].queueCount, devProps.queueFamilyProps[cId].queueCount)));
		assert(pgcCount);
		deviceQueueCreateInfos[currentIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfos[currentIndex].pNext = nullptr;
		deviceQueueCreateInfos[currentIndex].flags = 0;
		deviceQueueCreateInfos[currentIndex].queueFamilyIndex = pId;
		deviceQueueCreateInfos[currentIndex].queueCount = pgcCount;
		deviceQueueCreateInfos[currentIndex].pQueuePriorities = &priority;
		devProps.queueFamilyProps[pId].queueCount -= pgcCount;
		currentIndex+=1;
		//TODO problem if gId == cId
		deviceQueueCreateInfos[currentIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfos[currentIndex].pNext = nullptr;
		deviceQueueCreateInfos[currentIndex].flags = 0;
		deviceQueueCreateInfos[currentIndex].queueFamilyIndex = gId;
		deviceQueueCreateInfos[currentIndex].queueCount = pgcCount;
		deviceQueueCreateInfos[currentIndex].pQueuePriorities = &priority;
		devProps.queueFamilyProps[gId].queueCount -= pgcCount;
		currentIndex+=1;
		
		deviceQueueCreateInfos[currentIndex].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfos[currentIndex].pNext = nullptr;
		deviceQueueCreateInfos[currentIndex].flags = 0;
		deviceQueueCreateInfos[currentIndex].queueFamilyIndex = cId;
		deviceQueueCreateInfos[currentIndex].queueCount = pgcCount;
		deviceQueueCreateInfos[currentIndex].pQueuePriorities = &priority;
		devProps.queueFamilyProps[cId].queueCount -= pgcCount;
		currentIndex+=1;
	}

	printf("Create %d Present-Graphics-Compute Queues\n",pgcCount);
	
	VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = queueFamilyCount;
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
	deviceCreateInfo.enabledLayerCount = devExtLayers.neededLayers.size();
	deviceCreateInfo.ppEnabledLayerNames = devExtLayers.neededLayers.data();
	deviceCreateInfo.enabledExtensionCount = devExtLayers.neededExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = devExtLayers.neededExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
	
	VCHECKCALL (vkCreateDevice (physicalDevice, &deviceCreateInfo, nullptr, &vDevice.device), "Device Creation Failed\n");
	
	currentIndex = 0;
	if(separateTransferQueue){
		VkQueue tQueue;
		vkGetDeviceQueue(vDevice.device, tId, 0, &tQueue);
	}
	for(size_t i = 0; i < pgcCount; ++i){
		VPGCQueue* queue;
		if(pgcId != -1){
			queue = new VCombinedPGCQueue();
			vkGetDeviceQueue(vDevice.device, pgcId, i, &queue->presentQueue);
			queue->graphicsQueue = queue->presentQueue;
			queue->computeQueue = queue->presentQueue;
		}else if(pgId != -1){
			queue = new VPartlyPGCQueue();
			vkGetDeviceQueue(vDevice.device, pgId, i, &queue->presentQueue);
			queue->graphicsQueue = queue->presentQueue;
			vkGetDeviceQueue(vDevice.device, cId, i, &queue->computeQueue);
		}else{
			queue = new VSinglePGCQueue();
			vkGetDeviceQueue(vDevice.device, pId, i, &queue->presentQueue);
			vkGetDeviceQueue(vDevice.device, gId, i, &queue->graphicsQueue);
			vkGetDeviceQueue(vDevice.device, cId, i, &queue->computeQueue);
		}
		queue->presentQId = pId;
		queue->graphicsQId = gId;
		queue->computeQId = cId;
		queue->combinedPGQ = (pId == gId);
		queue->combinedGCQ = (gId == cId);
		queue->presentCanTransfer = (tId == pId);
		queue->graphicsCanTransfer = (tId == gId);
		queue->computeCanTransfer = (tId == cId);
		vDevice.pgcQueues.push_back(queue);
	}
	
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