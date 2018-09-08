#include "VulkanInstance.h"
#include "VulkanWindow.h"
#include "VulkanTransferOperator.h"
#include <fstream>
#include "VulkanContext.h"
#include "VulkanModel.h"

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
		V_CHECKCALL ( vk::enumerateInstanceExtensionProperties ( nullptr, &count, nullptr ), printf ( "Could not get Extension-count" ) );
	} else {
		V_CHECKCALL ( device.enumerateDeviceExtensionProperties ( nullptr, &count, nullptr ), printf ( "Could not get Extension-count" ) );
	}
	extensions->resize ( count );
	if ( !device ) {
		V_CHECKCALL ( vk::enumerateInstanceExtensionProperties ( nullptr, &count, extensions->data() ), printf ( "Could not get Extensions" ) );
	} else {
		V_CHECKCALL ( device.enumerateDeviceExtensionProperties ( nullptr, &count, extensions->data() ), printf ( "Could not get Extensions" ) );
	}

	if ( !device ) {
		V_CHECKCALL ( vk::enumerateInstanceLayerProperties ( &count, nullptr ), printf ( "Could not get Layer-count" ) );
	} else {
		V_CHECKCALL ( device.enumerateDeviceLayerProperties ( &count, nullptr ), printf ( "Could not get Layer-count" ) );
	}
	layers->resize ( count );
	if ( !device ) {
		V_CHECKCALL ( vk::enumerateInstanceLayerProperties ( &count, layers->data() ), printf ( "Could not get Layers" ) );
	} else {
		V_CHECKCALL ( device.enumerateDeviceLayerProperties ( &count, layers->data() ), printf ( "Could not get Layers" ) );
	}
	for ( vk::LayerProperties& layerProp : *layers ) {

		if ( !device ) {
			V_CHECKCALL ( vk::enumerateInstanceExtensionProperties ( layerProp.layerName, &count, nullptr ), printf ( "Could not get Extension-count" ) );
		} else {
			V_CHECKCALL ( device.enumerateDeviceExtensionProperties ( layerProp.layerName, &count, nullptr ), printf ( "Could not get Extension-count" ) );
		}
		vk::ExtensionProperties extensionArray[count];
		if ( !device ) {
			V_CHECKCALL ( vk::enumerateInstanceExtensionProperties ( layerProp.layerName, &count, extensionArray ), printf ( "Could not get Extensions" ) );
		} else {
			V_CHECKCALL ( device.enumerateDeviceExtensionProperties ( layerProp.layerName, &count, extensionArray ), printf ( "Could not get Extensions" ) );
		}
		for ( size_t i = 0; i < count; ++i ) {
			addExtension ( extensions, extensionArray[i] );
		}
	}
}
bool VulkanExtLayerStruct::activateLayer ( const char* name ) {
	bool found = false;
	for ( vk::LayerProperties& layer : availableLayers ) {
		if ( !strcmp ( layer.layerName, name ) ) {
			found = true;
			break;
		}
	}
	if ( !found )
		return false;
	for ( const char* layName : neededLayers ) {
		if ( !strcmp ( layName, name ) ) {
			return true;
		}
	}
	neededLayers.push_back ( name );
	return true;
}
bool VulkanExtLayerStruct::activateExtension ( const char* name ) {
	bool found = false;
	for ( vk::ExtensionProperties& ext : availableExtensions ) {
		if ( !strcmp ( ext.extensionName, name ) ) {
			found = true;
			break;
		}
	}
	if ( !found )
		return false;
	for ( const char* extName : neededExtensions ) {
		if ( !strcmp ( extName, name ) ) {
			return true;
		}
	}
	neededExtensions.push_back ( name );
	return true;
}


Instance* initialize_instance ( const char* name ) {
	if ( strcmp ( name, "Vulkan" ) == 0 ) {
		glfwInit();
		VulkanInstance* instance = new VulkanInstance();
		return instance;
	}
	return nullptr;
}

