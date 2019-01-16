#include "VInstance.h"
#include "VWindow.h"
#include "VBufferStorage.h"
#include <fstream>
#include "VContext.h"
#include "VModel.h"
#include "VRenderBundle.h"
#include "VResourceManager.h"
#include "Specialization/VMainBundle.h"
#include "VRenderer.h"
#include "VSampler.h"

#include "Specialization/VSpecializations.h"

bool operator== ( vk::LayerProperties& lhs, vk::LayerProperties& rhs ) {
	return !strcmp ( lhs.layerName, rhs.layerName );
}
bool operator== ( vk::ExtensionProperties& lhs, vk::ExtensionProperties& rhs ) {
	return !strcmp ( lhs.extensionName, rhs.extensionName );
}

void addExtension ( std::vector<vk::ExtensionProperties>* propList, vk::ExtensionProperties prop ) {
	bool found = false;
	for ( vk::ExtensionProperties& it : *propList ) {
		if ( it == prop ) {
			found = true;
			break;
		}
	}
	if ( !found ) {
		propList->push_back ( prop );
	}
}

void gatherExtLayer ( vk::PhysicalDevice device, std::vector<vk::LayerProperties>* layers, std::vector<vk::ExtensionProperties>* extensions ) {

	u32 count;
	if ( !device ) {
		V_CHECKCALL ( vk::enumerateInstanceExtensionProperties ( nullptr, &count, nullptr ), v_logger.log<LogLevel::eError> ( "Could not get Extension-count" ) );
	} else {
		V_CHECKCALL ( device.enumerateDeviceExtensionProperties ( nullptr, &count, nullptr ), v_logger.log<LogLevel::eError> ( "Could not get Extension-count" ) );
	}
	extensions->resize ( count );
	if ( !device ) {
		V_CHECKCALL ( vk::enumerateInstanceExtensionProperties ( nullptr, &count, extensions->data() ), v_logger.log<LogLevel::eError> ( "Could not get Extensions" ) );
	} else {
		V_CHECKCALL ( device.enumerateDeviceExtensionProperties ( nullptr, &count, extensions->data() ), v_logger.log<LogLevel::eError> ( "Could not get Extensions" ) );
	}

	if ( !device ) {
		V_CHECKCALL ( vk::enumerateInstanceLayerProperties ( &count, nullptr ), v_logger.log<LogLevel::eError> ( "Could not get Layer-count" ) );
	} else {
		V_CHECKCALL ( device.enumerateDeviceLayerProperties ( &count, nullptr ), v_logger.log<LogLevel::eError> ( "Could not get Layer-count" ) );
	}
	layers->resize ( count );
	if ( !device ) {
		V_CHECKCALL ( vk::enumerateInstanceLayerProperties ( &count, layers->data() ), v_logger.log<LogLevel::eError> ( "Could not get Layers" ) );
	} else {
		V_CHECKCALL ( device.enumerateDeviceLayerProperties ( &count, layers->data() ), v_logger.log<LogLevel::eError> ( "Could not get Layers" ) );
	}
	for ( vk::LayerProperties& layerProp : *layers ) {

		if ( !device ) {
			V_CHECKCALL ( vk::enumerateInstanceExtensionProperties ( layerProp.layerName, &count, nullptr ), v_logger.log<LogLevel::eError> ( "Could not get Extension-count" ) );
		} else {
			V_CHECKCALL ( device.enumerateDeviceExtensionProperties ( layerProp.layerName, &count, nullptr ), v_logger.log<LogLevel::eError> ( "Could not get Extension-count" ) );
		}
		vk::ExtensionProperties extensionArray[count];
		if ( !device ) {
			V_CHECKCALL ( vk::enumerateInstanceExtensionProperties ( layerProp.layerName, &count, extensionArray ), v_logger.log<LogLevel::eError> ( "Could not get Extensions" ) );
		} else {
			V_CHECKCALL ( device.enumerateDeviceExtensionProperties ( layerProp.layerName, &count, extensionArray ), v_logger.log<LogLevel::eError> ( "Could not get Extensions" ) );
		}
		for ( size_t i = 0; i < count; ++i ) {
			addExtension ( extensions, extensionArray[i] );
		}
	}
}
bool VExtLayerStruct::activateLayer ( String name ) {
	bool found = false;
	for ( vk::LayerProperties& layer : availableLayers ) {
		if ( layer.layerName == name ) {
			found = true;
			break;
		}
	}
	if ( !found ) {
		v_logger.log<LogLevel::eWarn> ( "Layer %s not available", name.cstr );
		return false;
	}
	for ( String& layName : neededLayers ) {
		if ( layName == name ) {
			return true;
		}
	}
	neededLayers.push_back ( name );
	v_logger.log<LogLevel::eInfo> ( "Layer %s added", name.cstr );
	return true;
}
bool VExtLayerStruct::activateExtension ( String name ) {
	bool found = false;
	for ( vk::ExtensionProperties& ext : availableExtensions ) {
		if ( ext.extensionName == name ) {
			found = true;
			break;
		}
	}
	if ( !found ) {
		v_logger.log<LogLevel::eWarn> ( "Extension %s not available", name.cstr );
		return false;
	}
	for ( String& extName : neededExtensions ) {
		if ( extName == name ) {
			return true;
		}
	}
	neededExtensions.push_back ( name );
	v_logger.log<LogLevel::eInfo> ( "Extension %s added", name.cstr );
	return true;
}


Instance* create_instance ( String name ) {
	if ( name == "Vulkan" ) {
		glfwInit();
		VInstance* instance = new VInstance();
		return instance;
	}
	return nullptr;
}

