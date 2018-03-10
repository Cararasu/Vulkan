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
    uint64_t object, size_t location, int32_t messageCode,
    const char* pLayerPrefix, const char* pMessage, void* pUserData) {
	printf ("Layer: %s - Message: %s\n", pLayerPrefix, pMessage);
	fflush(stdout);
	return VK_TRUE;
}

PFN_vkCreateDebugReportCallbackEXT pfn_vkCreateDebugReportCallbackEXT;
PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT;


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

	uint32_t count;
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

	uint32_t devicecount = 0;
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

		uint32_t count;
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
		printf("WWW: %s\n", dev.vkPhysDevProps.deviceName);
	return true;
}
VInstance* VGlobal::createInstance(uint32_t GPUIndex){

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
	
	uint32_t chosenDeviceId = instance->deviceId;
	VPhysDeviceProps& devProps = physicalDevices[chosenDeviceId];
	instance->physicalDevice = devProps.physicalDevice;
	uint32_t pgcId = -1;
	uint32_t pgId = -1;
	uint32_t pId = -1;
	uint32_t gId = -1;
	uint32_t cId = -1;
	uint32_t tId = -1;

	uint32_t highestRate = 0;
	for (size_t j = 0; j < devProps.queueFamilyProps.size(); ++j) {
		uint32_t rating = 0;

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

		if (present && graphics && compute && (pgcId == (uint32_t) - 1 || rating > highestRate)) {
			pgcId = j;
		}
		if (present && graphics && (pgId == (uint32_t) - 1 || rating > highestRate)) {
			pgId = j;
		}
		if (present && (pId == (uint32_t) - 1 || rating > highestRate)) {
			pId = j;
		}
		if (graphics && (gId == (uint32_t) - 1 || rating > highestRate)) {
			gId = j;
		}
		if (compute && (cId == (uint32_t) - 1 || rating > highestRate)) {
			cId = j;
		}
		if (transfer && (tId == (uint32_t) - 1 && rating == 1)) { //for nvidia 1 transfer queue
			tId = j;
		}
		if (rating > highestRate)
			highestRate = rating;
	}
	size_t queueFamilyCount = 0;
	if (tId != (uint32_t) - 1)
		queueFamilyCount++;

	if (pgcId != (uint32_t) - 1) {
		pId = pgcId;
		gId = pgcId;
		cId = pgcId;
		queueFamilyCount++;
	} else if (pgId != (uint32_t) - 1) {
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
	if (tId != (uint32_t) - 1 && devProps.queueFamilyProps[tId].queueCount <= 1) {
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo (vk::DeviceQueueCreateFlags(), tId, 1, &priority);
		devProps.queueFamilyProps[tId].queueCount--;
		currentIndex++;
		separateTransferQueue = true;
	}
	if (pgcId != (uint32_t) - 1) {
		pgcCount = devProps.queueFamilyProps[pgcId].queueCount;
		pgcCount = std::min<uint32_t> (V_MAX_PGCQUEUE_COUNT, pgcCount);
		assert (pgcCount);
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo (vk::DeviceQueueCreateFlags(), pgcId, pgcCount, &priority);
		devProps.queueFamilyProps[pgcId].queueCount -= pgcCount;
		currentIndex += 1;
	} else if (pgId != (uint32_t) - 1) {
		pgcCount = std::min<uint32_t> (devProps.queueFamilyProps[pgId].queueCount, devProps.queueFamilyProps[cId].queueCount);
		pgcCount = std::min<uint32_t> (V_MAX_PGCQUEUE_COUNT, pgcCount);
		assert (pgcCount);
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo (vk::DeviceQueueCreateFlags(), pgId, pgcCount, &priority);
		devProps.queueFamilyProps[pgId].queueCount -= pgcCount;
		currentIndex++;

		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo (vk::DeviceQueueCreateFlags(), cId, pgcCount, &priority);
		devProps.queueFamilyProps[cId].queueCount -= pgcCount;
		currentIndex++;
	} else {
		pgcCount = std::min<uint32_t> (devProps.queueFamilyProps[pId].queueCount,
		                               std::min<uint32_t> (devProps.queueFamilyProps[gId].queueCount, devProps.queueFamilyProps[cId].queueCount));
		pgcCount = std::min<uint32_t> (V_MAX_PGCQUEUE_COUNT, pgcCount);
		assert (pgcCount);
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo (vk::DeviceQueueCreateFlags(), pId, pgcCount, &priority);
		devProps.queueFamilyProps[pId].queueCount -= pgcCount;
		currentIndex++;
		//TODO problem if gId == cId
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
		if (pgcId != (uint32_t) - 1) {
			queue = new VCombinedPGCQueue();
			instance->device.getQueue (pgcId, i, &queue->presentQueue);
			queue->graphicsQueue = queue->presentQueue;
			queue->computeQueue = queue->presentQueue;
		} else if (pgId != (uint32_t) - 1) {
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