void destroy_instance ( Instance* instance ) {
	if ( VulkanInstance* vulkan_instance = dynamic_cast<VulkanInstance*> ( instance ) ) {
		delete vulkan_instance;
		glfwTerminate();
	} else {
		delete instance;
	}
}
VideoMode glfw_to_videomode ( GLFWvidmode glfw_videomode ) {
	return {Extent2D<s32> ( glfw_videomode.width, glfw_videomode.height ), ( u32 ) glfw_videomode.refreshRate};
}

VulkanMonitor::VulkanMonitor ( GLFWmonitor* monitor ) : monitor ( monitor ) {
	this->name = glfwGetMonitorName ( monitor );

	int xpos, ypos;
	glfwGetMonitorPos ( monitor, &xpos, &ypos );
	this->offset.x = xpos;
	this->offset.y = ypos;


	int count;
	const GLFWvidmode* videomode = glfwGetVideoModes ( monitor, &count );
	for ( int i = 0; i < count; ++i ) {
		if ( videomode[i].redBits == 8 || videomode[i].greenBits == 8 || videomode[i].blueBits == 8 ) {
			videomodes.push_back ( glfw_to_videomode ( videomode[i] ) );
			this->extend = max_extend ( this->extend, videomodes.back().extend );
			//printf ("\tVideomode %dx%d r%dg%db%d %dHz\n", videomode[i].width, videomode[i].height, videomode[i].redBits, videomode[i].greenBits, videomode[i].blueBits, videomode[i].refreshRate);
		}
	}
	printf ( "Monitor %s: %dx%d %dx%d\n", this->name, this->offset.x, this->offset.y, this->extend.x, this->extend.y );
}
VulkanMonitor::~VulkanMonitor() {

}
VideoMode VulkanMonitor::current_mode() {
	return glfw_to_videomode ( *glfwGetVideoMode ( this->monitor ) );
}
VkBool32 VKAPI_PTR debugLogger (
    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
    u64 object, size_t location, int32_t messageCode,
    const char* pLayerPrefix, const char* pMessage, void* pUserData ) {
	printf ( "Layer: %s - Message: %s\n", pLayerPrefix, pMessage );
	fflush ( stdout );
	return VK_TRUE;
}
PFN_vkCreateDebugReportCallbackEXT pfn_vkCreateDebugReportCallbackEXT;
PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT;