void destroy_instance ( Instance* instance ) {
	if ( VInstance* vulkan_instance = dynamic_cast<VInstance*> ( instance ) ) {
		delete vulkan_instance;
		glfwTerminate();
	} else {
		delete instance;
	}
}
VideoMode glfw_to_videomode ( GLFWvidmode glfw_videomode ) {
	return {Extent2D<s32> ( glfw_videomode.width, glfw_videomode.height ), ( u32 ) glfw_videomode.refreshRate};
}

VMonitor::VMonitor ( GLFWmonitor* monitor ) : monitor ( monitor ) {
	this->name = glfwGetMonitorName ( monitor );

	int xpos, ypos;
	glfwGetMonitorPos ( monitor, &xpos, &ypos );
	this->offset.x = xpos;
	this->offset.y = ypos;


	int count;
	const GLFWvidmode* videomode = glfwGetVideoModes ( monitor, &count );
	videomodes.resize ( count );
	for ( int i = 0; i < count; ++i ) {
		if ( videomode[i].redBits == 8 || videomode[i].greenBits == 8 || videomode[i].blueBits == 8 ) {
			videomodes[i] = glfw_to_videomode ( videomode[i] );
			this->extend = max_extend ( this->extend, videomodes[i].extend );
			v_logger.log<LogLevel::eTrace> ("\tVideomode %dx%d r%dg%db%d %dHz", videomode[i].width, videomode[i].height, videomode[i].redBits, videomode[i].greenBits, videomode[i].blueBits, videomode[i].refreshRate);
		}
	}
	v_logger.log<LogLevel::eDebug> ( "Monitor %s: %dx%d %dx%d", this->name, this->offset.x, this->offset.y, this->extend.x, this->extend.y );
}
VMonitor::~VMonitor() {

}
VideoMode VMonitor::current_mode() {
	return glfw_to_videomode ( *glfwGetVideoMode ( this->monitor ) );
}
#ifndef NDEBUG
	VkBool32 VKAPI_PTR debugLogger (
		VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
		u64 object, size_t location, int32_t messageCode,
		const char* pLayerPrefix, const char* pMessage, void* pUserData ) {
		
		v_logger.log<LogLevel::eTrace> ( "Layer: %s - Message: %s", pLayerPrefix, pMessage );
		return VK_TRUE;
	}
	VkBool32 VKAPI_PTR infoLogger (
		VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
		u64 object, size_t location, int32_t messageCode,
		const char* pLayerPrefix, const char* pMessage, void* pUserData ) {
		
		v_logger.log<LogLevel::eDebug> ( "Layer: %s - Message: %s", pLayerPrefix, pMessage );
		return VK_TRUE;
	}
	VkBool32 VKAPI_PTR errorLogger (
		VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
		u64 object, size_t location, int32_t messageCode,
		const char* pLayerPrefix, const char* pMessage, void* pUserData ) {
		
		v_logger.log<LogLevel::eError> ( "Layer: %s - Message: %s", pLayerPrefix, pMessage );
		return VK_TRUE;
	}
	VkBool32 VKAPI_PTR warnLogger (
		VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
		u64 object, size_t location, int32_t messageCode,
		const char* pLayerPrefix, const char* pMessage, void* pUserData ) {
		
		v_logger.log<LogLevel::eWarn> ( "Layer: %s - Message: %s", pLayerPrefix, pMessage );
		return VK_TRUE;
	}
	PFN_vkCreateDebugReportCallbackEXT pfn_vkCreateDebugReportCallbackEXT;
	PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT;
#endif

