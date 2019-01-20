#include "VWindow.h"
#include "VInstance.h"
#include "VResourceManager.h"
#include <render/Timing.h>
#include <render/UTF.h>


KeyCode glfw_button_transform ( int button_code ) {
	switch ( button_code ) {
	case GLFW_KEY_UNKNOWN:
		return KeyCode::eUnknown;
	case GLFW_KEY_SPACE:
		return KeyCode::eSpace;
	case GLFW_KEY_APOSTROPHE:
		return KeyCode::eApostrophe;
	case GLFW_KEY_COMMA:
		return KeyCode::eComma;
	case GLFW_KEY_MINUS:
		return KeyCode::eMinus;
	case GLFW_KEY_PERIOD:
		return KeyCode::ePeriod;
	case GLFW_KEY_SLASH:
		return KeyCode::eSlash;
	case GLFW_KEY_0:
		return KeyCode::e0;
	case GLFW_KEY_1:
		return KeyCode::e1;
	case GLFW_KEY_2:
		return KeyCode::e2;
	case GLFW_KEY_3:
		return KeyCode::e3;
	case GLFW_KEY_4:
		return KeyCode::e4;
	case GLFW_KEY_5:
		return KeyCode::e5;
	case GLFW_KEY_6:
		return KeyCode::e6;
	case GLFW_KEY_7:
		return KeyCode::e7;
	case GLFW_KEY_8:
		return KeyCode::e8;
	case GLFW_KEY_9:
		return KeyCode::e9;
	case GLFW_KEY_SEMICOLON:
		return KeyCode::eSemicolon;
	case GLFW_KEY_EQUAL:
		return KeyCode::eEqual;
	case GLFW_KEY_A:
		return KeyCode::eA;
	case GLFW_KEY_B:
		return KeyCode::eB;
	case GLFW_KEY_C:
		return KeyCode::eC;
	case GLFW_KEY_D:
		return KeyCode::eD;
	case GLFW_KEY_E:
		return KeyCode::eE;
	case GLFW_KEY_F:
		return KeyCode::eF;
	case GLFW_KEY_G:
		return KeyCode::eG;
	case GLFW_KEY_H:
		return KeyCode::eH;
	case GLFW_KEY_I:
		return KeyCode::eI;
	case GLFW_KEY_J:
		return KeyCode::eJ;
	case GLFW_KEY_K:
		return KeyCode::eK;
	case GLFW_KEY_L:
		return KeyCode::eL;
	case GLFW_KEY_M:
		return KeyCode::eM;
	case GLFW_KEY_N:
		return KeyCode::eN;
	case GLFW_KEY_O:
		return KeyCode::eO;
	case GLFW_KEY_P:
		return KeyCode::eP;
	case GLFW_KEY_Q:
		return KeyCode::eQ;
	case GLFW_KEY_R:
		return KeyCode::eR;
	case GLFW_KEY_S:
		return KeyCode::eS;
	case GLFW_KEY_T:
		return KeyCode::eT;
	case GLFW_KEY_U:
		return KeyCode::eU;
	case GLFW_KEY_V:
		return KeyCode::eV;
	case GLFW_KEY_W:
		return KeyCode::eW;
	case GLFW_KEY_X:
		return KeyCode::eX;
	case GLFW_KEY_Y:
		return KeyCode::eY;
	case GLFW_KEY_Z:
		return KeyCode::eZ;
	case GLFW_KEY_LEFT_BRACKET:
		return KeyCode::eLBracket;
	case GLFW_KEY_BACKSLASH:
		return KeyCode::eBackslash;
	case GLFW_KEY_RIGHT_BRACKET:
		return KeyCode::eRBracket;
	case GLFW_KEY_GRAVE_ACCENT:
		return KeyCode::eGraveAccent;
	case GLFW_KEY_WORLD_1:
		return KeyCode::eWorld1;
	case GLFW_KEY_WORLD_2:
		return KeyCode::eWorld2;
	case GLFW_KEY_ESCAPE:
		return KeyCode::eEscape;
	case GLFW_KEY_ENTER:
		return KeyCode::eEnter;
	case GLFW_KEY_TAB:
		return KeyCode::eTab;
	case GLFW_KEY_BACKSPACE:
		return KeyCode::eBackspace;
	case GLFW_KEY_INSERT:
		return KeyCode::eInsert;
	case GLFW_KEY_DELETE:
		return KeyCode::eDelete;
	case GLFW_KEY_RIGHT:
		return KeyCode::eRight;
	case GLFW_KEY_LEFT:
		return KeyCode::eLeft;
	case GLFW_KEY_DOWN:
		return KeyCode::eDown;
	case GLFW_KEY_UP:
		return KeyCode::eUp;
	case GLFW_KEY_PAGE_UP:
		return KeyCode::ePageUp;
	case GLFW_KEY_PAGE_DOWN:
		return KeyCode::ePageDown;
	case GLFW_KEY_HOME:
		return KeyCode::eHome;
	case GLFW_KEY_END:
		return KeyCode::eEnd;
	case GLFW_KEY_CAPS_LOCK:
		return KeyCode::eCapsLock;
	case GLFW_KEY_SCROLL_LOCK:
		return KeyCode::eScrollLock;
	case GLFW_KEY_NUM_LOCK:
		return KeyCode::eNumLock;
	case GLFW_KEY_PRINT_SCREEN:
		return KeyCode::ePrintScreen;
	case GLFW_KEY_PAUSE:
		return KeyCode::ePause;
	case GLFW_KEY_F1:
		return KeyCode::eF1;
	case GLFW_KEY_F2:
		return KeyCode::eF2;
	case GLFW_KEY_F3:
		return KeyCode::eF3;
	case GLFW_KEY_F4:
		return KeyCode::eF4;
	case GLFW_KEY_F5:
		return KeyCode::eF5;
	case GLFW_KEY_F6:
		return KeyCode::eF6;
	case GLFW_KEY_F7:
		return KeyCode::eF7;
	case GLFW_KEY_F8:
		return KeyCode::eF8;
	case GLFW_KEY_F9:
		return KeyCode::eF9;
	case GLFW_KEY_F10:
		return KeyCode::eF10;
	case GLFW_KEY_F11:
		return KeyCode::eF11;
	case GLFW_KEY_F12:
		return KeyCode::eF12;
	case GLFW_KEY_F13:
		return KeyCode::eF13;
	case GLFW_KEY_F14:
		return KeyCode::eF14;
	case GLFW_KEY_F15:
		return KeyCode::eF15;
	case GLFW_KEY_F16:
		return KeyCode::eF16;
	case GLFW_KEY_F17:
		return KeyCode::eF17;
	case GLFW_KEY_F18:
		return KeyCode::eF18;
	case GLFW_KEY_F19:
		return KeyCode::eF19;
	case GLFW_KEY_F20:
		return KeyCode::eF20;
	case GLFW_KEY_F21:
		return KeyCode::eF21;
	case GLFW_KEY_F22:
		return KeyCode::eF22;
	case GLFW_KEY_F23:
		return KeyCode::eF23;
	case GLFW_KEY_F24:
		return KeyCode::eF24;
	case GLFW_KEY_F25:
		return KeyCode::eF25;
	case GLFW_KEY_KP_0:
		return KeyCode::eKP0;
	case GLFW_KEY_KP_1:
		return KeyCode::eKP1;
	case GLFW_KEY_KP_2:
		return KeyCode::eKP2;
	case GLFW_KEY_KP_3:
		return KeyCode::eKP3;
	case GLFW_KEY_KP_4:
		return KeyCode::eKP4;
	case GLFW_KEY_KP_5:
		return KeyCode::eKP5;
	case GLFW_KEY_KP_6:
		return KeyCode::eKP6;
	case GLFW_KEY_KP_7:
		return KeyCode::eKP7;
	case GLFW_KEY_KP_8:
		return KeyCode::eKP8;
	case GLFW_KEY_KP_9:
		return KeyCode::eKP9;
	case GLFW_KEY_KP_DECIMAL:
		return KeyCode::eKPDecimal;
	case GLFW_KEY_KP_DIVIDE:
		return KeyCode::eKPDivide;
	case GLFW_KEY_KP_MULTIPLY:
		return KeyCode::eKPMultiply;
	case GLFW_KEY_KP_SUBTRACT:
		return KeyCode::eKPSubtract;
	case GLFW_KEY_KP_ADD:
		return KeyCode::eKPAdd;
	case GLFW_KEY_KP_ENTER:
		return KeyCode::eKPEnter;
	case GLFW_KEY_KP_EQUAL:
		return KeyCode::eKPEqual;
	case GLFW_KEY_LEFT_SHIFT:
		return KeyCode::eLShift;
	case GLFW_KEY_LEFT_CONTROL:
		return KeyCode::eLCntrl;
	case GLFW_KEY_LEFT_ALT:
		return KeyCode::eLAlt;
	case GLFW_KEY_LEFT_SUPER:
		return KeyCode::eLSuper;
	case GLFW_KEY_RIGHT_SHIFT:
		return KeyCode::eRShift;
	case GLFW_KEY_RIGHT_CONTROL:
		return KeyCode::eRCntrl;
	case GLFW_KEY_RIGHT_ALT:
		return KeyCode::eRAlt;
	case GLFW_KEY_RIGHT_SUPER:
		return KeyCode::eRSuper;
	case GLFW_KEY_MENU:
		return KeyCode::eMenu;
	}
	return KeyCode::eUnknown;
}
KeyCode glfw_mouse_transform ( int mouse_code ) {
	switch ( mouse_code ) {
	case GLFW_MOUSE_BUTTON_LEFT:
		return KeyCode::eMouseLeft;
	case GLFW_MOUSE_BUTTON_RIGHT:
		return KeyCode::eMouseLeft;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		return KeyCode::eMouseLeft;
	case GLFW_MOUSE_BUTTON_4:
		return KeyCode::eMouse4;
	case GLFW_MOUSE_BUTTON_5:
		return KeyCode::eMouse5;
	case GLFW_MOUSE_BUTTON_6:
		return KeyCode::eMouse6;
	case GLFW_MOUSE_BUTTON_7:
		return KeyCode::eMouse7;
	case GLFW_MOUSE_BUTTON_8:
		return KeyCode::eMouse8;
	}
	return KeyCode::eUnknown;
}