VulkanInstance::VulkanInstance() {
	int count;
	GLFWmonitor** glfw_monitors = glfwGetMonitors ( &count );

	for ( int i = 0; i < count; ++i ) {
		VulkanMonitor* vulkanmonitor = new VulkanMonitor ( glfw_monitors[i] );
		monitor_map.insert ( std::make_pair ( glfw_monitors[i], vulkanmonitor ) );
		monitors.push_back ( vulkanmonitor );
	}
	if ( glfwVulkanSupported() ) {
		printf ( "Vulkan supported\n" );
	} else {
		printf ( "Vulkan not supported\n" );
		initialized = false;
		return;
	}
	gatherExtLayer ( vk::PhysicalDevice(), &extLayers.availableLayers, &extLayers.availableExtensions );

	printf ( "Instance Extensions:\n" );
	for ( vk::ExtensionProperties& extProp : extLayers.availableExtensions ) {
		printf ( "\t%s\n", extProp.extensionName );
	}
	printf ( "Instance Layers:\n" );
	for ( vk::LayerProperties& layerProp : extLayers.availableLayers ) {
		printf ( "\t%s\n", layerProp.layerName );
		printf ( "\t\tDesc: %s\n", layerProp.description );
	}
	u32 instanceExtCount;
	const char** glfwReqInstanceExt = glfwGetRequiredInstanceExtensions ( &instanceExtCount );
	for ( size_t i = 0; i < instanceExtCount; i++ ) {
		if ( !extLayers.activateExtension ( glfwReqInstanceExt[i] ) ) {
			printf ( "Extension %s not available\n", glfwReqInstanceExt[i] );
		} else {
			printf ( "Activate Extension %s\n", glfwReqInstanceExt[i] );
		}
	}
	if ( !extLayers.activateExtension ( VK_EXT_DEBUG_REPORT_EXTENSION_NAME ) ) {
		printf ( "Extension %s not available\n", VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
	}

	if ( !extLayers.activateLayer ( "VK_LAYER_LUNARG_standard_validation" ) ) {
		printf ( "Layer VK_LAYER_LUNARG_standard_validation not available\n" );
	}
	if ( !extLayers.activateLayer ( "VK_LAYER_LUNARG_swapchain" ) ) {
		printf ( "Layer VK_LAYER_LUNARG_swapchain not available\n" );
	}
	if ( !extLayers.activateLayer ( "VK_LAYER_RENDERDOC_Capture" ) ) {
		printf ( "Layer VK_LAYER_RENDERDOC_Capture not available\n" );
	}


	vk::ApplicationInfo appInfo ( "Vulcan Instance", VK_MAKE_VERSION ( 1, 0, 0 ), "Wupl-Engine", VK_MAKE_VERSION ( 1, 0, 0 ), VK_MAKE_VERSION ( 1, 0, 61 ) );

	vk::InstanceCreateInfo instanceCreateInfo ( vk::InstanceCreateFlags(), &appInfo,
	        extLayers.neededLayers.size(), extLayers.neededLayers.data(),
	        extLayers.neededExtensions.size(), extLayers.neededExtensions.data() );
	V_CHECKCALL ( vk::createInstance ( &instanceCreateInfo, nullptr, &m_instance ), printf ( "Instance Creation Failed\n" ) );

	pfn_vkCreateDebugReportCallbackEXT = ( PFN_vkCreateDebugReportCallbackEXT ) glfwGetInstanceProcAddress ( m_instance, "vkCreateDebugReportCallbackEXT" );
	pfn_vkDestroyDebugReportCallbackEXT = ( PFN_vkDestroyDebugReportCallbackEXT ) glfwGetInstanceProcAddress ( m_instance, "vkDestroyDebugReportCallbackEXT" );

	vk::DebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo (
	    vk::DebugReportFlagsEXT (
	        //vk::DebugReportFlagBitsEXT::eInformation |
	        vk::DebugReportFlagBitsEXT::eWarning |
	        vk::DebugReportFlagBitsEXT::ePerformanceWarning |
	        vk::DebugReportFlagBitsEXT::eError |
	        vk::DebugReportFlagBitsEXT::eDebug ),
	    &debugLogger,
	    nullptr
	);
	pfn_vkCreateDebugReportCallbackEXT ( m_instance, reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT*> ( &debugReportCallbackCreateInfo ), nullptr, reinterpret_cast<VkDebugReportCallbackEXT*> ( &debugReportCallbackEXT ) );

	u32 devicecount = 0;
	V_CHECKCALL ( m_instance.enumeratePhysicalDevices ( &devicecount, nullptr ), printf ( "Get physical-device count Failed\n" ) );

	vk::PhysicalDevice physDevices[devicecount];
	devices.resize ( devicecount );
	V_CHECKCALL ( m_instance.enumeratePhysicalDevices ( &devicecount, physDevices ), printf ( "Get physical-devicec Failed\n" ) );

	for ( size_t i = 0; i < devicecount; i++ ) {
		VulkanDevice* vulkan_device = new VulkanDevice();
		devices[i] = vulkan_device;

		vulkan_device->physical_device = physDevices[i];
		physDevices[i].getProperties ( &vulkan_device->vkPhysDevProps );
		physDevices[i].getFeatures ( &vulkan_device->vkPhysDevFeatures );

		gatherExtLayer ( physDevices[i], &vulkan_device->availableLayers, &vulkan_device->availableExtensions );

		printf ( "Physical Device %s\n", vulkan_device->vkPhysDevProps.deviceName );

		vk::PhysicalDeviceMemoryProperties memProperties;
		vulkan_device->physical_device.getMemoryProperties ( &memProperties );

		for ( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ ) {
			printf ( "Memory %d\n", i );
			printf ( "\tHeap Index %d\n", memProperties.memoryTypes[i].heapIndex );
			if ( memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal )
				printf ( "\tDevice Local\n" );
			if ( memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible )
				printf ( "\tHost Visible\n" );
			if ( memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent )
				printf ( "\tHost Coherent\n" );
			if ( memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eHostCached )
				printf ( "\tHost Cached\n" );
			if ( memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eLazilyAllocated )
				printf ( "\tLazily Allocated\n" );
		}
		for ( uint32_t i = 0; i < memProperties.memoryHeapCount; i++ ) {
			printf ( "Heap %d\n", i );
			printf ( "\tSize %I64u\n", memProperties.memoryHeaps[i].size );
			if ( memProperties.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal )
				printf ( "\tDevice Local\n" );
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
				if ( glfwGetPhysicalDevicePresentationSupport ( m_instance, physDevices[i], j ) ) {
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

VulkanInstance::~VulkanInstance() {

	for ( VulkanDeferredCall& def_call : deferred_calls ) {
		def_call.call ( def_call.frame_index );
	}
	for ( auto entry : monitor_map ) {
		delete entry.second;
	}
	for ( auto entry : window_map ) {
		destroy_window ( entry.second );
	}
	for ( auto ele : datagroup_store ) {
		delete ele;
	}
	for ( auto ele : contextbase_store ) {
		delete ele;
	}
	for ( auto ele : modelbase_store ) {
		delete ele;
	}
	for ( auto ele : model_store ) {
		delete ele;
	}
	delete transfer_control;
	m_device.destroy ( nullptr );

}
bool VulkanInstance::initialize ( Device* device ) {
	if ( !device )
		device = devices[0];
	v_device = dynamic_cast<VulkanDevice*> ( device );
	if ( !initialized && !vulkan_device )
		return false;
	VulkanExtLayerStruct extLayers;
	extLayers.availableExtensions = v_device->availableExtensions;
	extLayers.availableLayers = v_device->availableLayers;
	printf ( "Device Extensions available:\n" );
	for ( vk::ExtensionProperties& prop : extLayers.availableExtensions ) {
		printf ( "\t%s\n", prop.extensionName );
	}
	printf ( "Device Layers available:\n" );
	for ( vk::LayerProperties& prop : extLayers.availableLayers ) {
		printf ( "\t%s\n", prop.layerName );
	}

	if ( !extLayers.activateExtension ( VK_KHR_SWAPCHAIN_EXTENSION_NAME ) ) {
		printf ( "Extension %s not available\n", VK_KHR_SWAPCHAIN_EXTENSION_NAME );
	}
	if ( !extLayers.activateExtension ( VK_NV_GLSL_SHADER_EXTENSION_NAME ) ) {
		printf ( "Extension %s not available\n", VK_NV_GLSL_SHADER_EXTENSION_NAME );
	}
	const float priority = 1.0f;

	u32 pgcId = -1;
	u32 pgId = -1;
	u32 pId = -1;
	u32 gId = -1;
	u32 cId = -1;

	u32 highestRate = 0;
	for ( size_t j = 0; j < v_device->queueFamilyProps.size(); ++j ) {
		u32 rating = 0;

		if ( vk::QueueFlags ( v_device->queueFamilyProps[j].queueFlags ) == vk::QueueFlagBits::eTransfer && v_device->queueFamilyProps[j].queueCount < 1 ) {
			queues.dedicated_transfer_queue = true;
			queues.transfer_queue_id = j;
		}
		bool present = glfwGetPhysicalDevicePresentationSupport ( m_instance, v_device->physical_device, j );
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
	if ( queues.dedicated_transfer_queue )
		queueFamilyCount++;

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

	vk::DeviceQueueCreateInfo deviceQueueCreateInfos[queueFamilyCount];
	size_t currentIndex = 0;
	size_t pgcCount = 0;
	bool separateTransferQueue = false;
	if ( queues.dedicated_transfer_queue ) {
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), queues.transfer_queue_id, 1, &priority );
		v_device->queueFamilyProps[queues.transfer_queue_id].queueCount--;
		currentIndex++;
		separateTransferQueue = true;
	}
	if ( pgcId != ( u32 ) - 1 ) {
		pgcCount = v_device->queueFamilyProps[pgcId].queueCount;
		pgcCount = std::min<u32> ( V_MAX_PGCQUEUE_COUNT, pgcCount );
		assert ( pgcCount );
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), pgcId, pgcCount, &priority );
		v_device->queueFamilyProps[pgcId].queueCount -= pgcCount;
		currentIndex++;
	} else if ( pgId != ( u32 ) - 1 ) {
		pgcCount = std::min<u32> ( v_device->queueFamilyProps[pgId].queueCount, v_device->queueFamilyProps[cId].queueCount );
		pgcCount = std::min<u32> ( V_MAX_PGCQUEUE_COUNT, pgcCount );
		assert ( pgcCount );
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), pgId, pgcCount, &priority );
		v_device->queueFamilyProps[pgId].queueCount -= pgcCount;
		currentIndex++;

		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), cId, pgcCount, &priority );
		v_device->queueFamilyProps[cId].queueCount -= pgcCount;
		currentIndex++;
	} else {
		pgcCount = std::min<u32> ( v_device->queueFamilyProps[pId].queueCount,
		                           std::min<u32> ( v_device->queueFamilyProps[gId].queueCount, v_device->queueFamilyProps[cId].queueCount ) );
		pgcCount = std::min<u32> ( V_MAX_PGCQUEUE_COUNT, pgcCount );
		assert ( pgcCount );
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), pId, pgcCount, &priority );
		v_device->queueFamilyProps[pId].queueCount -= pgcCount;
		currentIndex++;

		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), gId, pgcCount, &priority );
		v_device->queueFamilyProps[gId].queueCount -= pgcCount;
		currentIndex++;

		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), cId, pgcCount, &priority );
		v_device->queueFamilyProps[cId].queueCount -= pgcCount;
		currentIndex++;
	}

	printf ( "Create %d Present-Graphics-Compute Queues\n", pgcCount );

	vk::PhysicalDeviceFeatures physicalDeviceFeatures;
	physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
	physicalDeviceFeatures.multiDrawIndirect = VK_TRUE;
	physicalDeviceFeatures.fillModeNonSolid = VK_TRUE;// for only mesh rendering

	vk::DeviceCreateInfo deviceCreateInfo ( vk::DeviceCreateFlags(), queueFamilyCount, deviceQueueCreateInfos,
	                                        extLayers.neededLayers.size(), extLayers.neededLayers.data(),
	                                        extLayers.neededExtensions.size(), extLayers.neededExtensions.data(),
	                                        &physicalDeviceFeatures );

	V_CHECKCALL ( v_device->physical_device.createDevice ( &deviceCreateInfo, nullptr, &m_device ), printf ( "Device Creation Failed\n" ) );

	if ( queues.dedicated_transfer_queue ) {
		m_device.getQueue ( queues.transfer_queue_id, 0, &queues.transfer_queue );
	}
	for ( size_t i = 0; i < pgcCount; ++i ) {
		PGCQueueWrapper queue_wrapper;
		if ( pgcId != ( u32 ) - 1 ) {
			m_device.getQueue ( pgcId, i, &queue_wrapper.graphics_queue );
			queue_wrapper.present_queue = queue_wrapper.graphics_queue;
			queue_wrapper.compute_queue = queue_wrapper.graphics_queue;
		} else if ( pgId != ( u32 ) - 1 ) {
			m_device.getQueue ( pgcId, i, &queue_wrapper.graphics_queue );
			m_device.getQueue ( pgcId, i, &queue_wrapper.compute_queue );
			queue_wrapper.present_queue = queue_wrapper.graphics_queue;
		} else {
			m_device.getQueue ( gId, i, &queue_wrapper.graphics_queue );
			m_device.getQueue ( pId, i, &queue_wrapper.present_queue );
			m_device.getQueue ( cId, i, &queue_wrapper.compute_queue );
		}
		queue_wrapper.graphics_queue_id = gId;
		queue_wrapper.present_queue_id = pId;
		queue_wrapper.compute_queue_id = cId;
		queue_wrapper.combined_graphics_present_queue = ( pId == gId );
		queue_wrapper.combined_graphics_compute_queue = ( gId == cId );
		queues.pgc.push_back ( queue_wrapper );
	}

	transfer_control = new VulkanTransferController ( this );
	return true;
}