VInstance::VInstance() {
	int count;
	GLFWmonitor** glfw_monitors = glfwGetMonitors ( &count );

	monitors.resize ( count );
	for ( int i = 0; i < count; ++i ) {
		VMonitor* vmonitor = new VMonitor ( glfw_monitors[i] );
		monitor_map.insert ( std::make_pair ( glfw_monitors[i], vmonitor ) );
		monitors[i] = vmonitor;
	}
	if ( glfwVulkanSupported() ) {
		v_logger.log<LogLevel::eInfo> ( "Vulkan supported" );
	} else {
		v_logger.log<LogLevel::eError> ( "Vulkan not supported" );
		initialized = false;
		return;
	}
	gatherExtLayer ( vk::PhysicalDevice(), &extLayers.availableLayers, &extLayers.availableExtensions );

	if(v_logger.level <= LogLevel::eDebug) {
		v_logger.log<LogLevel::eDebug> ( "Instance Extensions:" );
		for ( vk::ExtensionProperties& extProp : extLayers.availableExtensions ) {
			v_logger.log<LogLevel::eDebug> ( "\t%s", extProp.extensionName );
		}
		v_logger.log<LogLevel::eDebug> ( "Instance Layers:" );
		for ( vk::LayerProperties& layerProp : extLayers.availableLayers ) {
			v_logger.log<LogLevel::eDebug> ( "\t%s", layerProp.layerName );
			v_logger.log<LogLevel::eDebug> ( "\t\tDesc: %s", layerProp.description );
		}
	}
	u32 instanceExtCount;
	const char** glfwReqInstanceExt = glfwGetRequiredInstanceExtensions ( &instanceExtCount );
	for ( size_t i = 0; i < instanceExtCount; i++ ) {
		extLayers.activateExtension ( glfwReqInstanceExt[i] );
	}
#ifndef NDEBUG
	extLayers.activateExtension ( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
	extLayers.activateLayer ( "VK_LAYER_LUNARG_standard_validation" );
#endif

	vk::ApplicationInfo appInfo ( "Vulcan Instance", VK_MAKE_VERSION ( 1, 0, 0 ), "Wupl-Engine", VK_MAKE_VERSION ( 1, 0, 0 ), VK_MAKE_VERSION ( 1, 0, 61 ) );

	{
		//Array<const char*> neededLayers(extLayers.neededLayers.size());
		const char* neededLayers[extLayers.neededLayers.size()];
		for ( int i = 0; i < extLayers.neededLayers.size(); i++ ) {
			neededLayers[i] = extLayers.neededLayers[i].cstr;
		}
		//Array<const char*> neededExtensions(extLayers.neededExtensions.size());
		const char* neededExtensions[extLayers.neededExtensions.size()];
		for ( int i = 0; i < extLayers.neededExtensions.size(); i++ ) {
			neededExtensions[i] = extLayers.neededExtensions[i].cstr;
		}
		vk::InstanceCreateInfo instanceCreateInfo ( vk::InstanceCreateFlags(), &appInfo,
		        extLayers.neededLayers.size(), neededLayers,
		        extLayers.neededExtensions.size(), neededExtensions );
		V_CHECKCALL ( vk::createInstance ( &instanceCreateInfo, nullptr, &v_instance ), v_logger.log<LogLevel::eError> ( "Instance Creation Failed" ) );

	}
#ifndef NDEBUG
	pfn_vkCreateDebugReportCallbackEXT = ( PFN_vkCreateDebugReportCallbackEXT ) glfwGetInstanceProcAddress ( v_instance, "vkCreateDebugReportCallbackEXT" );
	pfn_vkDestroyDebugReportCallbackEXT = ( PFN_vkDestroyDebugReportCallbackEXT ) glfwGetInstanceProcAddress ( v_instance, "vkDestroyDebugReportCallbackEXT" );
	
	vk::DebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo (
	    vk::DebugReportFlagsEXT (
	        vk::DebugReportFlagBitsEXT::ePerformanceWarning | 
			vk::DebugReportFlagBitsEXT::eDebug ),
	    &debugLogger,
	    nullptr
	);
	pfn_vkCreateDebugReportCallbackEXT ( v_instance, reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT*> ( &debugReportCallbackCreateInfo ), nullptr, reinterpret_cast<VkDebugReportCallbackEXT*> ( &debugReportCallbackEXT ) );
	debugReportCallbackCreateInfo = vk::DebugReportCallbackCreateInfoEXT(
	    vk::DebugReportFlagsEXT ( vk::DebugReportFlagBitsEXT::eInformation),
	    &infoLogger,
	    nullptr
	);
	pfn_vkCreateDebugReportCallbackEXT ( v_instance, reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT*> ( &debugReportCallbackCreateInfo ), nullptr, reinterpret_cast<VkDebugReportCallbackEXT*> ( &debugReportCallbackEXT ) );
	debugReportCallbackCreateInfo = vk::DebugReportCallbackCreateInfoEXT(
	    vk::DebugReportFlagsEXT ( vk::DebugReportFlagBitsEXT::eWarning ),
	    &warnLogger,
	    nullptr
	);
	pfn_vkCreateDebugReportCallbackEXT ( v_instance, reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT*> ( &debugReportCallbackCreateInfo ), nullptr, reinterpret_cast<VkDebugReportCallbackEXT*> ( &debugReportCallbackEXT ) );
	debugReportCallbackCreateInfo = vk::DebugReportCallbackCreateInfoEXT(
	    vk::DebugReportFlagsEXT ( vk::DebugReportFlagBitsEXT::eError ),
	    &errorLogger,
	    nullptr
	);
	pfn_vkCreateDebugReportCallbackEXT ( v_instance, reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT*> ( &debugReportCallbackCreateInfo ), nullptr, reinterpret_cast<VkDebugReportCallbackEXT*> ( &debugReportCallbackEXT ) );
#endif
	u32 devicecount = 0;
	V_CHECKCALL ( v_instance.enumeratePhysicalDevices ( &devicecount, nullptr ), v_logger.log<LogLevel::eError>  ( "Get physical-device count Failed" ) );

	vk::PhysicalDevice physDevices[devicecount];
	devices.resize ( devicecount );
	V_CHECKCALL ( v_instance.enumeratePhysicalDevices ( &devicecount, physDevices ), v_logger.log<LogLevel::eError>  ( "Get physical-devicec Failed" ) );

	for ( size_t i = 0; i < devicecount; i++ ) {
		VDevice* vulkan_device = new VDevice();
		devices[i] = vulkan_device;

		vulkan_device->physical_device = physDevices[i];
		physDevices[i].getProperties ( &vulkan_device->vkPhysDevProps );
		physDevices[i].getFeatures ( &vulkan_device->vkPhysDevFeatures );

		gatherExtLayer ( physDevices[i], &vulkan_device->extLayers.availableLayers, &vulkan_device->extLayers.availableExtensions );

		v_logger.log<LogLevel::eInfo>  ( "Physical Device %s", vulkan_device->vkPhysDevProps.deviceName );

		vk::PhysicalDeviceMemoryProperties memProperties;
		vulkan_device->physical_device.getMemoryProperties ( &memProperties );

		if(v_logger.level == LogLevel::eDebug) {
			for ( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ ) {
				v_logger.log<LogLevel::eDebug>  ( "Memory %d", i );
				v_logger.log<LogLevel::eDebug> ( "\tHeap Index %d %s", memProperties.memoryTypes[i].heapIndex, to_string ( memProperties.memoryTypes[i].propertyFlags ).c_str() );
			}
			for ( uint32_t i = 0; i < memProperties.memoryHeapCount; i++ ) {
				v_logger.log<LogLevel::eDebug> ( "Heap %d", i );
				v_logger.log<LogLevel::eDebug> ( "\tSize %" PRIu64 " %s", memProperties.memoryHeaps[i].size, to_string ( memProperties.memoryHeaps[i].flags ).c_str() );
			}
		}


		u32 count;
		vulkan_device->physical_device.getQueueFamilyProperties ( &count, nullptr );
		vulkan_device->queueFamilyProps.resize ( count );
		vulkan_device->physical_device.getQueueFamilyProperties ( &count, vulkan_device->queueFamilyProps.data() );

		{
			//rate a physicalDevice
			vulkan_device->rating = 0;
			if ( vulkan_device->vkPhysDevProps.deviceType == vk::PhysicalDeviceType::eDiscreteGpu ) {
				vulkan_device->rating += 1000;
			}
			// Maximum possible size of textures affects graphics quality
			vulkan_device->rating += vulkan_device->vkPhysDevProps.limits.maxImageDimension2D;
			// Application can't function without geometry shaders
			if ( !vulkan_device->vkPhysDevFeatures.geometryShader ) {
				vulkan_device->rating = 0;
			}
			bool canPresent = false;
			for ( size_t j = 0; j < vulkan_device->queueFamilyProps.size(); j++ ) {
				if ( glfwGetPhysicalDevicePresentationSupport ( v_instance, physDevices[i], j ) ) {
					canPresent = true;
					break;
				}
			}
			if ( !canPresent )
				vulkan_device->rating = 0;
		}
		vulkan_device->name = vulkan_device->vkPhysDevProps.deviceName;
	}

	std::sort ( devices.begin(), devices.end(), [] ( Device * lhs, Device * rhs ) {
		return lhs->rating > rhs->rating;
	} );


	initialized = true;
	
}

VInstance::~VInstance() {
	wait_for_frame ( frame_index );
	while (!ready_staging_buffer_queue.empty()) {
		delete ready_staging_buffer_queue.front();
		ready_staging_buffer_queue.pop();
	}
	while (!free_staging_buffer_queue.empty()) {
		delete free_staging_buffer_queue.front().second;
		free_staging_buffer_queue.pop();
	}
	for ( auto& ele : v_model_map ) {
		for ( VModel* model : ele.second ) {
			if ( model ) delete model;
		}
	}
	for ( auto& ele : v_context_map ) {
		for ( VContext* context : ele.second ) {
			if ( context ) delete context;
		}
	}
	for ( auto entry : monitor_map ) {
		delete entry.second;
	}
	for ( auto entry : window_map ) {
		destroy_window ( entry.second );
	}
	delete m_resource_manager;
	for ( auto ele : instancegroup_store ) {
		delete ele;
	}
	for ( auto ele : renderstage_store ) {
		delete ele;
	}
	for ( std::pair<ContextBaseId, VContextBase> ele : contextbase_map ) {
		if ( ele.second.descriptorset_layout ) vk_device().destroyDescriptorSetLayout ( ele.second.descriptorset_layout );
	}
	for ( vk::Fence fence : free_fences ) {
		vk_device().destroyFence ( fence );
	}
	delete context_bufferstorage;
	vk_device().destroyCommandPool ( transfer_commandpool );

	m_device.destroy ( nullptr );

}
bool VInstance::initialize ( InstanceOptions options, Device* device ) {
	if ( !device )
		device = devices[0];
	this->options = options;
	v_device = dynamic_cast<VDevice*> ( device );
	if ( !initialized && !device )
		return false;
	if(v_logger.level <= LogLevel::eDebug) {
		v_logger.log<LogLevel::eDebug> ( "Device Extensions available:" );
		for ( vk::ExtensionProperties& prop : v_device->extLayers.availableExtensions ) {
			v_logger.log<LogLevel::eDebug> ( "\t%s", prop.extensionName );
		}
		v_logger.log<LogLevel::eDebug> ( "Device Layers available:" );
		for ( vk::LayerProperties& prop : v_device->extLayers.availableLayers ) {
			v_logger.log<LogLevel::eDebug> ( "\t%s", prop.layerName );
		}
	}
	
	v_device->extLayers.activateExtension ( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
	
#ifndef NDEBUG
	v_device->extLayers.activateLayer ( "VK_LAYER_LUNARG_standard_validation" );
#endif
	const float priority = 1.0f;

	u32 pgcId = -1;
	u32 pgId = -1;
	u32 pId = -1;
	u32 gId = -1;
	u32 cId = -1;

	u32 highestRate = 0;
	for ( size_t j = 0; j < v_device->queueFamilyProps.size(); ++j ) {
		u32 rating = 0;

		if ( vk::QueueFlags ( v_device->queueFamilyProps[j].queueFlags ) == vk::QueueFlagBits::eTransfer && v_device->queueFamilyProps[j].queueCount >= 1 ) {
			queues.dedicated_transfer_queue = true;
			queues.transfer_queue_id = j;
		}
		bool present = glfwGetPhysicalDevicePresentationSupport ( v_instance, v_device->physical_device, j );
		bool graphics = ( bool ) vk::QueueFlags ( v_device->queueFamilyProps[j].queueFlags & vk::QueueFlagBits::eGraphics );
		bool compute = ( bool ) vk::QueueFlags ( v_device->queueFamilyProps[j].queueFlags & vk::QueueFlagBits::eCompute );
		bool sparse = ( bool ) vk::QueueFlags ( v_device->queueFamilyProps[j].queueFlags & vk::QueueFlagBits::eSparseBinding );
		if ( present ) rating++;
		if ( graphics ) rating++;
		if ( compute ) rating++;
		if ( sparse ) rating++;

		if ( present && graphics && compute && ( pgcId == ( u32 ) - 1 || rating > highestRate ) ) {
			pgcId = j;
		}
		if ( present && graphics && ( pgId == ( u32 ) - 1 || rating > highestRate ) ) {
			pgId = j;
		}
		if ( present && ( pId == ( u32 ) - 1 || rating > highestRate ) ) {
			pId = j;
		}
		if ( graphics && ( gId == ( u32 ) - 1 || rating > highestRate ) ) {
			gId = j;
		}
		if ( compute && ( cId == ( u32 ) - 1 || rating > highestRate ) ) {
			cId = j;
		}
		if ( rating > highestRate )
			highestRate = rating;
	}
	size_t queueFamilyCount = 0;
	if ( queues.dedicated_transfer_queue ) {
		v_logger.log<LogLevel::eDebug> ( "Dedicated Transfer Queue Found" );
		queueFamilyCount++;
	}

	if ( pgcId != ( u32 ) - 1 ) {
		pId = pgcId;
		gId = pgcId;
		cId = pgcId;
		queueFamilyCount++;
	} else if ( pgId != ( u32 ) - 1 ) {
		pId = pgId;
		gId = pgId;
		queueFamilyCount += 2;
	} else {
		queueFamilyCount += 3;
	}
	if ( queues.transfer_queue_id == -1 ) {
		queues.transfer_queue_id = gId;
	}

	vk::DeviceQueueCreateInfo deviceQueueCreateInfos[queueFamilyCount];
	size_t currentIndex = 0;
	if ( queues.dedicated_transfer_queue ) {
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), queues.transfer_queue_id, 1, &priority );
		v_device->queueFamilyProps[queues.transfer_queue_id].queueCount--;
		currentIndex++;
	}
	if ( pgcId != ( u32 ) - 1 ) {
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), pgcId, 1, &priority );
		v_device->queueFamilyProps[pgcId].queueCount -= 1;
		currentIndex++;
	} else if ( pgId != ( u32 ) - 1 ) {
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), pgId, 1, &priority );
		v_device->queueFamilyProps[pgId].queueCount -= 1;
		currentIndex++;

		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), cId, 1, &priority );
		v_device->queueFamilyProps[cId].queueCount -= 1;
		currentIndex++;
	} else {
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), pId, 1, &priority );
		v_device->queueFamilyProps[pId].queueCount -= 1;
		currentIndex++;

		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), gId, 1, &priority );
		v_device->queueFamilyProps[gId].queueCount -= 1;
		currentIndex++;

		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), cId, 1, &priority );
		v_device->queueFamilyProps[cId].queueCount -= 1;
		currentIndex++;
	}

	v_logger.log<LogLevel::eDebug> ( "Create %d Present-Graphics-Compute Queues", 1 );

	vk::PhysicalDeviceFeatures physicalDeviceFeatures;
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
	physicalDeviceFeatures.multiDrawIndirect = VK_TRUE;
	physicalDeviceFeatures.depthClamp = VK_TRUE;
	physicalDeviceFeatures.fillModeNonSolid = VK_TRUE;// for only mesh rendering

	{
		//Array<const char*> neededLayers(extLayers.neededLayers.size());
		const char* neededLayers[v_device->extLayers.neededLayers.size()];
		for ( int i = 0; i < v_device->extLayers.neededLayers.size(); i++ ) {
			neededLayers[i] = v_device->extLayers.neededLayers[i].cstr;
		}
		//Array<const char*> neededExtensions(extLayers.neededExtensions.size());
		const char* neededExtensions[v_device->extLayers.neededExtensions.size()];
		for ( int i = 0; i < v_device->extLayers.neededExtensions.size(); i++ ) {
			neededExtensions[i] = v_device->extLayers.neededExtensions[i].cstr;
		}
		vk::DeviceCreateInfo deviceCreateInfo ( vk::DeviceCreateFlags(), queueFamilyCount, deviceQueueCreateInfos,
		                                        v_device->extLayers.neededLayers.size(), neededLayers,
		                                        v_device->extLayers.neededExtensions.size(), neededExtensions,
		                                        &physicalDeviceFeatures );
		V_CHECKCALL ( v_device->physical_device.createDevice ( &deviceCreateInfo, nullptr, &m_device ), v_logger.log<LogLevel::eError> ( "Device Creation Failed" ) );

	}


	if ( queues.dedicated_transfer_queue ) {
		m_device.getQueue ( queues.transfer_queue_id, 0, &queues.transfer_queue );
	}
	if ( pgcId != ( u32 ) - 1 ) {
		m_device.getQueue ( pgcId, 0, &queues.graphics_queue );
		queues.present_queue = queues.graphics_queue;
		queues.compute_queue = queues.graphics_queue;
	} else if ( pgId != ( u32 ) - 1 ) {
		m_device.getQueue ( pgcId, 0, &queues.graphics_queue );
		m_device.getQueue ( pgcId, 0, &queues.compute_queue );
		queues.present_queue = queues.graphics_queue;
	} else {
		m_device.getQueue ( gId, 0, &queues.graphics_queue );
		m_device.getQueue ( pId, 0, &queues.present_queue );
		m_device.getQueue ( cId, 0, &queues.compute_queue );
	}
	queues.graphics_queue_id = gId;
	queues.present_queue_id = pId;
	queues.compute_queue_id = cId;
	queues.combined_graphics_present_queue = ( pId == gId );
	queues.combined_graphics_compute_queue = ( gId == cId );

	transfer_commandpool = vk_device().createCommandPool (
	                           vk::CommandPoolCreateInfo ( vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queue_wrapper()->transfer_queue_id )
	                       );

	context_bufferstorage = new VUpdateableBufferStorage(this, vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst);
	register_specializations ( this );
	return true;
}

