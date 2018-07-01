#include "VulkanInstance.h"
#include "VulkanWindow.h"

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

	printf ( "%d\n", count );
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
			if ( memProperties.memoryTypes[i].propertyFlags | vk::MemoryPropertyFlagBits::eDeviceLocal )
				printf ( "\tDevice Local\n" );
			if ( memProperties.memoryTypes[i].propertyFlags | vk::MemoryPropertyFlagBits::eHostVisible )
				printf ( "\tHost Visible\n" );
			if ( memProperties.memoryTypes[i].propertyFlags | vk::MemoryPropertyFlagBits::eHostCoherent )
				printf ( "\tHost Coherent\n" );
			if ( memProperties.memoryTypes[i].propertyFlags | vk::MemoryPropertyFlagBits::eHostCached )
				printf ( "\tHost Cached\n" );
			if ( memProperties.memoryTypes[i].propertyFlags | vk::MemoryPropertyFlagBits::eLazilyAllocated )
				printf ( "\tLazily Allocated\n" );
		}
		for ( uint32_t i = 0; i < memProperties.memoryHeapCount; i++ ) {
			printf ( "Heap %d\n", i );
			printf ( "\tSize %I64u\n", memProperties.memoryHeaps[i].size );
			if ( memProperties.memoryHeaps[i].flags | vk::MemoryHeapFlagBits::eDeviceLocal )
				printf ( "\tDevice Local\n" );
			if ( memProperties.memoryHeaps[i].flags | vk::MemoryHeapFlagBits::eMultiInstanceKHX )
				printf ( "\tMulti Instance Visible\n" );
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
	
	v_resource_manager = new VulkanResourceManager(this);
	
	initialized = true;
}

VulkanInstance::~VulkanInstance() {
	for ( auto entry : monitor_map ) {
		delete entry.second;
	}
	for ( auto entry : window_map ) {
		destroy_window(entry.second);
	}
	m_device.destroy(nullptr);
	
	delete v_resource_manager;
}
bool VulkanInstance::initialize ( Device* device ) {
	if ( !device )
		device = devices[0];
	vulkan_device = dynamic_cast<VulkanDevice*> ( device );
	if ( !initialized && !vulkan_device )
		return false;
	VExtLayerStruct extLayers;
	extLayers.availableExtensions = vulkan_device->availableExtensions;
	extLayers.availableLayers = vulkan_device->availableLayers;
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
	for ( size_t j = 0; j < vulkan_device->queueFamilyProps.size(); ++j ) {
		u32 rating = 0;

		if ( vk::QueueFlags ( vulkan_device->queueFamilyProps[j].queueFlags ) == vk::QueueFlagBits::eTransfer && vulkan_device->queueFamilyProps[j].queueCount < 1 ) {
			queues.dedicated_transfer_queue = true;
			queues.transfer_queue_id = j;
		}
		bool present = glfwGetPhysicalDevicePresentationSupport ( m_instance, vulkan_device->physical_device, j );
		bool graphics = ( bool ) vk::QueueFlags ( vulkan_device->queueFamilyProps[j].queueFlags & vk::QueueFlagBits::eGraphics );
		bool compute = ( bool ) vk::QueueFlags ( vulkan_device->queueFamilyProps[j].queueFlags & vk::QueueFlagBits::eCompute );
		bool sparse = ( bool ) vk::QueueFlags ( vulkan_device->queueFamilyProps[j].queueFlags & vk::QueueFlagBits::eSparseBinding );
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
		vulkan_device->queueFamilyProps[queues.transfer_queue_id].queueCount--;
		currentIndex++;
		separateTransferQueue = true;
	}
	if ( pgcId != ( u32 ) - 1 ) {
		pgcCount = vulkan_device->queueFamilyProps[pgcId].queueCount;
		pgcCount = std::min<u32> ( V_MAX_PGCQUEUE_COUNT, pgcCount );
		assert ( pgcCount );
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), pgcId, pgcCount, &priority );
		vulkan_device->queueFamilyProps[pgcId].queueCount -= pgcCount;
		currentIndex++;
	} else if ( pgId != ( u32 ) - 1 ) {
		pgcCount = std::min<u32> ( vulkan_device->queueFamilyProps[pgId].queueCount, vulkan_device->queueFamilyProps[cId].queueCount );
		pgcCount = std::min<u32> ( V_MAX_PGCQUEUE_COUNT, pgcCount );
		assert ( pgcCount );
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), pgId, pgcCount, &priority );
		vulkan_device->queueFamilyProps[pgId].queueCount -= pgcCount;
		currentIndex++;

		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), cId, pgcCount, &priority );
		vulkan_device->queueFamilyProps[cId].queueCount -= pgcCount;
		currentIndex++;
	} else {
		pgcCount = std::min<u32> ( vulkan_device->queueFamilyProps[pId].queueCount,
		                           std::min<u32> ( vulkan_device->queueFamilyProps[gId].queueCount, vulkan_device->queueFamilyProps[cId].queueCount ) );
		pgcCount = std::min<u32> ( V_MAX_PGCQUEUE_COUNT, pgcCount );
		assert ( pgcCount );
		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), pId, pgcCount, &priority );
		vulkan_device->queueFamilyProps[pId].queueCount -= pgcCount;
		currentIndex++;

		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), gId, pgcCount, &priority );
		vulkan_device->queueFamilyProps[gId].queueCount -= pgcCount;
		currentIndex++;

		deviceQueueCreateInfos[currentIndex] = vk::DeviceQueueCreateInfo ( vk::DeviceQueueCreateFlags(), cId, pgcCount, &priority );
		vulkan_device->queueFamilyProps[cId].queueCount -= pgcCount;
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

	V_CHECKCALL ( vulkan_device->physical_device.createDevice ( &deviceCreateInfo, nullptr, &m_device ), printf ( "Device Creation Failed\n" ) );

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

	return true;
}