Array<Monitor*>& VulkanInstance::get_monitors() {
	return monitors;
}
Array<Device*>& VulkanInstance::get_devices() {
	return devices;
}
void VulkanInstance::process_events() {
	glfwPollEvents();
}
void VulkanInstance::render_window(Window* window){
	v_render_window(dynamic_cast<VulkanWindow*>(window));
}
void VulkanInstance::v_render_window(VulkanWindow* window) {
	if(!window)
		return;
	window->v_update();
	frame_index++;
}
void VulkanInstance::render_windows(){
	for ( VulkanWindow* window : windows ) {
		window->v_update();
	}
	frame_index++;
}
bool VulkanInstance::is_window_open() {
	for ( Window* window : windows ) {
		if ( window->visible()->value )
			return true;
	}
	return false;
}

VulkanMonitor* VulkanInstance::get_primary_monitor_vulkan() {
	return monitor_map.find ( glfwGetPrimaryMonitor() )->second;
}
Monitor* VulkanInstance::get_primary_monitor() {
	return get_primary_monitor_vulkan();
}
Window* VulkanInstance::create_window() {
	VulkanWindow* window = new VulkanWindow ( this );
	windows.insert ( window );
	return window;
}
bool VulkanInstance::destroy_window ( Window* window ) {
	if ( VulkanWindow* vulk_window = dynamic_cast<VulkanWindow*> ( window ) ) {
		if ( windows.erase ( vulk_window ) > 0 ) {
			delete vulk_window;
			return true;
		} else {
			return false;
		}
	}
	return false;
}