void VInstance::process_events() {
	glfwPollEvents();
}
void VInstance::process_events(double timeout) {
	glfwWaitEventsTimeout(timeout);
}
bool VInstance::is_window_open() {
	for ( Window* window : windows ) {
		if ( (bool) window->visible() )
			return true;
	}
	return false;
}

VMonitor* VInstance::get_primary_monitor_vulkan() {
	return monitor_map.find ( glfwGetPrimaryMonitor() )->second;
}
Monitor* VInstance::get_primary_monitor() {
	return get_primary_monitor_vulkan();
}
ResourceManager* VInstance::resource_manager() {
	if ( m_resource_manager ) return m_resource_manager;
	m_resource_manager = new VResourceManager ( this );
	return m_resource_manager;
}
Window* VInstance::create_window() {
	VWindow* window = new VWindow ( this );
	windows.insert ( window );
	return window;
}
bool VInstance::destroy_window ( Window* window ) {
	if ( VWindow* vulk_window = dynamic_cast<VWindow*> ( window ) ) {
		if ( windows.erase ( vulk_window ) > 0 ) {
			delete vulk_window;
			return true;
		} else {
			return false;
		}
	}
	return false;
}
void VInstance::contextbase_registered ( ContextBaseId id ) {
	v_logger.log<LogLevel::eDebug> ( "Registered Context Base 0x%" PRIx32, id );
	const ContextBase* contextbase_ptr = contextbase ( id );
	u32 ds_size = 0;
	if ( contextbase_ptr->datagroup.size ) ds_size++;
	if ( contextbase_ptr->image_count ) ds_size++;
	if ( contextbase_ptr->image_count ) ds_size++;

	Array<vk::DescriptorSetLayoutBinding> dslbs ( ds_size );
	u32 index = 0;
	if ( contextbase_ptr->datagroup.size ) {
		dslbs[index] = vk::DescriptorSetLayoutBinding ( index, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll, nullptr );
		index++;
	}
	if ( contextbase_ptr->image_count ) {
		dslbs[index] = vk::DescriptorSetLayoutBinding ( index, vk::DescriptorType::eSampledImage, contextbase_ptr->image_count, vk::ShaderStageFlagBits::eAll, nullptr );
		index++;
	}
	if ( contextbase_ptr->sampler_count ) {
		dslbs[index] = vk::DescriptorSetLayoutBinding ( index, vk::DescriptorType::eSampler, contextbase_ptr->sampler_count, vk::ShaderStageFlagBits::eAll, nullptr );
		index++;
	}

	VContextBase v_contextbase;
	if ( ds_size ) v_contextbase.descriptorset_layout = vk_device ().createDescriptorSetLayout ( vk::DescriptorSetLayoutCreateInfo ( vk::DescriptorSetLayoutCreateFlags(), dslbs.size, dslbs.data ), nullptr ),
		                             contextbase_map[id] = v_contextbase;
}
void VInstance::modelbase_registered ( ModelBaseId id ) {
	v_logger.log<LogLevel::eDebug> ( "Registered Model Base 0x%" PRIx32, id );
}
void VInstance::instancebase_registered ( InstanceBaseId id ) {
	v_logger.log<LogLevel::eDebug> ( "Registered ModelInstance Base 0x%" PRIx32, id );
}
Context VInstance::create_context ( ContextBaseId contextbase_id ) {
	v_logger.log<LogLevel::eDebug> ( "Create Context 0x%" PRIx32, contextbase_id );
	
	VContext* created_context = v_context_map[contextbase_id].insert ( new VContext ( this, contextbase_id ) );
	return created_context->context();
}
Model VInstance::create_model ( ModelBaseId modelbase_id ) {
	v_logger.log<LogLevel::eDebug> ( "Create Model 0x%" PRIx32, modelbase_id );
	VModel* created_model = v_model_map[modelbase_id].insert ( new VModel ( this, modelbase_id ) );
	return created_model->model();
}


