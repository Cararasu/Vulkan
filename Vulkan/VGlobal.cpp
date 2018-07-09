#include "VGlobal.h"
#include "VHeader.h"
#include <stdio.h>
#include <algorithm>
#include <assert.h>
#include <algorithm>
#include "PipelineModule.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "VInstance.h"

VGlobal global;

VkBool32 VKAPI_PTR debugLogger (
    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
    u64 object, size_t location, int32_t messageCode,
    const char* pLayerPrefix, const char* pMessage, void* pUserData);

extern PFN_vkCreateDebugReportCallbackEXT pfn_vkCreateDebugReportCallbackEXT;
extern PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT;


bool operator== (vk::LayerProperties& lhs, vk::LayerProperties& rhs) {
	return !strcmp (lhs.layerName, rhs.layerName);
}
bool operator== (vk::ExtensionProperties& lhs, vk::ExtensionProperties& rhs) {
	return !strcmp (lhs.extensionName, rhs.extensionName);
}

void addExtension (std::vector<vk::ExtensionProperties>* propList, vk::ExtensionProperties prop) {
	bool found = false;
	for (vk::ExtensionProperties& it : *propList) {
		if (it == prop) {
			found = true;
			break;
		}
	}
	if (!found) {
		propList->push_back (prop);
	}
}

void gatherExtLayer (vk::PhysicalDevice device, std::vector<vk::LayerProperties>* layers, std::vector<vk::ExtensionProperties>* extensions) {

	u32 count;
	if (!device) {
		V_CHECKCALL (vk::enumerateInstanceExtensionProperties (nullptr, &count, nullptr), printf ("Could not get Extension-count"));
	} else {
		V_CHECKCALL (device.enumerateDeviceExtensionProperties (nullptr, &count, nullptr), printf ("Could not get Extension-count"));
	}
	extensions->resize (count);
	if (!device) {
		V_CHECKCALL (vk::enumerateInstanceExtensionProperties (nullptr, &count, extensions->data()), printf ("Could not get Extensions"));
	} else {
		V_CHECKCALL (device.enumerateDeviceExtensionProperties (nullptr, &count, extensions->data()), printf ("Could not get Extensions"));
	}

	if (!device) {
		V_CHECKCALL (vk::enumerateInstanceLayerProperties (&count, nullptr), printf ("Could not get Layer-count"));
	} else {
		V_CHECKCALL (device.enumerateDeviceLayerProperties (&count, nullptr), printf ("Could not get Layer-count"));
	}
	layers->resize (count);
	if (!device) {
		V_CHECKCALL (vk::enumerateInstanceLayerProperties (&count, layers->data()), printf ("Could not get Layers"));
	} else {
		V_CHECKCALL (device.enumerateDeviceLayerProperties (&count, layers->data()), printf ("Could not get Layers"));
	}
	for (vk::LayerProperties& layerProp : *layers) {

		if (!device) {
			V_CHECKCALL (vk::enumerateInstanceExtensionProperties (layerProp.layerName, &count, nullptr), printf ("Could not get Extension-count"));
		} else {
			V_CHECKCALL (device.enumerateDeviceExtensionProperties (layerProp.layerName, &count, nullptr), printf ("Could not get Extension-count"));
		}
		vk::ExtensionProperties extensionArray[count];
		if (!device) {
			V_CHECKCALL (vk::enumerateInstanceExtensionProperties (layerProp.layerName, &count, extensionArray), printf ("Could not get Extensions"));
		} else {
			V_CHECKCALL (device.enumerateDeviceExtensionProperties (layerProp.layerName, &count, extensionArray), printf ("Could not get Extensions"));
		}
		for (size_t i = 0; i < count; ++i) {
			addExtension (extensions, extensionArray[i]);
		}
	}
}