static std::vector<char> readFile ( const char* filename ) {
	std::ifstream file ( filename, std::ios::ate | std::ios::binary );

	if ( !file.is_open() ) {
		printf ( "Couldn't open File %s\n", filename );
		return std::vector<char>();
	}

	size_t fileSize = ( size_t ) file.tellg();
	std::vector<char> buffer ( fileSize );

	file.seekg ( 0 );
	file.read ( buffer.data(), fileSize );
	file.close();

	return buffer;
}
RId VulkanInstance::register_datagroupdef ( DataGroupDef& datagroupdef ) {
	return datagroup_store.insert ( new DataGroupDef ( datagroupdef ) );
}
const DataGroupDef* VulkanInstance::datagroupdef ( RId id ) {
	return datagroup_store[id];
}

RId VulkanInstance::register_contextbase ( RId datagroup_id ) {
	return contextbase_store.insert ( new VulkanContextBase ( this, datagroup_store[datagroup_id] ) );
}
const ContextBase VulkanInstance::contextbase ( RId id ) {
	const VulkanContextBase* v_contextbase = contextbase_store.get ( id );
	return {v_contextbase->id, v_contextbase->datagroup->id};
}

RId VulkanInstance::register_modelbase ( RId vertexdatagroup ) {
	return modelbase_store.insert ( new VulkanModelBase ( vertexdatagroup ) );
}
const ModelBase VulkanInstance::modelbase ( RId id ) {
	const VulkanModelBase* v_modelbase = modelbase_store[id];
	return {v_modelbase->id, v_modelbase->datagroup};
}
RId VulkanInstance::register_modelinstancebase ( Model model, RId datagroup_id ) {
	VulkanModel* v_model = model_store[model.id];

	return modelinstancebase_store.insert ( new VulkanModelInstanceBase ( this, datagroup_store[datagroup_id], v_model, modelbase_store[v_model->modelbase] ) );
}
const ModelInstanceBase VulkanInstance::modelinstancebase ( RId handle ) {

}
const ModelInstance VulkanInstance::create_instance ( RId modelinstancebase ) {

}

