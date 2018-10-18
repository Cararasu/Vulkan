#include "VWindow.h"
#include "VInstance.h"
#include "VQuadRenderer.h"



VWindow::VWindow ( VInstance* instance ) : m_instance ( instance ), quad_renderer ( new VQuadRenderer ( instance ) ), depth_image ( nullptr ) {

}
VWindow::~VWindow() {
	m_instance->destroy_window ( this );
	if ( depth_image )
		delete depth_image;
	if ( present_image )
		delete present_image;
	delete quad_renderer;
}

void VWindow::initialize() {
	glfwWindowHint ( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint ( GLFW_MAXIMIZED, ( bool ) m_maximized.wanted );
	glfwWindowHint ( GLFW_AUTO_ICONIFY, ( bool ) m_minimized.wanted );
	glfwWindowHint ( GLFW_FOCUSED, ( bool ) m_focused.wanted );
	glfwWindowHint ( GLFW_DECORATED, ( bool ) m_decorated.wanted );
	glfwWindowHint ( GLFW_VISIBLE, false );
	glfwWindowHint ( GLFW_RESIZABLE, ( bool ) m_resizable.wanted );

	VMonitor* fullscreen_monitor = dynamic_cast<VMonitor*> ( this->m_fullscreen_monitor.wanted );
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
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			printf ( "Position of Window %dx%d\n", x, y );
			vulkan_window->m_position.apply ( {x, y} );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetWindowSizeCallback ( window, [] ( GLFWwindow * window, int x, int y ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			printf ( "Size of Window %dx%d\n", x, y );
			vulkan_window->m_size.apply ( {x, y} );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetWindowCloseCallback ( window, [] ( GLFWwindow * window ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			glfwHideWindow ( window );
			vulkan_window->m_visible.apply ( false );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetWindowRefreshCallback ( window, [] ( GLFWwindow * window ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			//@Remove when rendering is delegated into a different thread
			printf ( "Refresh\n" );
			vulkan_window->m_instance->render_window ( vulkan_window );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetWindowFocusCallback ( window, [] ( GLFWwindow * window, int focus ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			vulkan_window->m_focused.apply ( focus == GLFW_TRUE );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetWindowIconifyCallback ( window, [] ( GLFWwindow * window, int iconified ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			vulkan_window->m_minimized.apply ( iconified == GLFW_TRUE );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetFramebufferSizeCallback ( window, [] ( GLFWwindow * window, int x, int y ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			vulkan_window->framebuffer_size_changed ( {x, y} );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetKeyCallback ( window, [] ( GLFWwindow * window, int key, int scancode, int action, int mods ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
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
			if ( mods & GLFW_MOD_SHIFT )   printf ( "Shift " );
			if ( mods & GLFW_MOD_CONTROL ) printf ( "Cntrl " );
			if ( mods & GLFW_MOD_ALT )     printf ( "Alt " );
			if ( mods & GLFW_MOD_SUPER )   printf ( "Super " );
			printf ( "\n" );
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetCharModsCallback ( window, [] ( GLFWwindow * window, unsigned int codepoint, int mods ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
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
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			//printf ( "Mouse Position %f, %f\n", xpos,  ypos);
		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	glfwSetCursorEnterCallback ( window, [] ( GLFWwindow * window, int entered ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
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
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {

		} else {
			printf ( "No Window Registered For GLFW-Window\n" );
		}
	} );
	//initialize Vulkan stuff

	image_available_guard_sem = m_instance->create_semaphore ();

	{
		u32 formatCount;
		m_instance->vk_physical_device ().getSurfaceFormatsKHR ( surface, &formatCount, nullptr );
		if ( formatCount != 0 ) {
			vk::SurfaceFormatKHR formats[formatCount];
			m_instance->vk_physical_device ().getSurfaceFormatsKHR ( surface, &formatCount, formats );

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
			for ( size_t i = 0; i < formatCount; i++ ) {
				printf ( "Present Format: %s Colorspace: %s\n", to_string ( formats[i].format ).c_str(), to_string ( formats[i].colorSpace ).c_str() );
			}
		}
	}
	chosen_presentation_mode = vk::PresentModeKHR::eFifo;//can be turned into global flags of what is supported
	{
		u32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR ( m_instance->vk_physical_device(), surface, &presentModeCount, nullptr );
		if ( presentModeCount != 0 ) {
			vk::PresentModeKHR presentModes[presentModeCount];
			m_instance->vk_physical_device().getSurfacePresentModesKHR ( surface, &presentModeCount, presentModes );
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
Image* VWindow::backed_image () {
	return present_image;
}
RendResult VWindow::update() {
	RendResult res = v_update();
	m_instance->frame_index++;
	return res;
}
RendResult VWindow::v_update() {
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
				VMonitor* vulkan_monitor = dynamic_cast<VMonitor*> ( m_fullscreen_monitor.wanted );
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
void VWindow::create_command_buffers() {
	if ( !window_graphics_command_pool ) {
		vk::CommandPoolCreateInfo createInfo ( vk::CommandPoolCreateFlagBits::eResetCommandBuffer, m_instance->queues.graphics_queue_id );
		m_instance->vk_device().createCommandPool ( &createInfo, nullptr, &window_graphics_command_pool );
	}

}

void VWindow::create_command_buffer ( u32 index ) {

}

void VWindow::render_frame() {
	if ( m_minimized.value )
		return;

	v_logger.log<LogLevel::eWarn> ( "--------------- FrameBoundary %d ---------------", m_instance->frame_index );
	m_instance->vk_device().acquireNextImageKHR ( swap_chain, std::numeric_limits<u64>::max(), image_available_guard_sem, vk::Fence(), &present_image_index );

	v_logger.log<LogLevel::eDebug> ( "PresetImageId: %d", present_image_index );
	FrameLocalData* data = current_framelocal_data();

	v_logger.log<LogLevel::eWarn> ( "Waiting for Frame %d", data->frame_index );
	//reset for frame
	m_instance->vk_device().waitForFences ( {data->image_presented_fence}, true, std::numeric_limits<u64>::max() );

	m_instance->vk_device().resetFences ( {data->image_presented_fence} );

	data->frame_index = m_instance->frame_index;

	QueueWrapper* queue_wrapper = &m_instance->queues;

	present_image->set_current_image ( present_image_index );

	if ( data->clear_command_buffer ) {
		data->clear_command_buffer.reset ( vk::CommandBufferResetFlags() );
	}
	if ( data->present_command_buffer ) {
		data->present_command_buffer.reset ( vk::CommandBufferResetFlags() );
	}
	{
		vk::CommandBufferAllocateInfo allocateInfo ( window_graphics_command_pool, vk::CommandBufferLevel::ePrimary, 2 );
		// create Command Buffers
		vk::CommandBuffer buffers[2];
		if ( data->clear_command_buffer && data->present_command_buffer ) {
			data->clear_command_buffer.reset ( vk::CommandBufferResetFlags() );
			data->present_command_buffer.reset ( vk::CommandBufferResetFlags() );
		} else {
			m_instance->vk_device().allocateCommandBuffers ( &allocateInfo, buffers );
			data->clear_command_buffer = buffers[0];
			data->present_command_buffer = buffers[1];
		}
		data->clear_command_buffer.begin ( vk::CommandBufferBeginInfo() );
		present_image->transition_layout ( vk::ImageLayout::eTransferDstOptimal, data->clear_command_buffer );
		depth_image->transition_layout ( vk::ImageLayout::eTransferDstOptimal, data->clear_command_buffer );
		data->clear_command_buffer.clearColorImage (
		    present_image->image,
		    vk::ImageLayout::eTransferDstOptimal,
		vk::ClearColorValue ( std::array<float, 4> ( {1.0f, 0.0f, 0.0f, 0.0f} ) ),
		{vk::ImageSubresourceRange ( present_image->aspect, 0, 1, 0, 1 ) }
		);
		data->clear_command_buffer.clearDepthStencilImage (
		    depth_image->image,
		    vk::ImageLayout::eTransferDstOptimal,
		    vk::ClearDepthStencilValue ( 1.0f, 0 ),
		{vk::ImageSubresourceRange ( depth_image->aspect, 0, 1, 0, 1 ) }
		);
		present_image->transition_layout ( vk::ImageLayout::eColorAttachmentOptimal, data->clear_command_buffer );
		depth_image->transition_layout ( vk::ImageLayout::eDepthStencilAttachmentOptimal, data->clear_command_buffer );

		data->clear_command_buffer.end();

		data->present_command_buffer.begin ( vk::CommandBufferBeginInfo() );
		present_image->transition_layout ( vk::ImageLayout::ePresentSrcKHR, data->present_command_buffer );
		data->present_command_buffer.end();
	}

	data->initialized = true;

	vk::SwapchainKHR swapChains[] = {swap_chain};

	vk::PipelineStageFlags waitDstStageMask ( vk::PipelineStageFlagBits::eBottomOfPipe );

	SubmitStore submit_store;

	SubmitInfo si;
	si.wait_dst_stage_mask = vk::PipelineStageFlags ( vk::PipelineStageFlagBits::eTransfer );
	si.need_sem_index = submit_store.semaphores.size();
	si.need_sem_count = 1;
	submit_store.semaphores.push_back ( image_available_guard_sem );
	si.comm_buff_index = submit_store.commandbuffers.size();
	si.comm_buff_count = 1;
	submit_store.commandbuffers.push_back ( data->clear_command_buffer );
	si.sig_sem_index = submit_store.semaphores.size();
	si.sig_sem_count = 1;
	submit_store.semaphores.push_back ( data->render_ready_sem );
	submit_store.submitinfos.push_back ( si );

	u32 sem_index = 1;
	quad_renderer->render ( present_image_index, &submit_store, 1, &sem_index );

	si.wait_dst_stage_mask = vk::PipelineStageFlags ( vk::PipelineStageFlagBits::eColorAttachmentOutput );
	si.need_sem_index = sem_index;
	si.need_sem_count = 1;
	si.comm_buff_index = submit_store.commandbuffers.size();
	si.comm_buff_count = 1;
	submit_store.commandbuffers.push_back ( data->present_command_buffer );
	si.sig_sem_index = submit_store.semaphores.size();
	si.sig_sem_count = 1;
	submit_store.semaphores.push_back ( data->present_ready_sem );
	submit_store.submitinfos.push_back ( si );

	submit_store.signal_fence = data->image_presented_fence;

	DynArray<vk::SubmitInfo> submitinfos;
	for ( SubmitInfo& submit_info : submit_store.submitinfos ) {
		submitinfos.push_back (
		    vk::SubmitInfo (
		        submit_info.need_sem_count, submit_store.semaphores.data() + submit_info.need_sem_index, //waitSem
		        &submit_info.wait_dst_stage_mask,
		        submit_info.comm_buff_count, submit_store.commandbuffers.data() + submit_info.comm_buff_index,//commandbuffers
		        submit_info.sig_sem_count, submit_store.semaphores.data() + submit_info.sig_sem_index//signalSem
		    )
		);
	}
	queue_wrapper->graphics_queue.submit ( submitinfos, submit_store.signal_fence );

	if ( !queue_wrapper->combined_graphics_present_queue ) {
		//@TODO synchronize
	}
	//present image
	vk::Result results;
	vk::PresentInfoKHR presentInfo (
	    1, &data->present_ready_sem,
	    //active_sems.size(), active_sems.data(),
	    1, swapChains,
	    &present_image_index, &results );
	queue_wrapper->present_queue.presentKHR ( &presentInfo );
	active_sems.clear();
	v_logger.log<LogLevel::eDebug> ( "---------------   EndFrame    ---------------\n" );
}


void VWindow::create_swapchain() {

	//needs to be done first, because it waits for the fences to finish, which empties the graphics/presentation queue
	destroy_frame_local_data();

	capabilities = m_instance->vk_physical_device().getSurfaceCapabilitiesKHR ( surface );

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

	u32 image_buffer_count = std::max<u32> ( capabilities.minImageCount, MAX_PRESENTIMAGE_COUNT );
	if ( capabilities.maxImageCount > 0 ) {
		image_buffer_count = std::min<u32> ( capabilities.maxImageCount, MAX_PRESENTIMAGE_COUNT );
		printf ( "Present Image Counts: %d\n", image_buffer_count );
	}

	if ( depth_image ) {
		delete depth_image;
	}
	vk::Format depth_format = m_instance->find_depth_image_format();
	depth_image = new VImageWrapper ( m_instance, {swap_chain_extend.x, swap_chain_extend.y, 0}, 1, 1, depth_format, vk::ImageTiling::eOptimal, vk::ImageUsageFlags ( vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferDst ),
	                                  vk::ImageAspectFlags() | vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, vk::MemoryPropertyFlags ( vk::MemoryPropertyFlagBits::eDeviceLocal ) );

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
		QueueWrapper* queue_wrapper = &m_instance->queues;
		if ( queue_wrapper->combined_graphics_present_queue ) {
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
			swapchainCreateInfo.queueFamilyIndexCount = 1; // Optional
			u32 queueFamilyIndices[] = {queue_wrapper->graphics_queue_id};
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices; // Optional
		} else {
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			u32 queueFamilyIndices[] = {queue_wrapper->present_queue_id, queue_wrapper->graphics_queue_id};
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

		printf ( "SUPPORTED %d\n", m_instance->vk_physical_device().getSurfaceSupportKHR ( queue_wrapper->present_queue_id, surface ) );

		V_CHECKCALL ( m_instance->vk_device().createSwapchainKHR ( &swapchainCreateInfo, nullptr, &swap_chain ), printf ( "Creation of Swapchain failed\n" ) );
		m_instance->vk_device().destroySwapchainKHR ( swapchainCreateInfo.oldSwapchain );
	}

	if ( present_image ) {
		delete present_image;
		present_image = nullptr;
	}
	std::vector<vk::Image> images = m_instance->vk_device().getSwapchainImagesKHR ( swap_chain );
	present_image = new VWindowImage ( m_instance, images.size(), images.data(), {swap_chain_extend.x, swap_chain_extend.y, 0}, 1, present_swap_format.format );

	create_frame_local_data ( images.size() );

}
void VWindow::framebuffer_size_changed ( Extent2D<s32> extent ) {
	printf ( "Size of Framebuffer %dx%d\n", extent.x, extent.y );
	printf ( "Minimized %d\n", m_minimized.value );
	printf ( "Visible %d\n", m_minimized.value );

	if ( extent.x > 0 && extent.y > 0 )
		create_swapchain();
	printf ( "Actual Extent %dx%d\n", swap_chain_extend.x, swap_chain_extend.y );

	v_render_target_wrapper.images.resize ( 1 );
	v_render_target_wrapper.images[0] = present_image;
	v_render_target_wrapper.depth_image = depth_image;
	v_render_target_wrapper.target_count = present_image->per_image_data.size();

	quad_renderer->update_extend ( Viewport<f32> ( 0.0f, 0.0f, swap_chain_extend.x, swap_chain_extend.y, 0.0f, 1.0f ), &v_render_target_wrapper );

}

vk::CommandPool VWindow::graphics_command_pool() {
	if ( !frame_local_data[present_image_index].graphics_command_pool )
		frame_local_data[present_image_index].graphics_command_pool = m_instance->m_device.createCommandPool ( vk::CommandPoolCreateInfo() );
	return frame_local_data[present_image_index].graphics_command_pool;
}
void VWindow::create_frame_local_data ( u32 count ) {

	frame_local_data.resize ( count );
	for ( u32 i = 0; i < count; i++ ) {

		frame_local_data[i].image_presented_fence = m_instance->vk_device().createFence ( vk::FenceCreateFlags ( vk::FenceCreateFlagBits::eSignaled ) ); //image is ready
		frame_local_data[i].present_ready_sem = m_instance->create_semaphore();
		frame_local_data[i].render_ready_sem = m_instance->create_semaphore();
	}
	create_command_buffers();
}
void VWindow::destroy_frame_local_data() {
	printf ( "Wait For Queues to clear out\n" );
	m_instance->m_device.waitIdle();
	u32 index = 0;
	for ( FrameLocalData& data : frame_local_data ) {
		if ( data.image_presented_fence ) {
			//@TODO figure out why this freezes the Screen on Ubuntu/GNOME
			//m_instance->vk_device().waitForFences ( {data.image_presented_fence}, true, std::numeric_limits<u64>::max() );
			m_instance->vk_device().destroyFence ( data.image_presented_fence );
		}
		m_instance->destroy_semaphore(data.present_ready_sem);
		m_instance->destroy_semaphore(data.render_ready_sem);
		index++;
	}
	//@Debugging clear so we assert in case we access it in a state, that we should not
	frame_local_data.clear();
}
RendResult VWindow::destroy() {
	printf ( "Destroy Semaphores\n" );
	if ( image_available_guard_sem )
		m_instance->destroy_semaphore(image_available_guard_sem);
	printf ( "Destroy Local Data\n" );
	destroy_frame_local_data();
	printf ( "Destroy Swap Chain\n" );
	if ( swap_chain )
		m_instance->vk_device().destroySwapchainKHR ( swap_chain );
	printf ( "Destroy Command Pools\n" );
	if ( window_graphics_command_pool ) {
		m_instance->vk_device().destroyCommandPool ( window_graphics_command_pool );
		window_graphics_command_pool = vk::CommandPool();
	}
	printf ( "Destroyed Everything\n" );
}