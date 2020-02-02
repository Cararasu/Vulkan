#include "VWindow.h"
#include "VInstance.h"
#include "VResourceManager.h"
#include <render/Timing.h>
#include <render/UTF.h>


KeyCode glfw_button_transform ( int button_code ) {
	switch ( button_code ) {
	case GLFW_KEY_UNKNOWN:
		return KeyCode::Unknown;
	case GLFW_KEY_SPACE:
		return KeyCode::Space;
	case GLFW_KEY_APOSTROPHE:
		return KeyCode::Apostrophe;
	case GLFW_KEY_COMMA:
		return KeyCode::Comma;
	case GLFW_KEY_MINUS:
		return KeyCode::Minus;
	case GLFW_KEY_PERIOD:
		return KeyCode::Period;
	case GLFW_KEY_SLASH:
		return KeyCode::Slash;
	case GLFW_KEY_0:
		return KeyCode::Digit0;
	case GLFW_KEY_1:
		return KeyCode::Digit1;
	case GLFW_KEY_2:
		return KeyCode::Digit2;
	case GLFW_KEY_3:
		return KeyCode::Digit3;
	case GLFW_KEY_4:
		return KeyCode::Digit4;
	case GLFW_KEY_5:
		return KeyCode::Digit5;
	case GLFW_KEY_6:
		return KeyCode::Digit6;
	case GLFW_KEY_7:
		return KeyCode::Digit7;
	case GLFW_KEY_8:
		return KeyCode::Digit8;
	case GLFW_KEY_9:
		return KeyCode::Digit9;
	case GLFW_KEY_SEMICOLON:
		return KeyCode::Semicolon;
	case GLFW_KEY_EQUAL:
		return KeyCode::Equal;
	case GLFW_KEY_A:
		return KeyCode::A;
	case GLFW_KEY_B:
		return KeyCode::B;
	case GLFW_KEY_C:
		return KeyCode::C;
	case GLFW_KEY_D:
		return KeyCode::D;
	case GLFW_KEY_E:
		return KeyCode::E;
	case GLFW_KEY_F:
		return KeyCode::F;
	case GLFW_KEY_G:
		return KeyCode::G;
	case GLFW_KEY_H:
		return KeyCode::H;
	case GLFW_KEY_I:
		return KeyCode::I;
	case GLFW_KEY_J:
		return KeyCode::J;
	case GLFW_KEY_K:
		return KeyCode::K;
	case GLFW_KEY_L:
		return KeyCode::L;
	case GLFW_KEY_M:
		return KeyCode::M;
	case GLFW_KEY_N:
		return KeyCode::N;
	case GLFW_KEY_O:
		return KeyCode::O;
	case GLFW_KEY_P:
		return KeyCode::P;
	case GLFW_KEY_Q:
		return KeyCode::Q;
	case GLFW_KEY_R:
		return KeyCode::R;
	case GLFW_KEY_S:
		return KeyCode::S;
	case GLFW_KEY_T:
		return KeyCode::T;
	case GLFW_KEY_U:
		return KeyCode::U;
	case GLFW_KEY_V:
		return KeyCode::V;
	case GLFW_KEY_W:
		return KeyCode::W;
	case GLFW_KEY_X:
		return KeyCode::X;
	case GLFW_KEY_Y:
		return KeyCode::Y;
	case GLFW_KEY_Z:
		return KeyCode::Z;
	case GLFW_KEY_LEFT_BRACKET:
		return KeyCode::LBracket;
	case GLFW_KEY_BACKSLASH:
		return KeyCode::Backslash;
	case GLFW_KEY_RIGHT_BRACKET:
		return KeyCode::RBracket;
	case GLFW_KEY_GRAVE_ACCENT:
		return KeyCode::GraveAccent;
	case GLFW_KEY_WORLD_1:
		return KeyCode::World1;
	case GLFW_KEY_WORLD_2:
		return KeyCode::World2;
	case GLFW_KEY_ESCAPE:
		return KeyCode::Escape;
	case GLFW_KEY_ENTER:
		return KeyCode::Enter;
	case GLFW_KEY_TAB:
		return KeyCode::Tab;
	case GLFW_KEY_BACKSPACE:
		return KeyCode::Backspace;
	case GLFW_KEY_INSERT:
		return KeyCode::Insert;
	case GLFW_KEY_DELETE:
		return KeyCode::Delete;
	case GLFW_KEY_RIGHT:
		return KeyCode::Right;
	case GLFW_KEY_LEFT:
		return KeyCode::Left;
	case GLFW_KEY_DOWN:
		return KeyCode::Down;
	case GLFW_KEY_UP:
		return KeyCode::Up;
	case GLFW_KEY_PAGE_UP:
		return KeyCode::PageUp;
	case GLFW_KEY_PAGE_DOWN:
		return KeyCode::PageDown;
	case GLFW_KEY_HOME:
		return KeyCode::Home;
	case GLFW_KEY_END:
		return KeyCode::End;
	case GLFW_KEY_CAPS_LOCK:
		return KeyCode::CapsLock;
	case GLFW_KEY_SCROLL_LOCK:
		return KeyCode::ScrollLock;
	case GLFW_KEY_NUM_LOCK:
		return KeyCode::NumLock;
	case GLFW_KEY_PRINT_SCREEN:
		return KeyCode::PrintScreen;
	case GLFW_KEY_PAUSE:
		return KeyCode::Pause;
	case GLFW_KEY_F1:
		return KeyCode::F1;
	case GLFW_KEY_F2:
		return KeyCode::F2;
	case GLFW_KEY_F3:
		return KeyCode::F3;
	case GLFW_KEY_F4:
		return KeyCode::F4;
	case GLFW_KEY_F5:
		return KeyCode::F5;
	case GLFW_KEY_F6:
		return KeyCode::F6;
	case GLFW_KEY_F7:
		return KeyCode::F7;
	case GLFW_KEY_F8:
		return KeyCode::F8;
	case GLFW_KEY_F9:
		return KeyCode::F9;
	case GLFW_KEY_F10:
		return KeyCode::F10;
	case GLFW_KEY_F11:
		return KeyCode::F11;
	case GLFW_KEY_F12:
		return KeyCode::F12;
	case GLFW_KEY_F13:
		return KeyCode::F13;
	case GLFW_KEY_F14:
		return KeyCode::F14;
	case GLFW_KEY_F15:
		return KeyCode::F15;
	case GLFW_KEY_F16:
		return KeyCode::F16;
	case GLFW_KEY_F17:
		return KeyCode::F17;
	case GLFW_KEY_F18:
		return KeyCode::F18;
	case GLFW_KEY_F19:
		return KeyCode::F19;
	case GLFW_KEY_F20:
		return KeyCode::F20;
	case GLFW_KEY_F21:
		return KeyCode::F21;
	case GLFW_KEY_F22:
		return KeyCode::F22;
	case GLFW_KEY_F23:
		return KeyCode::F23;
	case GLFW_KEY_F24:
		return KeyCode::F24;
	case GLFW_KEY_F25:
		return KeyCode::F25;
	case GLFW_KEY_KP_0:
		return KeyCode::KP0;
	case GLFW_KEY_KP_1:
		return KeyCode::KP1;
	case GLFW_KEY_KP_2:
		return KeyCode::KP2;
	case GLFW_KEY_KP_3:
		return KeyCode::KP3;
	case GLFW_KEY_KP_4:
		return KeyCode::KP4;
	case GLFW_KEY_KP_5:
		return KeyCode::KP5;
	case GLFW_KEY_KP_6:
		return KeyCode::KP6;
	case GLFW_KEY_KP_7:
		return KeyCode::KP7;
	case GLFW_KEY_KP_8:
		return KeyCode::KP8;
	case GLFW_KEY_KP_9:
		return KeyCode::KP9;
	case GLFW_KEY_KP_DECIMAL:
		return KeyCode::KPDecimal;
	case GLFW_KEY_KP_DIVIDE:
		return KeyCode::KPDivide;
	case GLFW_KEY_KP_MULTIPLY:
		return KeyCode::KPMultiply;
	case GLFW_KEY_KP_SUBTRACT:
		return KeyCode::KPSubtract;
	case GLFW_KEY_KP_ADD:
		return KeyCode::KPAdd;
	case GLFW_KEY_KP_ENTER:
		return KeyCode::KPEnter;
	case GLFW_KEY_KP_EQUAL:
		return KeyCode::KPEqual;
	case GLFW_KEY_LEFT_SHIFT:
		return KeyCode::LShift;
	case GLFW_KEY_LEFT_CONTROL:
		return KeyCode::LCntrl;
	case GLFW_KEY_LEFT_ALT:
		return KeyCode::LAlt;
	case GLFW_KEY_LEFT_SUPER:
		return KeyCode::LSuper;
	case GLFW_KEY_RIGHT_SHIFT:
		return KeyCode::RShift;
	case GLFW_KEY_RIGHT_CONTROL:
		return KeyCode::RCntrl;
	case GLFW_KEY_RIGHT_ALT:
		return KeyCode::RAlt;
	case GLFW_KEY_RIGHT_SUPER:
		return KeyCode::RSuper;
	case GLFW_KEY_MENU:
		return KeyCode::Menu;
	}
	return KeyCode::Unknown;
}
KeyCode glfw_mouse_transform ( int mouse_code ) {
	switch ( mouse_code ) {
	case GLFW_MOUSE_BUTTON_LEFT:
		return KeyCode::MouseLeft;
	case GLFW_MOUSE_BUTTON_RIGHT:
		return KeyCode::MouseLeft;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		return KeyCode::MouseLeft;
	case GLFW_MOUSE_BUTTON_4:
		return KeyCode::Mouse4;
	case GLFW_MOUSE_BUTTON_5:
		return KeyCode::Mouse5;
	case GLFW_MOUSE_BUTTON_6:
		return KeyCode::Mouse6;
	case GLFW_MOUSE_BUTTON_7:
		return KeyCode::Mouse7;
	case GLFW_MOUSE_BUTTON_8:
		return KeyCode::Mouse8;
	}
	return KeyCode::Unknown;
}