vk::ShaderModule VulkanInstance::load_shader_from_file ( const char* filename ) {

	std::vector<char> shaderCode = readFile ( filename );

	vk::ShaderModuleCreateInfo createInfo ( vk::ShaderModuleCreateFlags(), shaderCode.size(), ( const u32* ) shaderCode.data() );

	vk::ShaderModule shadermodule;
	V_CHECKCALL ( vulkan_device ( this ).createShaderModule ( &createInfo, nullptr, &shadermodule ), printf ( "Creation of Shadermodule failed\n" ) );
	return shadermodule;
}
const Model VulkanInstance::load_generic_model ( RId modelbase, u8* vertices, u32 vertexcount, u16* indices, u32 indexcount ) {
	const VulkanModelBase* v_modelbase = modelbase_store[modelbase];
	const DataGroupDef* datagroupdef = datagroup_store[v_modelbase->datagroup];

	VulkanModel* newmodel = new VulkanModel ( this, modelbase );
	newmodel->modelbase = modelbase;
	newmodel->index_is_2byte = true;
	newmodel->vertexoffset = 0;
	newmodel->vertexcount = vertexcount;
	newmodel->indexoffset = 0;
	newmodel->indexcount = indexcount;
	newmodel->vertexbuffer.init ( vertexcount * datagroupdef->size,
	                              vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
	                              vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );
	newmodel->indexbuffer.init ( indexcount * sizeof ( u16 ),
	                             vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
	                             vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );

	std::pair<u8*, vk::Semaphore> vertexdata = transfer_control->request_transfer_memory ( &newmodel->vertexbuffer, vertexcount * datagroupdef->size );
	std::pair<u8*, vk::Semaphore> indexdata = transfer_control->request_transfer_memory ( &newmodel->indexbuffer, indexcount * sizeof ( u16 ) );
	memcpy ( ( void* ) vertexdata.first, ( void* ) vertices, vertexcount * datagroupdef->size );
	memcpy ( ( void* ) indexdata.first, ( void* ) indices, indexcount * sizeof ( u16 ) );
	transfer_control->do_transfers();
	transfer_control->check_free();
	return {model_store.insert ( newmodel ), modelbase};
}
const Model VulkanInstance::load_generic_model ( RId modelbase, u8* vertices, u32 vertexcount, u32* indices, u32 indexcount ) {
	const VulkanModelBase* v_modelbase = modelbase_store[modelbase];
	const DataGroupDef* datagroupdef = datagroup_store[v_modelbase->datagroup];
	bool fitsin2bytes = vertexcount < 0x10000;

	VulkanModel* newmodel = new VulkanModel ( this, modelbase );
	newmodel->modelbase = modelbase;
	newmodel->index_is_2byte = false;
	newmodel->vertexoffset = 0;
	newmodel->vertexcount = vertexcount;
	newmodel->indexoffset = 0;
	newmodel->indexcount = indexcount;
	newmodel->vertexbuffer.init ( vertexcount * datagroupdef->size,
	                              vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
	                              vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );
	newmodel->indexbuffer.init ( indexcount * sizeof ( u32 ),
	                             vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
	                             vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );

	std::pair<u8*, vk::Semaphore> vertexdata = transfer_control->request_transfer_memory ( &newmodel->vertexbuffer, vertexcount * datagroupdef->size );
	std::pair<u8*, vk::Semaphore> indexdata = transfer_control->request_transfer_memory ( &newmodel->indexbuffer, indexcount * sizeof ( u32 ) );
	memcpy ( ( void* ) vertexdata.first, ( void* ) vertices, vertexcount * datagroupdef->size );
	memcpy ( ( void* ) indexdata.first, ( void* ) indices, indexcount * sizeof ( u16 ) );
	transfer_control->do_transfers();
	transfer_control->check_free();
	return {model_store.insert ( newmodel ), modelbase};
}