VWindow::VWindow ( VInstance* instance ) : v_instance ( instance ) {

}
VWindow::~VWindow() {
	v_instance->destroy_window ( this );
	
	for(VBaseImage* present_image : present_images) 
		if(present_image) 
			v_instance->m_resource_manager->v_delete_image ( present_image );
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
		v_logger.log<LogLevel::eDebug> ( "Fullscreen Videomode %dx%d %dHz", wanted_mode.extend.width, wanted_mode.extend.height, wanted_mode.refresh_rate );
	}

	window = glfwCreateWindow ( m_size.wanted.x, m_size.wanted.y, "Vulkan Test", fullscreen_monitor ? fullscreen_monitor->monitor : nullptr, nullptr );

	glfwSetWindowPos ( window, m_position.wanted.x, m_position.wanted.y );

	VCHECKCALL ( glfwCreateWindowSurface ( v_instance->v_instance, window, nullptr, ( VkSurfaceKHR* ) &surface ), v_logger.log<LogLevel::eError> ( "Creation of Surface failed" ) );

	glfwSetWindowUserPointer ( window, this );

	v_instance->window_map.insert ( std::make_pair ( window, this ) );
	m_visible.apply();
	m_position.apply();
	m_size.apply();

	glfwSetWindowPosCallback ( window, [] ( GLFWwindow * window, int x, int y ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::eWindow;
			event.window.action = WindowAction::eMoved;
			event.window.x = x;
			event.window.y = y;
			vulkan_window->eventqueue.push(event);
			v_logger.log<LogLevel::eDebug> ( "Position of Window %dx%d", x, y );
			vulkan_window->m_position.apply ( {x, y} );
		} else {
			v_logger.log<LogLevel::eError> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetWindowSizeCallback ( window, [] ( GLFWwindow * window, int x, int y ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::eWindow;
			event.window.action = WindowAction::eResized;
			event.window.x = x;
			event.window.y = y;
			vulkan_window->eventqueue.push(event);
			v_logger.log<LogLevel::eDebug> ( "Size of Window %dx%d", x, y );
			vulkan_window->m_size.apply ( {x, y} );
		} else {
			v_logger.log<LogLevel::eError> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetWindowCloseCallback ( window, [] ( GLFWwindow * window ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::eWindow;
			event.window.action = WindowAction::eClosed;
			vulkan_window->eventqueue.push(event);
			glfwHideWindow ( window );
			vulkan_window->m_visible.apply ( false );
		} else {
			v_logger.log<LogLevel::eError> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetWindowRefreshCallback ( window, [] ( GLFWwindow * window ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			//@Remove when rendering is delegated into a different thread
			v_logger.log<LogLevel::eDebug> ( "Refresh" );
			//vulkan_window->v_instance->present_window ( vulkan_window );
		} else {
			v_logger.log<LogLevel::eError> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetWindowFocusCallback ( window, [] ( GLFWwindow * window, int focus ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::eWindow;
			event.window.value = focus == GLFW_TRUE;
			vulkan_window->eventqueue.push(event);
			vulkan_window->m_focused.apply ( focus == GLFW_TRUE );
		} else {
			v_logger.log<LogLevel::eError> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetWindowIconifyCallback ( window, [] ( GLFWwindow * window, int iconified ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::eWindow;
			event.window.action = WindowAction::eIconify;
			event.window.value = iconified == GLFW_TRUE;
			vulkan_window->eventqueue.push(event);
			vulkan_window->m_minimized.apply ( iconified == GLFW_TRUE );
		} else {
			v_logger.log<LogLevel::eError> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetFramebufferSizeCallback ( window, [] ( GLFWwindow * window, int x, int y ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			vulkan_window->rendering_mutex.lock();
			vulkan_window->framebuffer_size_changed ( {x, y} );
			vulkan_window->rendering_mutex.unlock();
		} else {
			v_logger.log<LogLevel::eError> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetScrollCallback ( window, [] ( GLFWwindow * window, double xoffset, double yoffset ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::eScroll;
			event.scroll.deltax = xoffset;
			event.scroll.deltay = yoffset;
			vulkan_window->eventqueue.push(event);
		} else {
			v_logger.log<LogLevel::eError> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetKeyCallback ( window, [] ( GLFWwindow * window, int key, int scancode, int action, int mods ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::eButton;
			event.button.action = action == GLFW_PRESS ? PressAction::ePress : ( action == GLFW_RELEASE ? PressAction::eRelease : PressAction::eRepeat );
			event.button.shift = (mods | GLFW_MOD_SHIFT) != 0;
			event.button.cntrl = (mods | GLFW_MOD_CONTROL) != 0;
			event.button.alt = (mods | GLFW_MOD_ALT) != 0;
			event.button.super = (mods | GLFW_MOD_SUPER) != 0;
			event.button.keycode = glfw_button_transform ( key );
			event.button.scancode = scancode;
			const char* text = glfwGetKeyName ( key, scancode );
			if(text) {
				event.button.utf8[0] = text[0];
				event.button.utf8[1] = text[1];
				if(text[1]) {
					event.button.utf8[2] = text[2];
					if(text[2]){
						event.button.utf8[3] = text[3];
					}
				}
			} else {
				event.button.utf8[0] = '\0';
			}
			vulkan_window->eventqueue.push(event);
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
			printf ( "Key %d ", key );
			if ( mods & GLFW_MOD_SHIFT )   printf ( "Shift " );
			if ( mods & GLFW_MOD_CONTROL ) printf ( "Cntrl " );
			if ( mods & GLFW_MOD_ALT )     printf ( "Alt " );
			if ( mods & GLFW_MOD_SUPER )   printf ( "Super " );
			printf ( "\n" );
		} else {
			v_logger.log<LogLevel::eError> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetCharModsCallback ( window, [] ( GLFWwindow * window, unsigned int codepoint, int mods ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::eChar;
			utf32_to_utf8(codepoint, event.charinput.utf8);
			event.charinput.shift = (mods | GLFW_MOD_SHIFT) == true;
			event.charinput.cntrl = (mods | GLFW_MOD_CONTROL) == true;
			event.charinput.alt = (mods | GLFW_MOD_ALT) == true;
			event.charinput.super = (mods | GLFW_MOD_SUPER) == true;
			vulkan_window->eventqueue.push(event);
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
			v_logger.log<LogLevel::eError> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetCursorPosCallback ( window, [] ( GLFWwindow * window, double xpos, double ypos ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::eMouse;
			event.mouse.action = MouseMoveAction::eMoved;
			event.mouse.deltax = (vulkan_window->mouse_x < 0.0) ? 0.0 : xpos - vulkan_window->mouse_x;
			event.mouse.deltay = (vulkan_window->mouse_y < 0.0) ? 0.0 : ypos - vulkan_window->mouse_y;
			event.mouse.posx = vulkan_window->mouse_x = xpos;
			event.mouse.posy = vulkan_window->mouse_y = ypos;
			vulkan_window->eventqueue.push(event);
		} else {
			v_logger.log<LogLevel::eError> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetCursorEnterCallback ( window, [] ( GLFWwindow * window, int entered ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::eMouse;
			if ( entered ) {
				vulkan_window->mouse_x = -1.0;
				vulkan_window->mouse_y = -1.0;
				event.mouse.action = MouseMoveAction::eEntered;
				vulkan_window->eventqueue.push(event);
			} else {
				vulkan_window->mouse_x = -1.0;
				vulkan_window->mouse_y = -1.0;
				event.mouse.action = MouseMoveAction::eLeft;
			}
			event.mouse.posx = vulkan_window->mouse_x;
			event.mouse.posy = vulkan_window->mouse_x;
			event.mouse.deltax = 0.0f;
			event.mouse.deltay = 0.0f;
			vulkan_window->eventqueue.push(event);
		} else {
			v_logger.log<LogLevel::eError> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetMouseButtonCallback ( window, [] ( GLFWwindow * window, int button, int action, int mods ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::eButton;
			event.button.action = action == GLFW_PRESS ? PressAction::ePress : ( action == GLFW_RELEASE ? PressAction::eRelease : PressAction::eRepeat );
			event.button.shift = (mods | GLFW_MOD_SHIFT) != 0;
			event.button.cntrl = (mods | GLFW_MOD_CONTROL) != 0;
			event.button.alt = (mods | GLFW_MOD_ALT) != 0;
			event.button.super = (mods | GLFW_MOD_SUPER) != 0;
			event.button.keycode = glfw_mouse_transform ( button );
			event.button.scancode = 0;
			vulkan_window->eventqueue.push(event);
		} else {
			v_logger.log<LogLevel::eError> ( "No Window Registered For GLFW-Window" );
		}
	} );
	//initialize Vulkan stuff

	image_available_guard_sem = v_instance->create_semaphore ();

	{
		u32 formatCount;
		v_instance->vk_physical_device ().getSurfaceFormatsKHR ( surface, &formatCount, nullptr );
		if ( formatCount != 0 ) {
			vk::SurfaceFormatKHR formats[formatCount];
			v_instance->vk_physical_device ().getSurfaceFormatsKHR ( surface, &formatCount, formats );

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
			if ( v_logger.level <= LogLevel::eDebug ) {
				for ( size_t i = 0; i < formatCount; i++ ) {
					v_logger.log<LogLevel::eDebug> ( "Present Format: %s Colorspace: %s", to_string ( formats[i].format ).c_str(), to_string ( formats[i].colorSpace ).c_str() );
				}
			}
		}
	}
	chosen_presentation_mode = vk::PresentModeKHR::eFifo;//can be turned into global flags of what is supported
	{
		u32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR ( v_instance->vk_physical_device(), surface, &presentModeCount, nullptr );
		if ( presentModeCount != 0 ) {
			vk::PresentModeKHR presentModes[presentModeCount];
			v_instance->vk_physical_device().getSurfacePresentModesKHR ( surface, &presentModeCount, presentModes );
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

void VWindow::prepare_frame() {
	v_instance->vk_device().acquireNextImageKHR ( swap_chain, std::numeric_limits<u64>::max(), image_available_guard_sem, vk::Fence(), &present_image_index );

	FrameLocalData* data = current_framelocal_data();

	//reset for frame
	v_instance->vk_device().waitForFences ( {data->image_presented_fence}, true, std::numeric_limits<u64>::max() );
	v_instance->vk_device().resetFences ( {data->image_presented_fence} );
	v_instance->wait_for_frame ( data->frame_index );

	//TODO reset all needed command_buffers
	data->frame_index = v_instance->frame_index;
}
Image* VWindow::backed_image ( u32 index ) {
	return present_images[index];
}
u32 VWindow::backed_image_count ( ) {
	return present_images.size;
}
RendResult VWindow::update() {
	return v_update();
}
RendResult VWindow::v_update() {
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
			if ( m_minimized.changed() && !m_fullscreen_monitor.wanted ) {
				if(m_maximized.wanted)
					glfwIconifyWindow ( window );
				else 
					glfwRestoreWindow ( window );
				m_minimized.apply();
			}
			//maximized and not fullscreen
			else if ( m_maximized.changed() && !m_fullscreen_monitor.wanted ) {
				if(m_maximized.wanted)
					glfwMaximizeWindow ( window );
				else 
					glfwRestoreWindow ( window );
				m_maximized.apply();
			}
			//fullscreen, window mode cnd coming out of minimized or maximized
			else if ( m_fullscreen_monitor.changed() ) {
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
	return RendResult::eSuccess;
}
void VWindow::create_swapchain() {

	//needs to be done first, because it waits for the fences to finish, which empties the graphics/presentation queue
	destroy_frame_local_data();

	capabilities = v_instance->vk_physical_device().getSurfaceCapabilitiesKHR ( surface );

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
		v_logger.log<LogLevel::eDebug> ( "Present Image Counts: %d", image_buffer_count );
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
		QueueWrapper* queue_wrapper = &v_instance->queues;
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

		v_logger.log<LogLevel::eTrace> ( "SUPPORTED %d", v_instance->vk_physical_device().getSurfaceSupportKHR ( queue_wrapper->present_queue_id, surface ) );

		V_CHECKCALL ( v_instance->vk_device().createSwapchainKHR ( &swapchainCreateInfo, nullptr, &swap_chain ), v_logger.log<LogLevel::eError> ( "Creation of Swapchain failed" ) );
		v_instance->vk_device().destroySwapchainKHR ( swapchainCreateInfo.oldSwapchain );
	}
	std::vector<vk::Image> images = v_instance->vk_device().getSwapchainImagesKHR ( swap_chain );
	if ( !present_images.size ) {
		present_images.resize(images.size());
		for(int i = 0; i < present_images.size; i++) {
			present_images[i] = v_instance->m_resource_manager->v_images.insert ( new VBaseImage ( v_instance, this ) );
		}
	}
	for(int i = 0; i < present_images.size; i++) {
		VBaseImage* present_image = present_images[i];
		bool changed = present_image->width != swap_chain_extend.width || present_image->height != swap_chain_extend.height || present_image->depth != 0;
		if ( changed ) {
			v_logger.log<LogLevel::eInfo> ( "Resizing Window Frame %d from size %" PRId32 "x%" PRId32 "x%" PRId32 " to %" PRId32 "x%" PRId32 "x%" PRId32,
											i, present_image->width, present_image->height, present_image->depth, swap_chain_extend.width, swap_chain_extend.height, 0 );
		}
		present_image->v_set_extent ( swap_chain_extend.width, swap_chain_extend.height, 0 );
		present_image->v_set_format ( present_swap_format.format );
		present_image->init ( images[i] );
		if ( changed ) {
			auto it = v_instance->m_resource_manager->dependency_map.find ( present_image );
			if ( it != v_instance->m_resource_manager->dependency_map.end() ) {
				v_logger.log<LogLevel::eInfo> ( "Found %d dependant Image(s)", it->second.size() );
				for ( VBaseImage* image : it->second ) {
					image->rebuild_image ( present_image->width, present_image->height, present_image->depth );
				}
			}
		}
	}

	create_frame_local_data ( present_images.size );
}
void VWindow::framebuffer_size_changed ( Extent2D<s32> extent ) {
	v_logger.log<LogLevel::eDebug> ( "Size of Framebuffer %dx%d", extent.x, extent.y );
	v_logger.log<LogLevel::eTrace> ( "Minimized %d", m_minimized.value );
	v_logger.log<LogLevel::eTrace> ( "Visible %d", m_minimized.value );

	if ( extent.x > 0 && extent.y > 0 )
		create_swapchain();
	v_logger.log<LogLevel::eTrace> ( "Actual Extent %dx%d", swap_chain_extend.x, swap_chain_extend.y );
}

vk::CommandPool VWindow::graphics_command_pool() {
	if ( !frame_local_data[present_image_index].graphics_command_pool )
		frame_local_data[present_image_index].graphics_command_pool = v_instance->m_device.createCommandPool ( vk::CommandPoolCreateInfo() );
	return frame_local_data[present_image_index].graphics_command_pool;
}
void VWindow::create_frame_local_data ( u32 count ) {

	frame_local_data.resize ( count );
	for ( u32 i = 0; i < count; i++ ) {

		frame_local_data[i].image_presented_fence = v_instance->vk_device().createFence ( vk::FenceCreateFlags ( vk::FenceCreateFlagBits::eSignaled ) ); //image is ready
		frame_local_data[i].present_ready_sem = v_instance->create_semaphore();
	}
}
void VWindow::destroy_frame_local_data() {
	v_logger.log<LogLevel::eDebug> ( "Wait For Queues to clear out" );
	v_instance->m_device.waitIdle();
	u32 index = 0;
	for ( FrameLocalData& data : frame_local_data ) {
		if ( data.image_presented_fence ) {
			//@TODO figure out why this freezes the Screen on Ubuntu/GNOME
			//v_instance->vk_device().waitForFences ( {data.image_presented_fence}, true, std::numeric_limits<u64>::max() );
			v_instance->vk_device().destroyFence ( data.image_presented_fence );
		}
		v_instance->destroy_semaphore ( data.present_ready_sem );
		index++;
	}
	//@Debugging clear so we assert in case we access it in a state, that we should not
	frame_local_data.clear();
}
RendResult VWindow::destroy() {
	v_logger.log<LogLevel::eDebug> ( "Destroy Semaphores" );
	if ( image_available_guard_sem )
		v_instance->destroy_semaphore ( image_available_guard_sem );
	v_logger.log<LogLevel::eDebug> ( "Destroy Local Data" );
	destroy_frame_local_data();
	v_logger.log<LogLevel::eDebug> ( "Destroy Swap Chain" );
	if ( swap_chain )
		v_instance->vk_device().destroySwapchainKHR ( swap_chain );
	v_logger.log<LogLevel::eDebug> ( "Destroy Command Pools" );
	if ( window_graphics_command_pool ) {
		v_instance->vk_device().destroyCommandPool ( window_graphics_command_pool );
		window_graphics_command_pool = vk::CommandPool();
	}
	v_logger.log<LogLevel::eDebug> ( "Destroyed Everything" );
	return RendResult::eSuccess;
}