VWindow::VWindow ( VInstance* instance ) : v_instance ( instance ) {

}
VWindow::~VWindow() {
	v_instance->destroy_window ( this );

	for ( VBaseImage* present_image : present_images )
		if ( present_image )
			v_instance->m_resource_manager->v_delete_image ( present_image );
}

void VWindow::initialize() {
	glfwWindowHint ( GLFW_CLIENT_API, GLFW_NO_API );
	switch ( m_showmode.wanted ) {
	case WindowShowMode::Maximized:
		glfwWindowHint ( GLFW_MAXIMIZED, true );
		glfwWindowHint ( GLFW_AUTO_ICONIFY, false );
		break;
	case WindowShowMode::Minimized:
		glfwWindowHint ( GLFW_MAXIMIZED, false );
		glfwWindowHint ( GLFW_AUTO_ICONIFY, false );
		break;
	case WindowShowMode::Windowed:
		glfwWindowHint ( GLFW_MAXIMIZED, false );
		glfwWindowHint ( GLFW_AUTO_ICONIFY, false );
		break;
	}
	glfwWindowHint ( GLFW_FLOATING, false );
	glfwWindowHint ( GLFW_VISIBLE, false );
	glfwWindowHint ( GLFW_RESIZABLE, ( bool ) m_resizable.wanted );
	glfwWindowHint ( GLFW_DECORATED, m_border.wanted == WindowBorder::Normal );

	VMonitor* fullscreen_monitor = dynamic_cast<VMonitor*> ( this->m_fullscreen_monitor.wanted );
	if ( fullscreen_monitor ) { //it is fullscreen
		VideoMode wanted_mode = fullscreen_monitor->find_best_videomode ( m_size.wanted, m_refreshrate.wanted );
		m_size = wanted_mode.extend;
		m_refreshrate = wanted_mode.refresh_rate;
		glfwWindowHint ( GLFW_REFRESH_RATE, m_refreshrate.wanted );

		m_refreshrate.apply();
		v_logger.log<LogLevel::Debug> ( "Fullscreen Videomode %dx%d %dHz", wanted_mode.extend.width, wanted_mode.extend.height, wanted_mode.refresh_rate );
	}

	window = glfwCreateWindow ( m_size.wanted.x, m_size.wanted.y, "Vulkan Test", fullscreen_monitor ? fullscreen_monitor->monitor : nullptr, nullptr );

	if ( m_showmode.wanted == WindowShowMode::Windowed ) {
		glfwSetWindowPos ( window, m_position.wanted.x, m_position.wanted.y );
	}
	m_showmode.apply();

	VCHECKCALL ( glfwCreateWindowSurface ( v_instance->v_instance, window, nullptr, ( VkSurfaceKHR* ) &surface ), v_logger.log<LogLevel::Error> ( "Creation of Surface failed" ) );

	glfwSetWindowUserPointer ( window, this );

	v_instance->window_map.insert ( std::make_pair ( window, this ) );
	m_visible.apply();
	m_position.apply();
	m_size.apply();
	m_border.apply();

	glfwSetWindowPosCallback ( window, [] ( GLFWwindow * window, int x, int y ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::Window;
			event.window.action = WindowAction::Moved;
			event.window.x = x;
			event.window.y = y;
			vulkan_window->eventqueue.push ( event );
			v_logger.log<LogLevel::Debug> ( "Position of Window %dx%d", x, y );
			vulkan_window->m_position.apply ( {x, y} );
		} else {
			v_logger.log<LogLevel::Error> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetWindowSizeCallback ( window, [] ( GLFWwindow * window, int x, int y ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::Window;
			event.window.action = WindowAction::Resized;
			event.window.x = x;
			event.window.y = y;
			vulkan_window->eventqueue.push ( event );
			v_logger.log<LogLevel::Debug> ( "Size of Window %dx%d", x, y );
			vulkan_window->m_size.apply ( {x, y} );
		} else {
			v_logger.log<LogLevel::Error> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetWindowCloseCallback ( window, [] ( GLFWwindow * window ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::Window;
			event.window.action = WindowAction::Closed;
			vulkan_window->eventqueue.push ( event );
			glfwHideWindow ( window );
			vulkan_window->m_visible.apply ( false );
		} else {
			v_logger.log<LogLevel::Error> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetWindowRefreshCallback ( window, [] ( GLFWwindow * window ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			//@Remove when rendering is delegated into a different thread
			v_logger.log<LogLevel::Debug> ( "Refresh" );
			//vulkan_window->v_instance->present_window ( vulkan_window );
		} else {
			v_logger.log<LogLevel::Error> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetWindowFocusCallback ( window, [] ( GLFWwindow * window, int focus ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::Window;
			event.window.action = WindowAction::Focused;
			event.window.value = focus == GLFW_TRUE;
			vulkan_window->eventqueue.push ( event );
			vulkan_window->m_focused.apply ( focus == GLFW_TRUE );
		} else {
			v_logger.log<LogLevel::Error> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetWindowIconifyCallback ( window, [] ( GLFWwindow * window, int iconified ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::Window;
			event.window.action = WindowAction::Iconify;
			event.window.value = iconified == GLFW_TRUE;
			vulkan_window->m_showmode.apply ( iconified == GLFW_TRUE ? WindowShowMode::Minimized : WindowShowMode::Windowed );
			vulkan_window->eventqueue.push ( event );
		} else {
			v_logger.log<LogLevel::Error> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetFramebufferSizeCallback ( window, [] ( GLFWwindow * window, int x, int y ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::Window;
			event.window.action = WindowAction::FrameResized;
			event.window.x = x;
			event.window.y = y;
			vulkan_window->eventqueue.push ( event );
			v_logger.log<LogLevel::Debug> ( "Size of Framebuffer %dx%d", x, y );
			vulkan_window->rendering_mutex.lock();
			vulkan_window->framebuffer_size_changed ( {x, y} );
			vulkan_window->rendering_mutex.unlock();
		} else {
			v_logger.log<LogLevel::Error> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetScrollCallback ( window, [] ( GLFWwindow * window, double xoffset, double yoffset ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::Scroll;
			event.scroll.deltax = xoffset;
			event.scroll.deltay = yoffset;
			vulkan_window->eventqueue.push ( event );
		} else {
			v_logger.log<LogLevel::Error> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetKeyCallback ( window, [] ( GLFWwindow * window, int key, int scancode, int action, int mods ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::Button;
			event.button.action = action == GLFW_PRESS ? PressAction::Press : ( action == GLFW_RELEASE ? PressAction::Release : PressAction::Repeat );
			event.button.shift = ( mods | GLFW_MOD_SHIFT ) != 0;
			event.button.cntrl = ( mods | GLFW_MOD_CONTROL ) != 0;
			event.button.alt = ( mods | GLFW_MOD_ALT ) != 0;
			event.button.super = ( mods | GLFW_MOD_SUPER ) != 0;
			event.button.keycode = glfw_button_transform ( key );
			event.button.scancode = scancode;
			const char* text = glfwGetKeyName ( key, scancode );
			if ( text ) {
				event.button.utf8[0] = text[0];
				event.button.utf8[1] = text[1];
				if ( text[1] ) {
					event.button.utf8[2] = text[2];
					if ( text[2] ) {
						event.button.utf8[3] = text[3];
					}
				}
			} else {
				event.button.utf8[0] = '\0';
			}
			vulkan_window->eventqueue.push ( event );
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
			v_logger.log<LogLevel::Error> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetCharModsCallback ( window, [] ( GLFWwindow * window, unsigned int codepoint, int mods ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::Char;
			utf32_to_utf8 ( codepoint, event.charinput.utf8 );
			event.charinput.shift = ( mods & GLFW_MOD_SHIFT ) != 0;
			event.charinput.cntrl = ( mods & GLFW_MOD_CONTROL ) != 0;
			event.charinput.alt = ( mods & GLFW_MOD_ALT ) != 0;
			event.charinput.super = ( mods & GLFW_MOD_SUPER ) != 0;
			vulkan_window->eventqueue.push ( event );
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
			v_logger.log<LogLevel::Error> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetCursorPosCallback ( window, [] ( GLFWwindow * window, double xpos, double ypos ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::Mouse;
			event.mouse.action = MouseMoveAction::Moved;
			event.mouse.deltax = ( vulkan_window->mouse_x < 0.0 ) ? 0.0 : xpos - vulkan_window->mouse_x;
			event.mouse.deltay = ( vulkan_window->mouse_y < 0.0 ) ? 0.0 : ypos - vulkan_window->mouse_y;
			event.mouse.posx = vulkan_window->mouse_x = xpos;
			event.mouse.posy = vulkan_window->mouse_y = ypos;
			vulkan_window->eventqueue.push ( event );
		} else {
			v_logger.log<LogLevel::Error> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetCursorEnterCallback ( window, [] ( GLFWwindow * window, int entered ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::Mouse;
			if ( entered ) {
				vulkan_window->mouse_x = -1.0;
				vulkan_window->mouse_y = -1.0;
				event.mouse.action = MouseMoveAction::Entered;
				vulkan_window->eventqueue.push ( event );
			} else {
				vulkan_window->mouse_x = -1.0;
				vulkan_window->mouse_y = -1.0;
				event.mouse.action = MouseMoveAction::Left;
			}
			event.mouse.posx = vulkan_window->mouse_x;
			event.mouse.posy = vulkan_window->mouse_x;
			event.mouse.deltax = 0.0f;
			event.mouse.deltay = 0.0f;
			vulkan_window->eventqueue.push ( event );
		} else {
			v_logger.log<LogLevel::Error> ( "No Window Registered For GLFW-Window" );
		}
	} );
	glfwSetMouseButtonCallback ( window, [] ( GLFWwindow * window, int button, int action, int mods ) {
		VWindow* vulkan_window = static_cast<VWindow*> ( glfwGetWindowUserPointer ( window ) );
		if ( vulkan_window ) {
			OSEvent event;
			event.type = OSEventType::Button;
			event.button.action = action == GLFW_PRESS ? PressAction::Press : ( action == GLFW_RELEASE ? PressAction::Release : PressAction::Repeat );
			event.button.shift = ( mods & GLFW_MOD_SHIFT ) != 0;
			event.button.cntrl = ( mods & GLFW_MOD_CONTROL ) != 0;
			event.button.alt = ( mods & GLFW_MOD_ALT ) != 0;
			event.button.super = ( mods & GLFW_MOD_SUPER ) != 0;
			event.button.keycode = glfw_mouse_transform ( button );
			event.button.scancode = 0;
			vulkan_window->eventqueue.push ( event );
		} else {
			v_logger.log<LogLevel::Error> ( "No Window Registered For GLFW-Window" );
		}
	} );
	//initialize Vulkan stuff

	image_available_guard_sem = v_instance->create_semaphore ();

	{
		u32 formatCount;
		v_instance->vk_physical_device().getSurfaceFormatsKHR ( surface, &formatCount, nullptr );
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
			if ( v_logger.level <= LogLevel::Debug ) {
				for ( size_t i = 0; i < formatCount; i++ ) {
					v_logger.log<LogLevel::Debug> ( "Present Format: %s Colorspace: %s", to_string ( formats[i].format ).c_str(), to_string ( formats[i].colorSpace ).c_str() );
				}
			}
		}
	}
	chosen_presentation_mode = vk::PresentModeKHR::eFifo;//can be turned into global flags of what is supported
	{
		u32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR ( v_instance->vk_physical_device(), surface, &presentModeCount, nullptr );
		bool fifo_relaxed = false, immediate = false, mailbox = false;
		if ( presentModeCount != 0 ) {
			vk::PresentModeKHR presentModes[presentModeCount];
			v_instance->vk_physical_device().getSurfacePresentModesKHR ( surface, &presentModeCount, presentModes );
			for ( size_t i = 0; i < presentModeCount; i++ ) {
				if ( presentModes[i] == vk::PresentModeKHR::eMailbox ) {
					mailbox = true;
				} else if ( presentModes[i] == vk::PresentModeKHR::eImmediate ) {
					immediate = true;
				} else if ( presentModes[i] == vk::PresentModeKHR::eFifoRelaxed ) {
					fifo_relaxed = true;
				}
			}
		}
		if ( false ) {
			//if ( mailbox ) {
			chosen_presentation_mode = vk::PresentModeKHR::eMailbox;
		} else if ( false ) { //immediate
			chosen_presentation_mode = vk::PresentModeKHR::eImmediate;
		} else if ( false ) {
			//} else if ( fifo_relaxed ) {
			chosen_presentation_mode = vk::PresentModeKHR::eFifoRelaxed;
		} else {
			chosen_presentation_mode = vk::PresentModeKHR::eFifo;
		}
		chosen_presentation_mode = vk::PresentModeKHR::eFifo;
	}

	if ( m_visible.wanted ) {
		glfwShowWindow ( window );
	}
	framebuffer_size_changed ( m_size.value );

}

void VWindow::prepare_frame() {
	v_instance->vk_device().acquireNextImageKHR ( swap_chain, std::numeric_limits<u64>::max(), image_available_guard_sem, vk::Fence(), &present_image_index );

	printf ( "Acquire %d\n", present_image_index );
	FrameLocalData* data = current_framelocal_data();

	//reset for frame
	v_instance->vk_device().waitForFences ( 1, &data->image_presented_fence, true, std::numeric_limits<u64>::max() );
	v_instance->vk_device().resetFences ( 1, &data->image_presented_fence );
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
		if ( m_border.changed() ) { //TODO rebuild window
			m_border.apply();
		}

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
			if ( m_showmode.changed() ) {
				switch ( m_showmode.wanted ) {
				case WindowShowMode::Maximized:
					if ( m_fullscreen_monitor.wanted ) {
						VMonitor* vulkan_monitor = dynamic_cast<VMonitor*> ( m_fullscreen_monitor.wanted );
						if ( vulkan_monitor ) {
							VideoMode wanted_mode = vulkan_monitor->find_best_videomode ( m_size.wanted, m_refreshrate.wanted );
							m_size = wanted_mode.extend;
							m_refreshrate = wanted_mode.refresh_rate;
						}
						glfwSetWindowMonitor ( window, vulkan_monitor ? vulkan_monitor->monitor : nullptr, m_position.wanted.x, m_position.wanted.y, m_size.wanted.x, m_size.wanted.y, m_refreshrate.wanted );
						m_fullscreen_monitor.apply();
					} else {
						glfwMaximizeWindow ( window );
					}
					break;
				case WindowShowMode::Minimized:
					glfwIconifyWindow ( window );
					break;
				case WindowShowMode::Windowed:
					glfwRestoreWindow ( window );
					break;
				}
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
			case CursorMode::Normal:
				glfwSetInputMode ( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
				break;
			case CursorMode::Invisible:
				glfwSetInputMode ( window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN );
				break;
			case CursorMode::Catch:
				glfwSetInputMode ( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
				break;
			}
			m_cursormode.apply();
		}
	}
	//seperate branch because we might need to recreate the window
	if ( !window ) {
		initialize();
	}
	return RendResult::Success;
}
void VWindow::create_swapchain() {

	//needs to be done first, because it waits for the fences to finish, which empties the graphics/presentation queue
	destroy_frame_local_data();

	V_CHECKCALL(v_instance->vk_physical_device().getSurfaceCapabilitiesKHR ( surface, &capabilities ), printf("Cannot get SurfaceCapabilities"));

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
		v_logger.log<LogLevel::Debug> ( "Present Image Counts: %d", image_buffer_count );
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
		u32 queueFamilyIndices[] = {queue_wrapper->graphics_queue_id, queue_wrapper->present_queue_id};
		if ( queue_wrapper->combined_graphics_present_queue ) {
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
			swapchainCreateInfo.queueFamilyIndexCount = 1; // Optional
		} else {
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
		}
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		swapchainCreateInfo.preTransform = capabilities.currentTransform;//VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR??
		switch ( m_alphablend.value ) {
		case WindowAlphaBlend::Opaque:
			swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
			break;
		case WindowAlphaBlend::Blend:
			swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::ePostMultiplied;
			break;
		}
		swapchainCreateInfo.presentMode = chosen_presentation_mode;
		swapchainCreateInfo.clipped = VK_TRUE;//clip pixels that are behind other windows

		vk::Bool32 support;
		V_CHECKCALL(v_instance->vk_physical_device().getSurfaceSupportKHR ( queue_wrapper->present_queue_id, surface, &support ), printf("Cannot get SurfaceSupport\n"));

		v_logger.log<LogLevel::Trace> ( "SUPPORTED %d", support);

		V_CHECKCALL ( v_instance->vk_device().createSwapchainKHR ( &swapchainCreateInfo, nullptr, &swap_chain ), v_logger.log<LogLevel::Error> ( "Creation of Swapchain failed" ) );
		v_instance->vk_device().destroySwapchainKHR ( swapchainCreateInfo.oldSwapchain, nullptr );
	}
	u32 image_count;
	std::vector<vk::Image> images;
	V_CHECKCALL(v_instance->vk_device().getSwapchainImagesKHR ( swap_chain, &image_count, nullptr ), printf("Cannot get SwapchainImage-Count"));
	images.resize(image_count);
	V_CHECKCALL(v_instance->vk_device().getSwapchainImagesKHR ( swap_chain, &image_count, images.data() ), printf("Cannot get SwapchainImages"));
	if ( !present_images.size ) {
		present_images.resize ( images.size() );
		for ( size_t i = 0; i < present_images.size; i++ ) {
			present_images[i] = v_instance->m_resource_manager->v_images.insert ( new VBaseImage ( v_instance, this ) );
		}
	}
	for ( int i = 0; i < present_images.size; i++ ) {
		VBaseImage* present_image = present_images[i];
		bool changed = present_image->width != swap_chain_extend.width || present_image->height != swap_chain_extend.height || present_image->depth != 0;
		if ( changed ) {
			v_logger.log<LogLevel::Info> ( "Resizing Window Frame %d from size %" PRId32 "x%" PRId32 "x%" PRId32 " to %" PRId32 "x%" PRId32 "x%" PRId32,
			                                i, present_image->width, present_image->height, present_image->depth, swap_chain_extend.width, swap_chain_extend.height, 0 );
		}
		present_image->v_set_extent ( swap_chain_extend.width, swap_chain_extend.height, 0 );
		present_image->v_set_format ( present_swap_format.format );
		present_image->init ( images[i] );
		if ( changed ) {
			auto it = v_instance->m_resource_manager->dependency_map.find ( present_image );
			if ( it != v_instance->m_resource_manager->dependency_map.end() ) {
				v_logger.log<LogLevel::Info> ( "Found %d dependant Image(s)", it->second.size() );
				for ( VBaseImage* image : it->second ) {
					image->rebuild_image ( present_image->width, present_image->height, present_image->depth );
				}
			}
		}
	}

	create_frame_local_data ( present_images.size );
}
void VWindow::framebuffer_size_changed ( Extent2D<s32> extent ) {
	v_logger.log<LogLevel::Debug> ( "Size of Framebuffer %dx%d", extent.x, extent.y );

	if ( extent.x > 0 && extent.y > 0 )
		create_swapchain();
	v_logger.log<LogLevel::Trace> ( "Actual Extent %dx%d", swap_chain_extend.x, swap_chain_extend.y );
}

vk::CommandPool VWindow::graphics_command_pool() {
	if ( !frame_local_data[present_image_index].graphics_command_pool ) {
		vk::CommandPoolCreateInfo create_info;
		V_CHECKCALL(v_instance->m_device.createCommandPool ( &create_info, nullptr, &frame_local_data[present_image_index].graphics_command_pool ), printf("Cannot create GraphicsCommandPool\n"));
	}
	return frame_local_data[present_image_index].graphics_command_pool;
}
void VWindow::create_frame_local_data ( u32 count ) {

	frame_local_data.resize ( count );
	for ( u32 i = 0; i < count; i++ ) {
		vk::FenceCreateInfo create_info{ vk::FenceCreateFlags ( vk::FenceCreateFlagBits::eSignaled ) };
		V_CHECKCALL(v_instance->m_device.createFence ( &create_info, nullptr, &frame_local_data[i].image_presented_fence ), printf("Cannot create Fence\n"));
		frame_local_data[i].present_ready_sem = v_instance->create_semaphore();
	}
}
void VWindow::destroy_frame_local_data() {
	v_logger.log<LogLevel::Debug> ( "Wait For Queues to clear out" );
	v_instance->m_device.waitIdle();
	u32 index = 0;
	for ( FrameLocalData& data : frame_local_data ) {
		if ( data.image_presented_fence ) {
			//@TODO figure out why this freezes the Screen on Ubuntu/GNOME
			//v_instance->vk_device().waitForFences ( {data.image_presented_fence}, true, std::numeric_limits<u64>::max() );
			v_instance->vk_device().destroyFence ( data.image_presented_fence, nullptr );
		}
		v_instance->destroy_semaphore ( data.present_ready_sem );
		index++;
	}
	//@Debugging clear so we assert in case we access it in a state, that we should not
	frame_local_data.clear();
}
RendResult VWindow::destroy() {
	v_logger.log<LogLevel::Debug> ( "Destroy Semaphores" );
	if ( image_available_guard_sem )
		v_instance->destroy_semaphore ( image_available_guard_sem );
	v_logger.log<LogLevel::Debug> ( "Destroy Local Data" );
	destroy_frame_local_data();
	v_logger.log<LogLevel::Debug> ( "Destroy Swap Chain" );
	if ( swap_chain )
		v_instance->vk_device().destroySwapchainKHR ( swap_chain, nullptr );
	v_logger.log<LogLevel::Debug> ( "Destroy Command Pools" );
	if ( window_graphics_command_pool ) {
		v_instance->vk_device().destroyCommandPool ( window_graphics_command_pool, nullptr );
		window_graphics_command_pool = vk::CommandPool();
	}
	v_logger.log<LogLevel::Debug> ( "Destroyed Everything" );
	return RendResult::Success;
}
