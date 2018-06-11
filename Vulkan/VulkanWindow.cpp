#include "VulkanWindow.h"
#include "VulkanInstance.h"


void UIVulkanWindowSection::render_frame() {
	//printf ( "Render-UI Section\n" );
}

VulkanWindow::VulkanWindow ( VulkanInstance* instance ) : m_instance ( instance ) {

}
VulkanWindow::~VulkanWindow() {
	m_instance->destroy_window ( this );
}

RendResult VulkanWindow::set_root_section ( WindowSection* section ) {
	VulkanWindowSection* root_section = dynamic_cast<VulkanWindowSection*> ( section );
	if ( root_section ) {
		if ( root_section->m_instance != m_instance )
			return RendResult::eWrongInstance;
		if ( m_root_section ) {
			m_root_section->unregister_parent();
		}
		RendResult res;
		if ( ( res = root_section->register_parent ( this, nullptr ) ) != RendResult::eSuccess ) {
			return res;
		}
		m_root_section = root_section;
		m_root_section->update_viewport ( Viewport<f32> ( 0.0f, 0.0f, framebuffer_size.x, framebuffer_size.y, 0.0f, 1.0f ), nullptr );
	} else {
		return RendResult::eWrongType;
	}
}
RendResult VulkanWindow::update() {

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
	}
	if ( !window ) {

		glfwWindowHint ( GLFW_CLIENT_API, GLFW_NO_API );
		glfwWindowHint ( GLFW_MAXIMIZED, ( bool ) m_maximized.wanted );
		glfwWindowHint ( GLFW_AUTO_ICONIFY, ( bool ) m_minimized.wanted );
		glfwWindowHint ( GLFW_FOCUSED, ( bool ) m_focused.wanted );
		glfwWindowHint ( GLFW_DECORATED, ( bool ) m_decorated.wanted );
		glfwWindowHint ( GLFW_VISIBLE, ( bool ) m_visible.wanted );
		glfwWindowHint ( GLFW_RESIZABLE, ( bool ) m_resizable.wanted );

		VulkanMonitor* fullscreen_monitor = dynamic_cast<VulkanMonitor*> ( this->m_fullscreen_monitor.wanted );
		//force fullscreen
		//fullscreen_monitor = dynamic_cast<VulkanMonitor*> (this->m_instance->get_primary_monitor());
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
		framebuffer_size_changed ( m_size.value.x, m_size.value.y );

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
				//TODO implement???
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
				vulkan_window->framebuffer_size_changed ( x, y );
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
		capabilities = vulkan_physical_device ( m_instance ).getSurfaceCapabilitiesKHR ( surface );
		{
			vk::Extent2D actualExtent = {m_size.value.x, m_size.value.y};
			if ( capabilities.currentExtent.width != std::numeric_limits<u32>::max() ) {
				vk::Extent2D extend = capabilities.currentExtent;
				swap_chain_extend = {extend.width, extend.height};
			} else {
				swap_chain_extend.width = std::max ( capabilities.minImageExtent.width, std::min ( capabilities.maxImageExtent.width, actualExtent.width ) );
				swap_chain_extend.height = std::max ( capabilities.minImageExtent.height, std::min ( capabilities.maxImageExtent.height, actualExtent.height ) );
			}
		}
		u32 image_count = std::max<u32> ( capabilities.minImageCount, MAX_PRESENTIMAGE_COUNT );
		if ( capabilities.maxImageCount > 0 ) {
			image_count = std::min<u32> ( capabilities.maxImageCount, MAX_PRESENTIMAGE_COUNT );
			printf ( "Present Image Counts: %d\n", image_count );
		}
		frame_local_data.resize ( image_count );
		create_swapchain();
		framestate = FrameState::eInitialized;

	}

	if ( ( bool ) m_visible && m_root_section ) {
		m_root_section->render_frame();
	}
}
void VulkanWindow::create_swapchain() {

}
void VulkanWindow::framebuffer_size_changed ( s32 x, s32 y ) {
	printf ( "Size of Framebuffer %dx%d\n", x, y );
	framebuffer_size.x = x;
	framebuffer_size.y = y;
	if ( m_root_section ) {
		m_root_section->update_viewport ( Viewport<f32> ( 0.0f, 0.0f, framebuffer_size.x, framebuffer_size.y, 0.0f, 1.0f ), nullptr );
	}
	create_swapchain();
}
RendResult VulkanWindow::destroy() {
	if ( image_available_guard_sem )
		destroy_semaphore ( m_instance, image_available_guard_sem );
}