GPUMemory VulkanInstance::allocate_gpu_memory ( vk::MemoryRequirements mem_req, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) {
	GPUMemory memory;

	memory.heap_index = findMemoryType ( mem_req.memoryTypeBits, needed | recommended );
	memory.property_flags = needed | recommended;
	if ( memory.heap_index == std::numeric_limits<u32>::max() ) {
		memory.heap_index = findMemoryType ( mem_req.memoryTypeBits, needed );
		memory.property_flags = needed;
	}
	if ( memory.heap_index != std::numeric_limits<u32>::max() ) {
		vk::MemoryAllocateInfo allocInfo ( mem_req.size, memory.heap_index );
		printf ( "Allocate %d Bytes\n", mem_req.size );

		V_CHECKCALL ( m_device.allocateMemory ( &allocInfo, nullptr, &memory.memory ), printf ( "Failed To Create Image Memory\n" ) );

		memory.size = mem_req.size;
	} else {
		printf ( "Cannot Allocate %d bytes Memory for Memtypes: 0x%x Needed Properties: 0x%x Recommended Properties: 0x%x\n",
		         mem_req.size, mem_req.memoryTypeBits,
		         static_cast<VkMemoryPropertyFlags> ( needed ), static_cast<VkMemoryPropertyFlags> ( recommended ) );
	}
	return memory;
}
RendResult VulkanInstance::free_gpu_memory ( GPUMemory memory ) {
	if ( memory.memory ) {
		printf ( "Freeing %d Bytes of Memory\n", memory.size );
		vulkan_device ( this ).freeMemory ( memory.memory, nullptr );
		memory.memory = vk::DeviceMemory();
	}
	return RendResult::eSuccess;
}
vk::ImageView VulkanInstance::createImageView2D ( vk::Image image, u32 mipBase, u32 mipOffset, vk::Format format, vk::ImageAspectFlags aspectFlags ) {

	vk::ImageViewCreateInfo imageViewCreateInfo (
	    vk::ImageViewCreateFlags(),
	    image,
	    vk::ImageViewType::e2D, format,
	    vk::ComponentMapping ( vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity ),
	    vk::ImageSubresourceRange ( aspectFlags, mipBase, mipOffset, 0, 1 )
	);

	vk::ImageView imageView;

	V_CHECKCALL ( m_device.createImageView ( &imageViewCreateInfo, nullptr, &imageView ), printf ( "Creation of ImageView failed\n" ) );

	return imageView;
}
vk::ImageView VulkanInstance::createImageView2DArray ( vk::Image image, u32 mipBase, u32 mipOffset, u32 arrayOffset, u32 arraySize, vk::Format format, vk::ImageAspectFlags aspectFlags ) {

	vk::ImageViewCreateInfo imageViewCreateInfo (
	    vk::ImageViewCreateFlags(),
	    image,
	    vk::ImageViewType::e2DArray, format,
	    vk::ComponentMapping ( vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity ),
	    vk::ImageSubresourceRange ( aspectFlags, mipBase, mipOffset, arrayOffset, arraySize )
	);

	vk::ImageView imageView;

	V_CHECKCALL ( m_device.createImageView ( &imageViewCreateInfo, nullptr, &imageView ), printf ( "Creation of ImageView failed\n" ) );

	return imageView;
}
void VulkanInstance::destroyImageView ( vk::ImageView imageview ) {
	m_device.destroyImageView ( imageview );
}