bool VGlobal::preInitialize() {

	if (glfwVulkanSupported()) {
		printf ("Vulkan supported\n");
		// Vulkan is available, at least for compute
	} else {

		printf ("Vulkan not supported\n");
		return false;
	}
	gatherExtLayer (vk::PhysicalDevice(), &extLayers.availableLayers, &extLayers.availableExtensions);

	return true;
}
bool VGlobal::initializeInstance (const char* appName, const char* engineName) {

	vk::ApplicationInfo appInfo (appName, VK_MAKE_VERSION (1, 0, 0), engineName, VK_MAKE_VERSION (1, 0, 0), VK_MAKE_VERSION (1, 0, 61));

	vk::InstanceCreateInfo instanceCreateInfo (vk::InstanceCreateFlags(), &appInfo,
	        extLayers.neededLayers.size(), extLayers.neededLayers.data(),
	        extLayers.neededExtensions.size(), extLayers.neededExtensions.data());
	V_CHECKCALL (vk::createInstance (&instanceCreateInfo, nullptr, &vkinstance), printf ("Instance Creation Failed\n"));

	pfn_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) glfwGetInstanceProcAddress (vkinstance, "vkCreateDebugReportCallbackEXT");
	pfn_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) glfwGetInstanceProcAddress (vkinstance, "vkDestroyDebugReportCallbackEXT");

	vk::DebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo (
	    vk::DebugReportFlagsEXT (
	        //vk::DebugReportFlagBitsEXT::eInformation |
	        vk::DebugReportFlagBitsEXT::eWarning |
	        vk::DebugReportFlagBitsEXT::ePerformanceWarning |
	        vk::DebugReportFlagBitsEXT::eError |
	        vk::DebugReportFlagBitsEXT::eDebug),
	    &debugLogger,
	    nullptr
	);
	pfn_vkCreateDebugReportCallbackEXT (vkinstance, reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT*> (&debugReportCallbackCreateInfo), nullptr, reinterpret_cast<VkDebugReportCallbackEXT*> (&debugReportCallbackEXT));

	u32 devicecount = 0;
	V_CHECKCALL (vkinstance.enumeratePhysicalDevices (&devicecount, nullptr), printf ("Get physical-device count Failed\n"));

	vk::PhysicalDevice physDevices[devicecount];
	physicalDevices.resize (devicecount);
	V_CHECKCALL (vkinstance.enumeratePhysicalDevices (&devicecount, physDevices), printf ("Get physical-devicec Failed\n"));
	
	for (size_t i = 0; i < devicecount; i++) {
		VPhysDeviceProps& prop = physicalDevices[i];
		
		prop.physicalDevice = physDevices[i];
		physDevices[i].getProperties (&prop.vkPhysDevProps);
		physDevices[i].getFeatures (&prop.vkPhysDevFeatures);

		gatherExtLayer (physDevices[i], &prop.availableLayers, &prop.availableExtensions);

		printf("Physical Device %s\n", prop.vkPhysDevProps.deviceName);
		
		vk::PhysicalDeviceMemoryProperties memProperties;
		prop.physicalDevice.getMemoryProperties(&memProperties);
		
		for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			printf("Memory %d\n", i);
			printf("\tHeap Index %d\n", memProperties.memoryTypes[i].heapIndex);
			if(memProperties.memoryTypes[i].propertyFlags | vk::MemoryPropertyFlagBits::eDeviceLocal)
				printf("\tDevice Local\n");
			if(memProperties.memoryTypes[i].propertyFlags | vk::MemoryPropertyFlagBits::eHostVisible)
				printf("\tHost Visible\n");
			if(memProperties.memoryTypes[i].propertyFlags | vk::MemoryPropertyFlagBits::eHostCoherent)
				printf("\tHost Coherent\n");
			if(memProperties.memoryTypes[i].propertyFlags | vk::MemoryPropertyFlagBits::eHostCached)
				printf("\tHost Cached\n");
			if(memProperties.memoryTypes[i].propertyFlags | vk::MemoryPropertyFlagBits::eLazilyAllocated)
				printf("\tLazily Allocated\n");
		}
		for(uint32_t i = 0; i < memProperties.memoryHeapCount; i++) {
			printf("Heap %d\n", i);
			printf("\tSize %lld\n", memProperties.memoryHeaps[i].size);
			if(memProperties.memoryHeaps[i].flags | vk::MemoryHeapFlagBits::eDeviceLocal)
				printf("\tDevice Local\n");
		}


		u32 count;
		prop.physicalDevice.getQueueFamilyProperties (&count, nullptr);
		prop.queueFamilyProps.resize (count);
		prop.physicalDevice.getQueueFamilyProperties (&count, prop.queueFamilyProps.data());

		{
			//rate a physicalDevice
			prop.rating = 0;
			if (prop.vkPhysDevProps.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
				prop.rating += 1000;
			}
			// Maximum possible size of textures affects graphics quality
			prop.rating += prop.vkPhysDevProps.limits.maxImageDimension2D;
			// Application can't function without geometry shaders
			if (!prop.vkPhysDevFeatures.geometryShader) {
				prop.rating = 0;
			}
			bool canPresent = false;
			for (size_t j = 0; j < prop.queueFamilyProps.size(); j++) {
				if (glfwGetPhysicalDevicePresentationSupport (vkinstance, physDevices[i], j)) {
					canPresent = true;
					break;
				}
			}
			if (!canPresent)
				prop.rating = 0;
		}
	}

	std::sort (physicalDevices.begin(), physicalDevices.end(), [] (VPhysDeviceProps & lhs, VPhysDeviceProps & rhs) {
		return lhs.rating > rhs.rating;
	});
	for(auto& dev : physicalDevices)
