#include "VulkanWindow.h"
#include "VulkanInstance.h"


void UIVulkanWindowSection::render_frame() {
	//printf ( "Render-UI Section\n" );
}

VulkanWindow::VulkanWindow (VulkanInstance* instance) : m_instance (instance) {

}
VulkanWindow::~VulkanWindow() {
	m_instance->destroy_window (this);
}

RendResult VulkanWindow::set_root_section (WindowSection* section) {
	VulkanWindowSection* root_section = dynamic_cast<VulkanWindowSection*> (section);
	if (root_section) {
		if (root_section->m_instance != m_instance)
			return RendResult::eWrongInstance;
		if (m_root_section) {
			m_root_section->unregister_parent();
		}
		RendResult res;
		if ( (res = root_section->register_parent (this, nullptr)) != RendResult::eSuccess) {
			return res;
		}
		m_root_section = root_section;
		m_root_section->update_viewport (Viewport<f32> (0.0f, 0.0f, framebuffer_size.x, framebuffer_size.y, 0.0f, 1.0f), nullptr);
	} else {
		return RendResult::eWrongType;
	}
}
RendResult VulkanWindow::update() {

	if (window) {
		while (true) {   //just so we don't need a goto for this logic ;-)
			//not visible
			if (m_visible.changed()) {
				if (m_visible.wanted) {
					glfwShowWindow (window);
				} else {
					glfwHideWindow (window);
				}
				m_visible.apply();
			}
			//minimized
			if (m_minimized.changed() && m_minimized.wanted) {
				glfwIconifyWindow (window);
				break;
			}
			//maximized and not fullscreen
			else if (m_maximized.changed() && m_maximized.wanted && !m_fullscreen_monitor.wanted) {
				glfwMaximizeWindow (window);
				break;
			}
			//fullscreen, window mode cnd coming out of minimized or maximized
			else if (m_minimized.changed() || m_maximized.changed() || m_fullscreen_monitor.changed()) {
				VulkanMonitor* vulkan_monitor = dynamic_cast<VulkanMonitor*> (m_fullscreen_monitor.wanted);
				if (vulkan_monitor) {
					VideoMode wanted_mode = vulkan_monitor->find_best_videomode (m_size.wanted, m_refreshrate.wanted);
					m_size = wanted_mode.extend;
					m_refreshrate = wanted_mode.refresh_rate;
				}
				glfwSetWindowMonitor (window, vulkan_monitor ? vulkan_monitor->monitor : nullptr, m_position.wanted.x, m_position.wanted.y, m_size.wanted.x, m_size.wanted.y, m_refreshrate.wanted);
				m_fullscreen_monitor.apply();
				break;
			}
			if (m_size.changed()) {
				glfwSetWindowSize (window, m_size.wanted.x, m_size.wanted.y);
			}
			if (m_position.changed()) {
				glfwSetWindowPos (window, m_position.wanted.x, m_position.wanted.y);
			}
			break;
		}

		if (m_cursormode.changed()) {
			switch (m_cursormode.wanted) {
			case CursorMode::eNormal:
				glfwSetInputMode (window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				break;
			case CursorMode::eInvisible:
				glfwSetInputMode (window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				break;
			case CursorMode::eCatch:
				glfwSetInputMode (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				break;
			}
			m_cursormode.apply();
		}
	}
	if (!window) {

		glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint (GLFW_MAXIMIZED, (bool) m_maximized.wanted);
		glfwWindowHint (GLFW_AUTO_ICONIFY, (bool) m_minimized.wanted);
		glfwWindowHint (GLFW_FOCUSED, (bool) m_focused.wanted);
		glfwWindowHint (GLFW_DECORATED, (bool) m_decorated.wanted);
		glfwWindowHint (GLFW_VISIBLE, (bool) m_visible.wanted);
		glfwWindowHint (GLFW_RESIZABLE, (bool) m_resizable.wanted);

		VulkanMonitor* fullscreen_monitor = dynamic_cast<VulkanMonitor*> (this->m_fullscreen_monitor.wanted);
		if (fullscreen_monitor) {   //it is fullscreen
			VideoMode wanted_mode = fullscreen_monitor->find_best_videomode (m_size.wanted, m_refreshrate.wanted);
			m_size = wanted_mode.extend;
			m_refreshrate = wanted_mode.refresh_rate;
			glfwWindowHint (GLFW_REFRESH_RATE, m_refreshrate.wanted);

			m_refreshrate.apply();
			printf ("Fullscreen Videomode %dx%d %dHz\n", wanted_mode.extend.width, wanted_mode.extend.height, wanted_mode.refresh_rate);
		}

		window = glfwCreateWindow (m_size.wanted.x, m_size.wanted.y, "Vulkan Test", fullscreen_monitor ? fullscreen_monitor->monitor : nullptr, nullptr);

		glfwSetWindowPos (window, m_position.wanted.x, m_position.wanted.y);

		VCHECKCALL (glfwCreateWindowSurface (m_instance->m_instance, window, nullptr, (VkSurfaceKHR*) &surface), printf ("Creation of Surface failed"));

		glfwSetWindowUserPointer (window, this);

		m_instance->window_map.insert (std::make_pair (window, this));
		m_visible.apply();
		m_position.apply();
		m_size.apply();

		glfwSetWindowPosCallback (window, [] (GLFWwindow * window, int x, int y) {
			VulkanWindow* vulkan_window = static_cast<VulkanWindow*> (glfwGetWindowUserPointer (window));
			if (vulkan_window) {
				printf ("Position of Window %dx%d\n", x, y);
				vulkan_window->m_position.apply ({x, y});
			} else {
				printf ("No Window Registered For GLFW-Window\n");
			}
		});
		glfwSetWindowSizeCallback (window, [] (GLFWwindow * window, int x, int y) {
			VulkanWindow* vulkan_window = static_cast<VulkanWindow*> (glfwGetWindowUserPointer (window));
			if (vulkan_window) {
				printf ("Size of Window %dx%d\n", x, y);
				vulkan_window->m_size.apply ({x, y});
			} else {
				printf ("No Window Registered For GLFW-Window\n");
			}
		});
		glfwSetWindowCloseCallback (window, [] (GLFWwindow * window) {
			VulkanWindow* vulkan_window = static_cast<VulkanWindow*> (glfwGetWindowUserPointer (window));
			if (vulkan_window) {
				glfwHideWindow (window);
				vulkan_window->m_visible.apply (false);
			} else {
				printf ("No Window Registered For GLFW-Window\n");
			}
		});
		glfwSetWindowRefreshCallback (window, [] (GLFWwindow * window) {
			VulkanWindow* vulkan_window = static_cast<VulkanWindow*> (glfwGetWindowUserPointer (window));
			if (vulkan_window) {
				//TODO implement???
			} else {
				printf ("No Window Registered For GLFW-Window\n");
			}
		});
		glfwSetWindowFocusCallback (window, [] (GLFWwindow * window, int focus) {
			VulkanWindow* vulkan_window = static_cast<VulkanWindow*> (glfwGetWindowUserPointer (window));
			if (vulkan_window) {
				vulkan_window->m_focused.apply (focus == GLFW_TRUE);
			} else {
				printf ("No Window Registered For GLFW-Window\n");
			}
		});
		glfwSetWindowIconifyCallback (window, [] (GLFWwindow * window, int iconified) {
			VulkanWindow* vulkan_window = static_cast<VulkanWindow*> (glfwGetWindowUserPointer (window));
			if (vulkan_window) {
				vulkan_window->m_minimized.apply (iconified == GLFW_TRUE);
			} else {
				printf ("No Window Registered For GLFW-Window\n");
			}
		});
		glfwSetFramebufferSizeCallback (window, [] (GLFWwindow * window, int x, int y) {
			VulkanWindow* vulkan_window = static_cast<VulkanWindow*> (glfwGetWindowUserPointer (window));
			if (vulkan_window) {
				vulkan_window->framebuffer_size_changed (x, y);
			} else {
				printf ("No Window Registered For GLFW-Window\n");
			}
		});

		//initialize Vulkan stuff

		image_available_guard_sem = create_semaphore (m_instance);

		{
			u32 formatCount;
			vulkan_physical_device (m_instance).getSurfaceFormatsKHR (surface, &formatCount, nullptr);
			printf("WWW %d\n", formatCount);
			if (formatCount != 0) {
				vk::SurfaceFormatKHR formats[formatCount];
				vulkan_physical_device (m_instance).getSurfaceFormatsKHR (surface, &formatCount, formats);

				present_swap_format = formats[0];

				if (formatCount == 1 && formats[0].format == vk::Format::eUndefined) {
					present_swap_format = {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
				}
				for (size_t i = 0; i < formatCount; i++) {
					if (formats[i].format == vk::Format::eB8G8R8A8Unorm && formats[i].colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
						present_swap_format = formats[i];
						break;
					}
				}
			}
		}
		chosen_presentation_mode = vk::PresentModeKHR::eFifo;//can be turned into global flags of what is supported
		{
			u32 presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR (vulkan_physical_device (m_instance), surface, &presentModeCount, nullptr);
			if (presentModeCount != 0) {
				vk::PresentModeKHR presentModes[presentModeCount];
				vulkan_physical_device (m_instance).getSurfacePresentModesKHR (surface, &presentModeCount, presentModes);
				chosen_presentation_mode = vk::PresentModeKHR::eFifo;
				for (size_t i = 0; i < presentModeCount; i++) {
					if (presentModes[i] == vk::PresentModeKHR::eMailbox) {
						chosen_presentation_mode = vk::PresentModeKHR::eMailbox;
						break;
					} else if (presentModes[i] == vk::PresentModeKHR::eImmediate) {
						chosen_presentation_mode = vk::PresentModeKHR::eImmediate;
					}
				}
			}
		}
		capabilities = vulkan_physical_device (m_instance).getSurfaceCapabilitiesKHR (surface);
		{
			vk::Extent2D actualExtent = {m_size.value.x, m_size.value.y};
			if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
				vk::Extent2D extend = capabilities.currentExtent;
				swap_chain_extend = {extend.width, extend.height};
			} else {
				swap_chain_extend.width = std::max (capabilities.minImageExtent.width, std::min (capabilities.maxImageExtent.width, actualExtent.width));
				swap_chain_extend.height = std::max (capabilities.minImageExtent.height, std::min (capabilities.maxImageExtent.height, actualExtent.height));
			}
		}
		image_buffer_count = std::max<u32> (capabilities.minImageCount, MAX_PRESENTIMAGE_COUNT);
		if (capabilities.maxImageCount > 0) {
			image_buffer_count = std::min<u32> (capabilities.maxImageCount, MAX_PRESENTIMAGE_COUNT);
			printf ("Present Image Counts: %d\n", image_buffer_count);
		}
		for (size_t i = 0; i < image_buffer_count; i++) {
			FrameLocalData frame_local;
			frame_local.image_presented_fence = vulkan_device(m_instance).createFence(vk::FenceCreateFlags());
			
			frame_local_data.push_back(frame_local);
		}
		framebuffer_size_changed (m_size.value.x, m_size.value.y);
		framestate = FrameState::eInitialized;

	}

	if ( (bool) m_visible && m_root_section) {
		m_root_section->render_frame();
	}
}
void VulkanWindow::create_swapchain() {

	for (auto& data : frame_local_data) {
		if (data.initialized && data.image_presented_fence) {
			vulkan_device (m_instance).waitForFences ({data.image_presented_fence}, true, std::numeric_limits<u64>::max());
			vulkan_device (m_instance).resetFences ({data.image_presented_fence});
		}
	}
	{
		vk::SwapchainCreateInfoKHR swapchainCreateInfo (
		    vk::SwapchainCreateFlagsKHR(),
		    surface,
		    image_buffer_count,
		    present_swap_format.format,
		    present_swap_format.colorSpace,
		    vk::Extent2D (swap_chain_extend.x, swap_chain_extend.y),
		    1,
		    vk::ImageUsageFlagBits::eColorAttachment,
		    vk::SharingMode::eExclusive,
		    0,
		    nullptr,
		    vk::SurfaceTransformFlagBitsKHR::eIdentity,
		    vk::CompositeAlphaFlagBitsKHR::eOpaque,
		    vk::PresentModeKHR::eImmediate,
		    0,
		    swap_chain);
		PGCQueueWrapper* pgc_queue_wrapper = m_instance->vulkan_pgc_queue (queue_index);
		if (pgc_queue_wrapper->combined_graphics_present_queue) {
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
		switch (m_alphablend.value) {
		case WindowAlphaBlend::eOpaque:
			swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
			break;
		case WindowAlphaBlend::eBlend:
			swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::ePostMultiplied;
			break;
		}
		swapchainCreateInfo.presentMode = chosen_presentation_mode;
		swapchainCreateInfo.clipped = VK_TRUE;//clip pixels that are behind other windows

		printf ("SUPPORTED %d\n", vulkan_physical_device(m_instance).getSurfaceSupportKHR (pgc_queue_wrapper->present_queue_id, surface));

		V_CHECKCALL (vulkan_device(m_instance).createSwapchainKHR (&swapchainCreateInfo, nullptr, &swap_chain), printf ("Creation of Swapchain failed\n"));
		vulkan_device(m_instance).destroySwapchainKHR (swapchainCreateInfo.oldSwapchain);
	}

	for (FrameLocalData& data : frame_local_data) {
		if (data.image_presented_fence) {
			vulkan_device(m_instance).destroyFence (data.image_presented_fence);
			data.image_presented_fence = vk::Fence();
		}
	}

	std::vector<vk::Image> swapChainImages = vulkan_device(m_instance).getSwapchainImagesKHR (swap_chain);
	
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		frame_local_data[i].present_image = swapChainImages[i];
		//frame_local_data[i].present_image_view = vulkan_device(m_instance).createImageView2D (data.swapChainImages[i], 0, 1, present_swap_format.format, vk::ImageAspectFlagBits::eColor);
		
	}
	swap_chain_extend = Extent2D<u32>(capabilities.maxImageExtent.width, capabilities.maxImageExtent.height);
	image_available_guard_sem_waited = false;
	vulkan_device(m_instance).acquireNextImageKHR (swap_chain, std::numeric_limits<u64>::max(), image_available_guard_sem, vk::Fence(), &present_image_index);

	{
		//create/recreate depth image
		destroy_depth_image();
		vk::Format depthFormat = m_instance->findDepthFormat();

		vk::Extent3D extent (swap_chain_extend.width, swap_chain_extend.height, 1);
		depth_image = new VulkanImageWrapper (m_instance, extent, 1, 1, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlags (vk::ImageUsageFlagBits::eDepthStencilAttachment),
		                               vk::ImageAspectFlagBits::eDepth, vk::MemoryPropertyFlags (vk::MemoryPropertyFlagBits::eDeviceLocal));
		depth_image_view = m_instance->createImageView2D (depth_image->image, 0, depth_image->mipMapLevels, depth_image->format, vk::ImageAspectFlags (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil));
		
		//depth_image->transitionImageLayout (depth_image->image, depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal,
		//                                 vk::ImageAspectFlags (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil), getCurrentGraphicsCommandPool(), pgcQueue->graphicsQueue);
		
	}
/*
	if (standardmodule.pipeline) {
		updateExtent (&instance->pipeline_module_builders.standard, &standardmodule, swapChainExtend);
	} else {
		standardmodule = createPipelineModule (&instance->pipeline_module_builders.standard, &instance->pipeline_module_layouts.standard, presentSwapFormat.format, swapChainExtend);
	}*/
	for (FrameLocalData& data : frame_local_data) {
		//data.presentImageView = instance->createImageView2D (data.presentImage, 0, 1, presentSwapFormat.format, vk::ImageAspectFlagBits::eColor);
		//data.firstShow = true;
		//vk::ImageView attachments[2] = {data.presentImageView, depthImageView};
		//vk::FramebufferCreateInfo framebufferInfo (vk::FramebufferCreateFlags(), standardmodule.renderPass, 2, attachments, swapChainExtend.width, swapChainExtend.height, 1);
		//data.framebuffer = vulkan_device(m_instance).createFramebuffer (framebufferInfo, nullptr);
	}
	FrameLocalData* data = &frame_local_data[present_image_index];

	PGCQueueWrapper* pgc_queue_wrapper = m_instance->vulkan_pgc_queue (queue_index);
	
	if(!image_available_guard_sem_waited){
		vk::SubmitInfo submitInfo;
		vk::PipelineStageFlags waitDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
		
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &image_available_guard_sem;
		submitInfo.pWaitDstStageMask = &waitDstStageMask;
		pgc_queue_wrapper->graphics_queue.submit({submitInfo}, data->image_presented_fence);
	}else{
		pgc_queue_wrapper->graphics_queue.submit({}, data->image_presented_fence);
	}
	
	//data->firstShow = false;
	//instance->device.waitForFences ({data->fence}, true, std::numeric_limits<u64>::max());
	//instance->device.resetFences ({data->fence});
}
void VulkanWindow::framebuffer_size_changed (s32 x, s32 y) {
	printf ("Size of Framebuffer %dx%d\n", x, y);
	framebuffer_size.x = x;
	framebuffer_size.y = y;
	if (m_root_section) {
		m_root_section->update_viewport (Viewport<f32> (0.0f, 0.0f, framebuffer_size.x, framebuffer_size.y, 0.0f, 1.0f), nullptr);
	}
	create_swapchain();
}
void VulkanWindow::destroy_depth_image(){
	if (depth_image_view) {
		vulkan_device(m_instance).destroyImageView (depth_image_view);
	}
	if(depth_image){
		depth_image->destroy();
		delete depth_image;
		depth_image = nullptr;
	}
}
RendResult VulkanWindow::destroy() {
	if (image_available_guard_sem)
		destroy_semaphore (m_instance, image_available_guard_sem);
	destroy_depth_image();
}