Array<Monitor*>& VulkanInstance::get_monitors() {
	return monitors;
}
Array<Device*>& VulkanInstance::get_devices() {
	return devices;
}
void VulkanInstance::process_events() {
	for ( Window* window : windows ) {
		window->update();
	}
	glfwPollEvents();
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
}

WindowSection* VulkanInstance::create_window_section ( WindowSectionType type ) {
	VulkanWindowSection* vulkan_section = nullptr;
	switch ( type ) {
	case WindowSectionType::eUI:
		vulkan_section = new UIVulkanWindowSection ( this );
	case WindowSectionType::eStack:
		break;
	case WindowSectionType::eWorld:
		break;
	}
	window_sections.insert ( vulkan_section );
	return vulkan_section;
}
bool VulkanInstance::destroy_window_section ( WindowSection* window_section ) {
	if ( VulkanWindowSection* vulk_window_sec = dynamic_cast<VulkanWindowSection*> ( window_section ) ) {
		if ( window_sections.erase ( vulk_window_sec ) > 0 ) {
			delete vulk_window_sec;
			return true;
		} else {
			return false;
		}
	}
}
ResourceManager* VulkanInstance::resource_manager() {
	return nullptr;
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

vk::DeviceMemory VulkanInstance::allocateMemory ( vk::MemoryRequirements memoryRequirement, vk::MemoryPropertyFlags properties ) {
	u32 memoryType = findMemoryType ( memoryRequirement.memoryTypeBits, properties );
	if ( memoryType == -1 )
		return vk::DeviceMemory();

	vk::MemoryAllocateInfo allocInfo ( memoryRequirement.size, memoryType );
	printf ( "Allocate %d Bytes\n", memoryRequirement.size );

	vk::DeviceMemory memory;
	V_CHECKCALL ( m_device.allocateMemory ( &allocInfo, nullptr, &memory ), printf ( "Failed To Create Image Memory\n" ) );
	return memory;
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
	vulkan_device->physical_device.getMemoryProperties ( &memProperties );

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
		vulkan_device->physical_device.getFormatProperties ( format, &props );
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