/**
 * @brief 
 * @param appName
 * @param engineName
 * @return 
 */
		printf("WWW: %s\n", dev.vkPhysDevProps.deviceName);
	return true;
}
VInstance* VGlobal::createInstance(u32 GPUIndex){

	if (GPUIndex >= physicalDevices.size())
		return nullptr;
	VInstance* instance = new VInstance(physicalDevices[GPUIndex].physicalDevice, GPUIndex);
	instance->extLayers.availableExtensions = physicalDevices[GPUIndex].availableExtensions;
	instance->extLayers.availableLayers = physicalDevices[GPUIndex].availableLayers;
	instance->extLayers.neededExtensions.clear();
	instance->extLayers.neededLayers.clear();
	return instance;
}
bool VGlobal::initializeDevice(VInstance* instance) {

	const float priority = 1.0f;
	
	u32 chosenDeviceId = instance->deviceId;
	VPhysDeviceProps& devProps = physicalDevices[chosenDeviceId];
	instance->physicalDevice = devProps.physicalDevice;
	u32 pgcId = -1;
	u32 pgId = -1;
	u32 pId = -1;
	u32 gId = -1;
	u32 cId = -1;
	u32 tId = -1;

	u32 highestRate = 0;
	for (size_t j = 0; j < devProps.queueFamilyProps.size(); ++j) {
		u32 rating = 0;

		bool present = glfwGetPhysicalDevicePresentationSupport (vkinstance, devProps.physicalDevice, j);
		bool graphics = (bool) vk::QueueFlags (devProps.queueFamilyProps[j].queueFlags & vk::QueueFlagBits::eGraphics);
		bool compute = (bool) vk::QueueFlags (devProps.queueFamilyProps[j].queueFlags & vk::QueueFlagBits::eCompute);
		bool transfer = (bool) vk::QueueFlags (devProps.queueFamilyProps[j].queueFlags & vk::QueueFlagBits::eTransfer);
		bool sparse = (bool) vk::QueueFlags (devProps.queueFamilyProps[j].queueFlags & vk::QueueFlagBits::eSparseBinding);
		if (present) rating++;
		if (graphics) rating++;
		if (compute) rating++;
		if (transfer) rating++;
		if (sparse) rating++;

		if (present && graphics && compute && (pgcId == (u32) - 1 || rating > highestRate)) {
			pgcId = j;
		}
		if (present && graphics && (pgId == (u32) - 1 || rating > highestRate)) {
			pgId = j;
		}
		if (present && (pId == (u32) - 1 || rating > highestRate)) {
			pId = j;
		}
		if (graphics && (gId == (u32) - 1 || rating > highestRate)) {
			gId = j;
		}
		if (compute && (cId == (u32) - 1 || rating > highestRate)) {
			cId = j;
		}
		if (transfer && (tId == (u32) - 1 && rating == 1)) { //for nvidia 1 transfer queue
			tId = j;
		}
		if (rating > highestRate)
			highestRate = rating;
	}
	size_t queueFamilyCount = 0;
	if (tId != (u32) - 1)
		queueFamilyCount++;

	if (pgcId != (u32) - 1) {
		pId = pgcId;
		gId = pgcId;
		cId = pgcId;
		queueFamilyCount++;
	} else if (pgId != (u32) - 1) {
		pId = pgId;
		gId = pgId;
		queueFamilyCount += 2;
	} else {
		queueFamilyCount += 3;
	}

	vk::DeviceQueueCreateInfo deviceQueueCreateInfos[queueFamilyCount];
	size_t currentIndex = 0;
	size_t pgcCount = 0;
	bool separateTransferQueue = false;
	if (tId != (u32) - 1 && devProps.queueFamilyProps[tId].queueCount <= 1) {
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo (vk::DeviceQueueCreateFlags(), tId, 1, &priority);
		devProps.queueFamilyProps[tId].queueCount--;
		currentIndex++;
		separateTransferQueue = true;
	}
	if (pgcId != (u32) - 1) {
		pgcCount = devProps.queueFamilyProps[pgcId].queueCount;
		pgcCount = std::min<u32> (V_MAX_PGCQUEUE_COUNT, pgcCount);
		assert (pgcCount);
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo (vk::DeviceQueueCreateFlags(), pgcId, pgcCount, &priority);
		devProps.queueFamilyProps[pgcId].queueCount -= pgcCount;
		currentIndex += 1;
	} else if (pgId != (u32) - 1) {
		pgcCount = std::min<u32> (devProps.queueFamilyProps[pgId].queueCount, devProps.queueFamilyProps[cId].queueCount);
		pgcCount = std::min<u32> (V_MAX_PGCQUEUE_COUNT, pgcCount);
		assert (pgcCount);
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo (vk::DeviceQueueCreateFlags(), pgId, pgcCount, &priority);
		devProps.queueFamilyProps[pgId].queueCount -= pgcCount;
		currentIndex++;

		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo (vk::DeviceQueueCreateFlags(), cId, pgcCount, &priority);
		devProps.queueFamilyProps[cId].queueCount -= pgcCount;
		currentIndex++;
	} else {
		pgcCount = std::min<u32> (devProps.queueFamilyProps[pId].queueCount,
		                               std::min<u32> (devProps.queueFamilyProps[gId].queueCount, devProps.queueFamilyProps[cId].queueCount));
		pgcCount = std::min<u32> (V_MAX_PGCQUEUE_COUNT, pgcCount);
		assert (pgcCount);
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo (vk::DeviceQueueCreateFlags(), pId, pgcCount, &priority);
		devProps.queueFamilyProps[pId].queueCount -= pgcCount;
		currentIndex++;
		//@TODO problem if gId == cId
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo (vk::DeviceQueueCreateFlags(), gId, pgcCount, &priority);
		devProps.queueFamilyProps[gId].queueCount -= pgcCount;
		currentIndex++;

		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo (vk::DeviceQueueCreateFlags(), cId, pgcCount, &priority);
		devProps.queueFamilyProps[cId].queueCount -= pgcCount;
		currentIndex++;
	}

	printf ("Create %d Present-Graphics-Compute Queues\n", pgcCount);

	vk::PhysicalDeviceFeatures physicalDeviceFeatures;
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
	physicalDeviceFeatures.multiDrawIndirect = VK_TRUE;
	physicalDeviceFeatures.fillModeNonSolid = VK_TRUE;// for only mesh rendering

	vk::DeviceCreateInfo deviceCreateInfo (vk::DeviceCreateFlags(), queueFamilyCount, deviceQueueCreateInfos,
	                                       instance->extLayers.neededLayers.size(), instance->extLayers.neededLayers.data(),
	                                       instance->extLayers.neededExtensions.size(), instance->extLayers.neededExtensions.data(),
	                                       &physicalDeviceFeatures);

	V_CHECKCALL (instance->physicalDevice.createDevice (&deviceCreateInfo, nullptr, &instance->device), printf ("Device Creation Failed\n"));

	currentIndex = 0;
	if (separateTransferQueue) {
		vk::Queue tQueue;
		instance->device.getQueue (tId, 0, &tQueue);
		instance->tqueue = new VTQueue (tId, tQueue);
	}
	for (size_t i = 0; i < pgcCount; ++i) {
		VPGCQueue* queue;
		if (pgcId != (u32) - 1) {
			queue = new VCombinedPGCQueue();
			instance->device.getQueue (pgcId, i, &queue->presentQueue);
			queue->graphicsQueue = queue->presentQueue;
			queue->computeQueue = queue->presentQueue;
		} else if (pgId != (u32) - 1) {
			queue = new VPartlyPGCQueue();
			instance->device.getQueue (pgId, i, &queue->presentQueue);
			queue->graphicsQueue = queue->presentQueue;
			instance->device.getQueue (cId, i, &queue->computeQueue);
		} else {
			queue = new VSinglePGCQueue();
			instance->device.getQueue (pId, i, &queue->presentQueue);
			instance->device.getQueue (gId, i, &queue->graphicsQueue);
			instance->device.getQueue (cId, i, &queue->computeQueue);
		}
		queue->presentQId = pId;
		queue->graphicsQId = gId;
		queue->computeQId = cId;
		queue->combinedPGQ = (pId == gId);
		queue->combinedGCQ = (gId == cId);
		instance->pgcQueues.push_back (queue);
	}
	instance->compQId = cId;
	instance->presentQId = pId;
	instance->graphQId = gId;
	instance->transfQId = tId;

	return true;
}



void VGlobal::terminate() {
	
	pfn_vkDestroyDebugReportCallbackEXT (vkinstance, debugReportCallbackEXT, nullptr);

	vkinstance.destroy (nullptr);
	glfwTerminate();
	return;
}
bool VExtLayerStruct::activateLayer (const char* name) {
	bool found = false;
	for (vk::LayerProperties& layer : availableLayers) {
		if (!strcmp (layer.layerName, name)) {
			found = true;
			break;
		}
	}
	if (!found)
		return false;
	for (const char* layName : neededLayers) {
		if (!strcmp (layName, name)) {
			return true;
		}
	}
	neededLayers.push_back (name);
	return true;
}
bool VExtLayerStruct::activateExtension (const char* name) {
	bool found = false;
	for (vk::ExtensionProperties& ext : availableExtensions) {
		if (!strcmp (ext.extensionName, name)) {
			found = true;
			break;
		}
	}
	if (!found)
		return false;
	for (const char* extName : neededExtensions) {
		if (!strcmp (extName, name)) {
			return true;
		}
	}
	neededExtensions.push_back (name);
	return true;
}
