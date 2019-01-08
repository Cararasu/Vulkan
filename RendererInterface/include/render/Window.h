#pragma once

#include "Dimensions.h"
#include "Monitor.h"
#include "World.h"
#include "Resources.h"
#include "WindowEvent.h"
#include <functional>
#include <mutex>
#include <queue>
#include "Queues.h"

enum class CursorMode {
	eNormal,
	eInvisible,
	eCatch,
};

enum class WindowAlphaBlend {
	eOpaque,
	eBlend
};


class Window {
protected:
	ChangeableValue<Offset2D<s32>> m_position = Offset2D<s32> ( 100, 100 );
	ChangeableValue<Extent2D<s32>> m_size = Extent2D<s32> ( 200, 200 );
	ChangeableValue<u32> m_refreshrate = 60;
	ChangeableValue<bool> m_maximized = false;
	ChangeableValue<bool> m_minimized = false;
	ChangeableValue<bool> m_focused = true;
	ChangeableValue<bool> m_decorated = true;
	ChangeableValue<bool> m_visible = false;
	ChangeableValue<bool> m_resizable = true;
	ChangeableValue<WindowAlphaBlend> m_alphablend = WindowAlphaBlend::eOpaque;
	ChangeableValue<CursorMode> m_cursormode = CursorMode::eNormal;
	ChangeableValue<Monitor*> m_fullscreen_monitor = nullptr;
	
	float mouse_x = -1.0f, mouse_y = -1.0f;
	
public:
	//TODO maybe switch out with a resizable circular buffer
	//it should be able to imlement 
	ConcurrentCyclingQueue<OSEvent> eventqueue;

	virtual ~Window() {}

	const ChangeableValue<Offset2D<s32>>& position() {
		return m_position;
	}
	const ChangeableValue<Extent2D<s32>>& size() {
		return m_size;
	}
	const ChangeableValue<u32>& refreshrate() {
		return m_refreshrate;
	}
	const ChangeableValue<bool>& maximized() {
		return m_maximized;
	}
	const ChangeableValue<bool>& minimized() {
		return m_minimized;
	}
	const ChangeableValue<bool>& focused() {
		return m_focused;
	}
	const ChangeableValue<bool>& decorated() {
		return m_decorated;
	}
	const ChangeableValue<bool>& visible() {
		return m_visible;
	}
	const ChangeableValue<bool>& resizable() {
		return m_resizable;
	}
	const ChangeableValue<WindowAlphaBlend>& alphablend() {
		return m_alphablend;
	}
	const ChangeableValue<CursorMode>& cursor_mode() {
		return m_cursormode;
	}
	const ChangeableValue<Monitor*>& fullscreen_monitor() {
		return m_fullscreen_monitor;
	}

	virtual Image* backed_image (u32 index) = 0;
	virtual u32 backed_image_count () = 0;
	virtual RendResult update() = 0;
	virtual RendResult destroy() = 0;

	//u32 max_buffer_images() = 0;
	//ChangeableValue<u32> buffered_image_count;

};