InstanceGroup* VInstance::create_instancegroup() {
	VInstanceGroup* instancegroup = new VInstanceGroup ( this );
	instancegroup_store.push_back ( instancegroup );
	return instancegroup;
}
ContextGroup* VInstance::create_contextgroup() {
	return new VContextGroup ( this );
}
RenderBundle* VInstance::create_renderbundle ( InstanceGroup* igroup, ContextGroup* cgroup, Array<const RenderStage*>& rstages, Array<ImageUsage>& image_types, Array<ImageDependency>& dependencies ) {
	//return new VRenderBundle ( igroup, cgroup, rstages, image_types, dependencies );
	return nullptr;
}
void VInstance::render_bundles ( Array<RenderBundle*> bundles ) {
	for ( RenderBundle* b : bundles ) {
		if ( VRenderBundle* bundle = dynamic_cast<VRenderBundle*> ( b ) ) {
			if ( bundle ) {
				bundle->v_dispatch();
			}
		}
		if ( VMainBundle* bundle = dynamic_cast<VMainBundle*> ( b ) ) {
			if ( bundle ) {
				bundle->v_dispatch();
			}
		}
	}
}
RenderBundle* VInstance::create_main_bundle ( InstanceGroup* igroup) {
	return new VMainBundle ( this, igroup );
}