void VulkanInstance::destroyCommandPool ( vk::CommandPool commandPool ) {
	vkDestroyCommandPool ( m_device, commandPool, nullptr );
}
vk::CommandBuffer VulkanInstance::createCommandBuffer ( vk::CommandPool commandPool, vk::CommandBufferLevel bufferLevel ) {
	vk::CommandBufferAllocateInfo allocateInfo ( commandPool, bufferLevel, 1 );

	vk::CommandBuffer commandBuffer;
	m_device.allocateCommandBuffers ( &allocateInfo, &commandBuffer );
	return commandBuffer;
}
void VulkanInstance::deleteCommandBuffer ( vk::CommandPool commandPool, vk::CommandBuffer commandBuffer ) {
	m_device.freeCommandBuffers ( commandPool, 1, &commandBuffer );
}

void VulkanInstance::copyData ( const void* srcData, vk::DeviceMemory dstMemory, vk::DeviceSize offset, vk::DeviceSize size ) {
	void* data;
	vkMapMemory ( m_device, dstMemory, offset, size, 0, &data );
	memcpy ( data, srcData, size );
	vkUnmapMemory ( m_device, dstMemory );
}

u32 VulkanInstance::findMemoryType ( u32 typeFilter, vk::MemoryPropertyFlags properties ) {
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

vk::Format VulkanInstance::findSupportedFormat ( const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features ) {
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

vk::Format VulkanInstance::findDepthFormat() {
	return findSupportedFormat (
	{vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat},
	vk::ImageTiling::eOptimal,
	vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);
}
