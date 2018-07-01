#include "VulkanWindow.h"
#include "VulkanInstance.h"



VulkanWindow::VulkanWindow ( VulkanInstance* instance ) : m_instance ( instance ) {

}
VulkanWindow::~VulkanWindow() {
	m_instance->destroy_window ( this );
}

RendResult VulkanWindow::root_section ( WindowSection* section ) {
	VulkanWindowSection* root_section = dynamic_cast<VulkanWindowSection*> ( section );
	if ( root_section ) {
		if ( root_section->v_instance != m_instance )
			return RendResult::eWrongInstance;
		if ( m_root_section ) {
			m_root_section->unregister_parent();
			RendResult res;
			if ( ( res = root_section->register_parent ( this, nullptr ) ) != RendResult::eSuccess ) {
				return res;
			}
			m_root_section->v_update_viewport ( Viewport<f32> (), nullptr );
		}
		m_root_section = root_section;
		if ( swap_chain ) {
			m_root_section->v_update_viewport ( Viewport<f32> ( 0.0f, 0.0f, framebuffer_size.x, framebuffer_size.y, -1.0f, 1.0f ), &v_render_target_wrapper );
		}
	} else if ( !section ) {//nullptr passed
		if ( m_root_section ) {//reset the current section
			m_root_section->v_update_viewport ( Viewport<f32> (), nullptr );
			m_root_section = nullptr;
		}
	} else {
		return RendResult::eWrongType;
	}
	return RendResult::eSuccess;
}
WindowSection* VulkanWindow::root_section () {
	return m_root_section;
}
void VulkanWindow::initialize() {
	glfwWindowHint ( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint ( GLFW_MAXIMIZED, ( bool ) m_maximized.wanted );
	glfwWindowHint ( GLFW_AUTO_ICONIFY, ( bool ) m_minimized.wanted );
	glfwWindowHint ( GLFW_FOCUSED, ( bool ) m_focused.wanted );
	glfwWindowHint ( GLFW_DECORATED, ( bool ) m_decorated.wanted );
	glfwWindowHint ( GLFW_VISIBLE, false );
	glfwWindowHint ( GLFW_RESIZABLE, ( bool ) m_resizable.wanted );

	VulkanMonitor* fullscreen_monitor = dynamic_cast<VulkanMonitor*> ( this->m_fullscreen_monitor.wanted );
	if ( fullscreen_monitor ) { //it is fullscreen
		VideoMode wanted_mode = fullscreen_monitor->find_best_videomode ( m_size.wanted, m_refreshrate.wanted );
		m_size = wanted_mode.extend;
		m_refreshrate = wanted_mode.refresh_rate;
		glfwWindowHint ( GLFW_REFRESH_RATE, m_refreshrate.wanted );

		m_refreshrate.apply();
		printf ( "Fullscreen Videomode %dx%d %dHz\n", wanted_mode.extend.width, wanted_mode.extend.height, wanted_mode.refresh_rate );
	}

	window = glfwCreateWindow ( m_size.wanted.x, m_size.wanted.y, "Vulkan Test", fullscreen_monitor ? fullscreen_monitor->monitor : nullptr, nullptr );

	glfwSetWindowPos ( window, m_position.wanted.x, m_position.wanted.y );

	VCHECKCALL ( glfwCreateWindowSurface ( m_instance->m_instance, window, nullptr, ( VkSurfaceKHR* ) &surface ), printf ( "Creation of Surface failed" ) );

	glfwSetWindowUserPointer ( window, this );

	m_instance->window_map.insert ( std::make_pair ( window, this ) );
	m_visible.apply();
	m_position.apply();
	m_size.apply();

	glfwSetWindowPosCallback ( window, [] ( GLFWwindow * window, int x, int y ) {
		VulkanWindow* vulkan_window = static_cast<VulkanWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			printf ( "Position of Window %dx%d\n", x, y );
			vulkan_window->m_position.apply ( {x, y} );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetWindowSizeCallback ( window, [] ( GLFWwindow * window, int x, int y ) {
		VulkanWindow* vulkan_window = static_cast<VulkanWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			printf ( "Size of Window %dx%d\n", x, y );
			vulkan_window->m_size.apply ( {x, y} );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetWindowCloseCallback ( window, [] ( GLFWwindow * window ) {
		VulkanWindow* vulkan_window = static_cast<VulkanWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			glfwHideWindow ( window );
			vulkan_window->m_visible.apply ( false );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetWindowRefreshCallback ( window, [] ( GLFWwindow * window ) {
		VulkanWindow* vulkan_window = static_cast<VulkanWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			//@TODO implement???
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetWindowFocusCallback ( window, [] ( GLFWwindow * window, int focus ) {
		VulkanWindow* vulkan_window = static_cast<VulkanWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			vulkan_window->m_focused.apply ( focus == GLFW_TRUE );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetWindowIconifyCallback ( window, [] ( GLFWwindow * window, int iconified ) {
		VulkanWindow* vulkan_window = static_cast<VulkanWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			vulkan_window->m_minimized.apply ( iconified == GLFW_TRUE );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetFramebufferSizeCallback ( window, [] ( GLFWwindow * window, int x, int y ) {
		VulkanWindow* vulkan_window = static_cast<VulkanWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			vulkan_window->framebuffer_size_changed ( {x, y} );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetKeyCallback ( window, [] ( GLFWwindow * window, int key, int scancode, int action, int mods ) {
		VulkanWindow* vulkan_window = static_cast<VulkanWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			switch ( action ) {
			case GLFW_PRESS:
				printf ( "Press " );
				break;
			case GLFW_REPEAT:
				printf ( "Repeat " );
				break;
			case GLFW_RELEASE:
				printf ( "Release " );
				break;
			default:
				printf ( "Unknown Action " );
				break;
			}
			printf ( "%d ", key );
			if ( mods & GLFW_MOD_SHIFT )
				printf ( "Shift " );
			if ( mods & GLFW_MOD_CONTROL )
				printf ( "Cntrl " );
			if ( mods & GLFW_MOD_ALT )
				printf ( "Alt " );
			if ( mods & GLFW_MOD_SUPER )
				printf ( "Super " );
			printf ( "\n" );

		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetCharModsCallback ( window, [] ( GLFWwindow * window, unsigned int codepoint, int mods ) {
		VulkanWindow* vulkan_window = static_cast<VulkanWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			printf ( "%lc ", ( wint_t ) codepoint );
			if ( mods & GLFW_MOD_SHIFT )
				printf ( "Shift " );
			if ( mods & GLFW_MOD_CONTROL )
				printf ( "Cntrl " );
			if ( mods & GLFW_MOD_ALT )
				printf ( "Alt " );
			if ( mods & GLFW_MOD_SUPER )
				printf ( "Super " );
			printf ( "\n" );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetCursorPosCallback ( window, [] ( GLFWwindow * window, double xpos, double ypos ) {
		VulkanWindow* vulkan_window = static_cast<VulkanWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			//printf ( "Mouse Position %f, %f\n", xpos,  ypos);
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetCursorEnterCallback ( window, [] ( GLFWwindow * window, int entered ) {
		VulkanWindow* vulkan_window = static_cast<VulkanWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			if ( entered )
				printf ( "Mouse Entered\n" );
			else
				printf ( "Mouse Left\n" );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetMouseButtonCallback ( window, [] ( GLFWwindow * window, int button, int action, int mods ) {
		VulkanWindow* vulkan_window = static_cast<VulkanWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {

		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	//initialize Vulkan stuff

	image_available_guard_sem = create_semaphore ( m_instance );

	{
		u32 formatCount;
		vulkan_physical_device ( m_instance ).getSurfaceFormatsKHR ( surface, &formatCount, nullptr );
		if ( formatCount != 0 ) {
			vk::SurfaceFormatKHR formats[formatCount];
			vulkan_physical_device ( m_instance ).getSurfaceFormatsKHR ( surface, &formatCount, formats );

			present_swap_format = formats[0];

			if ( formatCount == 1 && formats[0].format == vk::Format::eUndefined ) {
				present_swap_format = {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
			}
			for ( size_t i = 0; i < formatCount; i++ ) {
				if ( formats[i].format == vk::Format::eB8G8R8A8Unorm && formats[i].colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear ) {
					present_swap_format = formats[i];
					break;
				}
			}
		}
	}
	chosen_presentation_mode = vk::PresentModeKHR::eFifo;//can be turned into global flags of what is supported
	{
		u32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR ( vulkan_physical_device ( m_instance ), surface, &presentModeCount, nullptr );
		if ( presentModeCount != 0 ) {
			vk::PresentModeKHR presentModes[presentModeCount];
			vulkan_physical_device ( m_instance ).getSurfacePresentModesKHR ( surface, &presentModeCount, presentModes );
			chosen_presentation_mode = vk::PresentModeKHR::eFifo;
			for ( size_t i = 0; i < presentModeCount; i++ ) {
				if ( presentModes[i] == vk::PresentModeKHR::eMailbox ) {
					chosen_presentation_mode = vk::PresentModeKHR::eMailbox;
					break;
				} else if ( presentModes[i] == vk::PresentModeKHR::eImmediate ) {
					chosen_presentation_mode = vk::PresentModeKHR::eImmediate;
				}
			}
		}
	}

	if ( m_visible.wanted ) {
		glfwShowWindow ( window );
	}

	framebuffer_size_changed ( m_size.value );

}
RendResult VulkanWindow::update() {
	static double oldTime = 0.0;
	double newTime = glfwGetTime();
	//printf("Time: %f\n", 1.0 / (newTime - oldTime));
	oldTime = newTime;
	if ( window ) {
		while ( true ) { //just so we don't need a goto for this logic ;-)
			//not visible
			if ( m_visible.changed() ) {
				if ( m_visible.wanted ) {
					glfwShowWindow ( window );
				} else {
					glfwHideWindow ( window );
				}
				m_visible.apply();
			}
			//minimized
			if ( m_minimized.changed() && m_minimized.wanted ) {
				glfwIconifyWindow ( window );
				break;
			}
			//maximized and not fullscreen
			else if ( m_maximized.changed() && m_maximized.wanted && !m_fullscreen_monitor.wanted ) {
				glfwMaximizeWindow ( window );
				break;
			}
			//fullscreen, window mode cnd coming out of minimized or maximized
			else if ( m_minimized.changed() || m_maximized.changed() || m_fullscreen_monitor.changed() ) {
				VulkanMonitor* vulkan_monitor = dynamic_cast<VulkanMonitor*> ( m_fullscreen_monitor.wanted );
				if ( vulkan_monitor ) {
					VideoMode wanted_mode = vulkan_monitor->find_best_videomode ( m_size.wanted, m_refreshrate.wanted );
					m_size = wanted_mode.extend;
					m_refreshrate = wanted_mode.refresh_rate;
				}
				glfwSetWindowMonitor ( window, vulkan_monitor ? vulkan_monitor->monitor : nullptr, m_position.wanted.x, m_position.wanted.y, m_size.wanted.x, m_size.wanted.y, m_refreshrate.wanted );
				m_fullscreen_monitor.apply();
				break;
			}
			if ( m_size.changed() ) {
				glfwSetWindowSize ( window, m_size.wanted.x, m_size.wanted.y );
			}
			if ( m_position.changed() ) {
				glfwSetWindowPos ( window, m_position.wanted.x, m_position.wanted.y );
			}
			break;
		}

		if ( m_cursormode.changed() ) {
			switch ( m_cursormode.wanted ) {
			case CursorMode::eNormal:
				glfwSetInputMode ( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
				break;
			case CursorMode::eInvisible:
				glfwSetInputMode ( window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN );
				break;
			case CursorMode::eCatch:
				glfwSetInputMode ( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
				break;
			}
			m_cursormode.apply();
		}
	} else {
		initialize();
	}
	render_frame();
}
void VulkanWindow::create_command_buffers() {
	if ( !window_graphics_command_pool ) {
		vk::CommandPoolCreateInfo createInfo ( vk::CommandPoolCreateFlagBits::eResetCommandBuffer, vulkan_pgc_queue_wrapper ( m_instance, queue_index )->graphics_queue_id );
		vulkan_device ( m_instance ).createCommandPool ( &createInfo, nullptr, &window_graphics_command_pool );
	}
	vk::CommandBufferAllocateInfo allocateInfo ( window_graphics_command_pool, vk::CommandBufferLevel::ePrimary, 2 );

	for ( FrameLocalData& data : frame_local_data ) {
		// create Command Buffers
		vk::CommandBuffer buffers[2];
		if ( data.clear_command_buffer && data.present_command_buffer ) {
			data.clear_command_buffer.reset ( vk::CommandBufferResetFlags() );
			data.present_command_buffer.reset ( vk::CommandBufferResetFlags() );
		} else {
			vulkan_device ( m_instance ).allocateCommandBuffers ( &allocateInfo, buffers );
			data.clear_command_buffer = buffers[0];
			data.present_command_buffer = buffers[1];
		}
		data.clear_command_buffer.begin ( vk::CommandBufferBeginInfo() );
		data.present_image->transition_image_layout ( vk::ImageLayout::eTransferDstOptimal, data.clear_command_buffer );
		data.clear_command_buffer.clearColorImage (
		    data.present_image->image,
		    vk::ImageLayout::eTransferDstOptimal,
		vk::ClearColorValue ( std::array<float, 4> ( {1.0f, 0.0f, 0.0f, 0.0f} ) ),
		{vk::ImageSubresourceRange ( data.present_image->aspectFlags, 0, 1, 0, 1 ) }
		);
		data.present_image->transition_image_layout ( vk::ImageLayout::eColorAttachmentOptimal, data.clear_command_buffer );

		depth_image->transition_image_layout ( vk::ImageLayout::eTransferDstOptimal, data.clear_command_buffer );

		data.clear_command_buffer.clearDepthStencilImage (
		    depth_image->image,
		    vk::ImageLayout::eTransferDstOptimal,
		    vk::ClearDepthStencilValue ( 0.0f, 0 ),
		{vk::ImageSubresourceRange ( depth_image->aspectFlags, 0, 1, 0, 1 ) }
		);
		depth_image->transition_image_layout ( vk::ImageLayout::eDepthStencilAttachmentOptimal, data.clear_command_buffer );
		data.clear_command_buffer.end();

		data.present_command_buffer.begin ( vk::CommandBufferBeginInfo() );
		data.present_image->transition_image_layout ( vk::ImageLayout::ePresentSrcKHR, data.present_command_buffer );
		data.present_command_buffer.end();
	}

}

void VulkanWindow::render_frame() {
	if ( m_minimized.value )
		return;

	vulkan_device ( m_instance ).acquireNextImageKHR ( swap_chain, std::numeric_limits<u64>::max(), image_available_guard_sem, vk::Fence(), &present_image_index );
	FrameLocalData* data = &frame_local_data[present_image_index];
	//reset for frame
	vulkan_device ( m_instance ).waitForFences ( {data->image_presented_fence}, true, std::numeric_limits<u64>::max() );
	vulkan_device ( m_instance ).resetFences ( {data->image_presented_fence} );

	PGCQueueWrapper* pgc_queue_wrapper = m_instance->vulkan_pgc_queue ( queue_index );

	data->initialized = true;

	data->sem_wait_needed.clear();

	//clear image or do other stuff
	//data->clear_command_buffer

	//draw child stuff

	//present image

	vk::SwapchainKHR swapChains[] = {swap_chain};


	vk::PipelineStageFlags waitDstStageMask ( vk::PipelineStageFlagBits::eBottomOfPipe );

	{
		vk::SubmitInfo submitInfo;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &image_available_guard_sem;
		submitInfo.pWaitDstStageMask = &waitDstStageMask;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &data->render_ready_sem;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &data->clear_command_buffer;
		pgc_queue_wrapper->graphics_queue.submit ( {submitInfo}, vk::Fence() );
	}
	v_render_target_wrapper.color_format = data->present_image->format;
	v_render_target_wrapper.depth_stencil_format = depth_image->format;

	data->sem_wait_needed.push_back ( data->render_ready_sem );
	if ( ( bool ) m_visible ) {
		if ( m_root_section ) {
			m_root_section->render_frame();
		} else {

		}
	}

	{
		vk::SubmitInfo submitInfo;
		submitInfo.waitSemaphoreCount = data->sem_wait_needed.size();
		submitInfo.pWaitSemaphores = data->sem_wait_needed.data();
		submitInfo.pWaitDstStageMask = &waitDstStageMask;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &data->present_ready_sem;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &data->present_command_buffer;
		pgc_queue_wrapper->graphics_queue.submit ( {submitInfo}, data->image_presented_fence );
	}

	if ( !pgc_queue_wrapper->combined_graphics_present_queue ) {
		//@TODO synchronize
	}

	vk::PresentInfoKHR presentInfo ( 1, &data->present_ready_sem, 1, swapChains, &present_image_index, nullptr );
	pgc_queue_wrapper->present_queue.presentKHR ( &presentInfo );
}
void VulkanWindow::create_swapchain() {

	//needs to be done first, because it waits for the fences to finish, which empties the graphics/presentation queue
	destroy_frame_local_data();

	capabilities = vulkan_physical_device ( m_instance ).getSurfaceCapabilitiesKHR ( surface );

	Extent2D<u32> old_swap_chain_extend = swap_chain_extend;
	{
		Extent2D<s32> actualExtent = m_size.value;
		if ( capabilities.currentExtent.width != std::numeric_limits<u32>::max() ) {
			vk::Extent2D extend = capabilities.currentExtent;
			swap_chain_extend = {extend.width, extend.height};
		} else {
			swap_chain_extend.width = std::max<u32> ( capabilities.minImageExtent.width, std::min<u32> ( capabilities.maxImageExtent.width, actualExtent.width ) );
			swap_chain_extend.height = std::max<u32> ( capabilities.minImageExtent.height, std::min<u32> ( capabilities.maxImageExtent.height, actualExtent.height ) );
		}
	}
	image_buffer_count = std::max<u32> ( capabilities.minImageCount, MAX_PRESENTIMAGE_COUNT );
	if ( capabilities.maxImageCount > 0 ) {
		image_buffer_count = std::min<u32> ( capabilities.maxImageCount, MAX_PRESENTIMAGE_COUNT );
		printf ( "Present Image Counts: %d\n", image_buffer_count );
	}

	{
		vk::SwapchainCreateInfoKHR swapchainCreateInfo (
		    vk::SwapchainCreateFlagsKHR(),
		    surface,
		    image_buffer_count,
		    present_swap_format.format,
		    present_swap_format.colorSpace,
		    vk::Extent2D ( swap_chain_extend.x, swap_chain_extend.y ),
		    1,
		    vk::ImageUsageFlags() | vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
		    vk::SharingMode::eExclusive,
		    0,
		    nullptr,
		    vk::SurfaceTransformFlagBitsKHR::eIdentity,
		    vk::CompositeAlphaFlagBitsKHR::eOpaque,
		    vk::PresentModeKHR::eImmediate,
		    0,
		    swap_chain );
		PGCQueueWrapper* pgc_queue_wrapper = m_instance->vulkan_pgc_queue ( queue_index );
		if ( pgc_queue_wrapper->combined_graphics_present_queue ) {
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
			swapchainCreateInfo.queueFamilyIndexCount = 1; // Optional
			u32 queueFamilyIndices[] = {pgc_queue_wrapper->graphics_queue_id};
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices; // Optional
		} else {
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			u32 queueFamilyIndices[] = {pgc_queue_wrapper->present_queue_id, pgc_queue_wrapper->graphics_queue_id};
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		swapchainCreateInfo.preTransform = capabilities.currentTransform;//VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR??
		switch ( m_alphablend.value ) {
		case WindowAlphaBlend::eOpaque:
			swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
			break;
		case WindowAlphaBlend::eBlend:
			swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::ePostMultiplied;
			break;
		}
		swapchainCreateInfo.presentMode = chosen_presentation_mode;
		swapchainCreateInfo.clipped = VK_TRUE;//clip pixels that are behind other windows

		printf ( "SUPPORTED %d\n", vulkan_physical_device ( m_instance ).getSurfaceSupportKHR ( pgc_queue_wrapper->present_queue_id, surface ) );

		V_CHECKCALL ( vulkan_device ( m_instance ).createSwapchainKHR ( &swapchainCreateInfo, nullptr, &swap_chain ), printf ( "Creation of Swapchain failed\n" ) );
		vulkan_device ( m_instance ).destroySwapchainKHR ( swapchainCreateInfo.oldSwapchain );
	}

	swap_chain_extend = Extent2D<u32> ( capabilities.maxImageExtent.width, capabilities.maxImageExtent.height );

	{
		//create/recreate depth image
		destroy_depth_image();
		vk::Format depth_format = m_instance->findDepthFormat();

		vk::Extent3D extent ( swap_chain_extend.width, swap_chain_extend.height, 0 );
		depth_image = new VulkanImageWrapper ( m_instance, extent, 1, 1, depth_format, vk::ImageTiling::eOptimal, vk::ImageUsageFlags ( vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferDst ),
		                                       vk::ImageAspectFlags() | vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, vk::MemoryPropertyFlags ( vk::MemoryPropertyFlagBits::eDeviceLocal ) );
		depth_image_view = m_instance->createImageView2D ( depth_image->image, 0, depth_image->mipMapLevels, depth_image->format, vk::ImageAspectFlags ( vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil ) );

	}
	create_frame_local_data ( vulkan_device ( m_instance ).getSwapchainImagesKHR ( swap_chain ) );

}
void VulkanWindow::framebuffer_size_changed ( Extent2D<s32> extend ) {
	printf ( "Size of Framebuffer %dx%d\n", extend.x, extend.y );
	framebuffer_size.x = extend.x;
	framebuffer_size.y = extend.y;
	printf ( "Minimized %d\n", m_minimized.value );
	printf ( "Visible %d\n", m_minimized.value );
	if ( extend.x > 0 && extend.y > 0 )
		create_swapchain();

	v_render_target_wrapper.color_format = frame_local_data[0].present_image->format;
	v_render_target_wrapper.depth_stencil_format = depth_image->format;
	v_render_target_wrapper.targetcount = frame_local_data.size();
	m_root_section->v_update_viewport ( Viewport<f32> ( 0.0f, 0.0f, framebuffer_size.x, framebuffer_size.y, -1.0f, 1.0f ), &v_render_target_wrapper );
}
void VulkanWindow::destroy_depth_image() {
	if ( depth_image_view ) {
		vulkan_device ( m_instance ).destroyImageView ( depth_image_view );
		depth_image_view = vk::ImageView();
	}
	if ( depth_image ) {
		depth_image->destroy();
		delete depth_image;
		depth_image = nullptr;
	}
}
void VulkanWindow::create_frame_local_data ( std::vector<vk::Image> swapChainImages ) {

	frame_local_data.resize ( swapChainImages.size() );
	for ( size_t i = 0; i < swapChainImages.size(); i++ ) {

		frame_local_data[i].image_presented_fence = vulkan_device ( m_instance ).createFence ( vk::FenceCreateFlags ( vk::FenceCreateFlagBits::eSignaled ) ); //image is ready
		frame_local_data[i].present_ready_sem = create_semaphore ( m_instance );
		frame_local_data[i].generated_sems.push_back ( frame_local_data[i].present_ready_sem );
		frame_local_data[i].render_ready_sem = create_semaphore ( m_instance );
		frame_local_data[i].generated_sems.push_back ( frame_local_data[i].render_ready_sem );

		frame_local_data[i].present_image = new VulkanImageWrapper ( m_instance, swapChainImages[i], {swap_chain_extend.x, swap_chain_extend.y, 0}, 1, 1, present_swap_format.format, vk::ImageTiling::eOptimal,
		        vk::ImageUsageFlags ( vk::ImageUsageFlagBits::eColorAttachment ), vk::ImageAspectFlags ( vk::ImageAspectFlagBits::eColor ) );

		frame_local_data[i].present_image_view = m_instance->createImageView2D ( swapChainImages[i], 0, 1, present_swap_format.format, vk::ImageAspectFlagBits::eColor );
	}
	create_command_buffers();
}
void VulkanWindow::destroy_frame_local_data() {
	printf ( "Wait For Queues to clear out\n" );
	m_instance->m_device.waitIdle();
	for ( FrameLocalData& data : frame_local_data ) {
		if ( data.image_presented_fence ) {
			//@TODO figure out why this freezes the Screen on Ubuntu/GNOME
			//vulkan_device ( m_instance ).waitForFences ( {data.image_presented_fence}, true, std::numeric_limits<u64>::max() );
			vulkan_device ( m_instance ).destroyFence ( data.image_presented_fence );
		}
		for ( vk::Semaphore sem : data.generated_sems ) {
			vulkan_device ( m_instance ).destroySemaphore ( sem );
		}
		delete data.present_image;
		if ( data.present_image_view )
			vulkan_device ( m_instance ).destroyImageView ( data.present_image_view );
	}
	//@Debugging clear so we assert in case we access it in a state, that we should not
	frame_local_data.clear();
}
RendResult VulkanWindow::destroy() {
	printf ( "Destroy Semaphores\n" );
	if ( image_available_guard_sem )
		destroy_semaphore ( m_instance, image_available_guard_sem );
	printf ( "Destroy Local Data\n" );
	destroy_frame_local_data();
	printf ( "Destroy Depth Image\n" );
	destroy_depth_image();
	printf ( "Destroy Swap Chain\n" );
	if ( swap_chain )
		vulkan_device ( m_instance ).destroySwapchainKHR ( swap_chain );
	printf ( "Destroy Command Pools\n" );
	if ( window_graphics_command_pool ) {
		vulkan_device ( m_instance ).destroyCommandPool ( window_graphics_command_pool );
		window_graphics_command_pool = vk::CommandPool();
	}
	printf ( "Destroyed Everything\n" );
}