void VInstance::load_generic_model ( Model& model, void* vertices, u32 vertexcount, u16* indices, u32 indexcount ) {
	const ModelBase* modelbase_ptr = modelbase ( model.modelbase_id );

	u64 vertexbuffersize = vertexcount * modelbase_ptr->datagroup.size;
	u64 indexbuffersize = indexcount * sizeof ( u16 );

	VModel* newmodel = v_model_map[model.modelbase_id][model.id];
	newmodel->index_is_2byte = true;
	newmodel->vertexoffset = 0;
	newmodel->vertexcount = vertexcount;
	newmodel->indexoffset = 0;
	newmodel->indexcount = indexcount;
	newmodel->vertexbuffer.init ( vertexbuffersize,
	                              vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
	                              vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );
	newmodel->indexbuffer.init ( indexbuffersize,
	                             vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
	                             vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );


	VThinBuffer staging_buffer = request_staging_buffer ( vertexbuffersize + indexbuffersize );

	memcpy ( staging_buffer.mapped_ptr, vertices, vertexbuffersize );
	memcpy ( staging_buffer.mapped_ptr + vertexbuffersize, indices, indexbuffersize );

	Array<VSimpleTransferJob> jobs = {
		{staging_buffer, &newmodel->vertexbuffer, {0, 0, vertexbuffersize}},
		{staging_buffer, &newmodel->indexbuffer, {vertexbuffersize, 0, indexbuffersize}}
	};
	//TODO make asynch
	transfer_data ( jobs );
}
void VInstance::load_generic_model ( Model& model, void* vertices, u32 vertexcount, u32* indices, u32 indexcount ) {
	const ModelBase* modelbase_ptr = modelbase ( model.modelbase_id );
	bool fitsin2bytes = vertexcount < 0x10000;

	u64 vertexbuffersize = vertexcount * modelbase_ptr->datagroup.size;
	u64 indexbuffersize = indexcount * sizeof ( u32 );

	VModel* newmodel = v_model_map[model.modelbase_id][model.id];
	newmodel->index_is_2byte = false;
	newmodel->vertexoffset = 0;
	newmodel->vertexcount = vertexcount;
	newmodel->indexoffset = 0;
	newmodel->indexcount = indexcount;
	newmodel->vertexbuffer.init ( vertexbuffersize,
	                              vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
	                              vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );
	newmodel->indexbuffer.init ( indexbuffersize,
	                             vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
	                             vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );

	VThinBuffer staging_buffer = request_staging_buffer ( vertexbuffersize + indexbuffersize );

	memcpy ( staging_buffer.mapped_ptr, vertices, vertexbuffersize );
	memcpy ( staging_buffer.mapped_ptr + vertexbuffersize, indices, indexbuffersize );

	Array<VSimpleTransferJob> jobs = {
		{staging_buffer, &newmodel->vertexbuffer, {0, 0, vertexbuffersize}},
		{staging_buffer, &newmodel->indexbuffer, {vertexbuffersize, 0, indexbuffersize}}
	};
	//TODO make asynch
	transfer_data ( jobs );
}
void VInstance::unload_model ( ModelId modelbase_id ) {

}
void VInstance::set_context ( Model& model, Context& context ) {
	VModel* v_model = v_model_map[model.modelbase_id][model.id];
	const ModelBase* modelbase_ptr = modelbase ( model.modelbase_id );
	for ( u32 i = 0; i < modelbase_ptr->contextbase_ids.size; i++ ) {
		if ( modelbase_ptr->contextbase_ids[i] == context.contextbase_id ) {
			v_model->v_contexts[i] = v_context_map[context.contextbase_id][context.id];
		}
	}
}
void VInstance::update_context_data ( Context& context, void* data ) {
	VContext* v_context = v_context_map[context.contextbase_id][context.id];
	v_context->data = data;
}
void VInstance::update_context_image ( Context& context, u32 index, ImageUseRef imageuse ) {
	VContext* v_context = v_context_map[context.contextbase_id][context.id];
	const ContextBase* contextbase_ptr = contextbase ( context.contextbase_id );
	
	v_context->images[index] = VImageUseRef(imageuse);
	
	u32 writecount = 0;
	if ( contextbase_ptr->datagroup.size ) writecount++;
	if ( contextbase_ptr->image_count ) {
		//bind descriptorset to image
		vk::DescriptorImageInfo imagewrite = vk::DescriptorImageInfo ( vk::Sampler(), v_context->images[index].imageview(), vk::ImageLayout::eShaderReadOnlyOptimal );
		vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet ( v_context->descriptor_set, writecount, index, 1, vk::DescriptorType::eSampledImage, &imagewrite, nullptr, nullptr );
		vk_device().updateDescriptorSets ( 1, &writeDescriptorSet, 0, nullptr );
		writecount++;
	}
}
void VInstance::update_context_sampler ( Context& context, u32 index, Sampler* sampler ) {
	VContext* v_context = v_context_map[context.contextbase_id][context.id];
	const ContextBase* contextbase_ptr = contextbase ( context.contextbase_id );
	
	VSampler* v_sampler = m_resource_manager->v_samplers[sampler->id];
	v_context->samplers[index] = v_sampler;
	
	u32 writecount = 0;
	if ( contextbase_ptr->datagroup.size ) writecount++;
	if ( contextbase_ptr->image_count ) writecount++;
	if ( contextbase_ptr->sampler_count ) {
		//bind descriptorset to image
		vk::DescriptorImageInfo imagewrite = vk::DescriptorImageInfo ( v_sampler->sampler, vk::ImageView(), vk::ImageLayout::eUndefined );
		vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet ( v_context->descriptor_set, writecount, index, 1, vk::DescriptorType::eSampler, &imagewrite, nullptr, nullptr );
		vk_device().updateDescriptorSets ( 1, &writeDescriptorSet, 0, nullptr );
		writecount++;
	}
}

void VInstance::allocate_gpu_memory ( vk::MemoryRequirements mem_req, GPUMemory* memory ) {

	memory->heap_index = find_memory_type ( mem_req.memoryTypeBits, memory->needed | memory->recommended );
	memory->property_flags = memory->needed | memory->recommended;
	if ( memory->heap_index == std::numeric_limits<u32>::max() ) {
		memory->heap_index = find_memory_type ( mem_req.memoryTypeBits, memory->needed );
		memory->property_flags = memory->needed;
	}
	if ( memory->heap_index != std::numeric_limits<u32>::max() ) {
		vk::MemoryAllocateInfo allocInfo ( mem_req.size, memory->heap_index );
		V_CHECKCALL ( m_device.allocateMemory ( &allocInfo, nullptr, &memory->memory ), v_logger.log<LogLevel::eError> ( "Failed To Create Image Memory" ) );

		v_logger.log<LogLevel::eDebug> ( "Allocated %" PRIu64 " Bytes 0x%" PRIx64 "", mem_req.size, memory->memory );

		memory->size = mem_req.size;
	} else {
		v_logger.log<LogLevel::eError> ( "Cannot Allocate %" PRIu64 " bytes Memory for Memtypes: 0x%x Needed Properties: 0x%x Recommended Properties: 0x%x",
		         mem_req.size, mem_req.memoryTypeBits,
		         static_cast<VkMemoryPropertyFlags> ( memory->needed ), static_cast<VkMemoryPropertyFlags> ( memory->recommended ) );
	}
}
RendResult VInstance::free_gpu_memory ( GPUMemory memory ) {
	if ( memory.memory ) {
		v_logger.log<LogLevel::eDebug> ( "Freeing %" PRIu64 " Bytes of Memory 0x%" PRIx64 "", memory.size, memory.memory );
		vk_device ().freeMemory ( memory.memory, nullptr );
		memory.memory = vk::DeviceMemory();
	}
	return RendResult::eSuccess;
}
vk::ImageView VInstance::createImageView2D ( vk::Image image, u32 mipBase, u32 mipOffset, vk::Format format, vk::ImageAspectFlags aspectFlags ) {

	vk::ImageViewCreateInfo imageViewCreateInfo (
	    vk::ImageViewCreateFlags(),
	    image,
	    vk::ImageViewType::e2D, format,
	    vk::ComponentMapping ( vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity ),
	    vk::ImageSubresourceRange ( aspectFlags, mipBase, mipOffset, 0, 1 )
	);

	vk::ImageView imageView;

	V_CHECKCALL ( m_device.createImageView ( &imageViewCreateInfo, nullptr, &imageView ), v_logger.log<LogLevel::eError> ( "Creation of ImageView failed" ) );

	return imageView;
}
vk::ImageView VInstance::createImageView2DArray ( vk::Image image, u32 mipBase, u32 mipOffset, u32 arrayOffset, u32 arraySize, vk::Format format, vk::ImageAspectFlags aspectFlags ) {

	vk::ImageViewCreateInfo imageViewCreateInfo (
	    vk::ImageViewCreateFlags(),
	    image,
	    vk::ImageViewType::e2DArray, format,
	    vk::ComponentMapping ( vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity ),
	    vk::ImageSubresourceRange ( aspectFlags, mipBase, mipOffset, arrayOffset, arraySize )
	);

	vk::ImageView imageView;

	V_CHECKCALL ( m_device.createImageView ( &imageViewCreateInfo, nullptr, &imageView ), v_logger.log<LogLevel::eError> ( "Creation of ImageView failed" ) );

	return imageView;
}
void VInstance::destroyImageView ( vk::ImageView imageview ) {
	m_device.destroyImageView ( imageview );
}

void VInstance::destroyCommandPool ( vk::CommandPool commandPool ) {
	vkDestroyCommandPool ( m_device, commandPool, nullptr );
}
vk::CommandBuffer VInstance::createCommandBuffer ( vk::CommandPool commandPool, vk::CommandBufferLevel bufferLevel ) {
	vk::CommandBufferAllocateInfo allocateInfo ( commandPool, bufferLevel, 1 );
	vk::CommandBuffer commandBuffer;
	vk_device().allocateCommandBuffers ( &allocateInfo, &commandBuffer );
	return commandBuffer;
}
void VInstance::delete_command_buffer ( vk::CommandPool commandPool, vk::CommandBuffer commandBuffer ) {
	m_device.freeCommandBuffers ( commandPool, 1, &commandBuffer );
}

u32 VInstance::find_memory_type ( u32 typeFilter, vk::MemoryPropertyFlags properties ) {
	vk::PhysicalDeviceMemoryProperties memProperties;
	v_device->physical_device.getMemoryProperties ( &memProperties );
	for ( u32 i = 0; i < memProperties.memoryTypeCount; i++ ) {
		if ( ( typeFilter & ( 1 << i ) ) && ( memProperties.memoryTypes[i].propertyFlags & properties ) == properties ) {
			return i;
		}
	}
	for ( u32 i = 0; i < memProperties.memoryTypeCount; i++ ) {
		if ( ( typeFilter & ( 1 << i ) ) && ( memProperties.memoryTypes[i].propertyFlags & properties ) == properties ) {
			return i;
		}
	}
	return -1;
}

vk::Format VInstance::find_supported_image_format ( const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features ) {
	for ( vk::Format format : candidates ) {
		vk::FormatProperties props;
		v_device->physical_device.getFormatProperties ( format, &props );
		if ( tiling == vk::ImageTiling::eLinear && ( props.linearTilingFeatures & features ) == features ) {
			return format;
		} else if ( tiling == vk::ImageTiling::eOptimal && ( props.optimalTilingFeatures & features ) == features ) {
			return format;
		}
	}
	assert ( false );
	return vk::Format::eUndefined;
}

vk::Format VInstance::find_depth_image_format() {
	return find_supported_image_format (
	{vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat},
	vk::ImageTiling::eOptimal,
	vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);
}
