#pragma once

#include "Dimensions.h"
#include "Monitor.h"
#include "World.h"

struct UINode;

struct RenderTargetWrapper {

	Extent2D<u32> getSize();

};

enum class WindowSectionType {
	eUI,
	eStack,
	eWorld
};

struct WindowSection {
	const WindowSectionType section_type;

	WindowSection ( WindowSectionType type ) : section_type ( type ) {}
	virtual ~WindowSection() {}

	virtual void update_viewport ( Viewport<f32> viewport, RenderTargetWrapper* targetWrapper ) = 0;

	virtual void render_frame() = 0;

	virtual void set_root_node ( UINode* node ) = 0;
	virtual void set_stack_size ( uint32_t size ) = 0;
	virtual void set_layer ( u32 layer, WindowSection* section ) = 0;
	virtual void set_world(World* world) {}
};


enum class CursorMode {
	eNormal,
	eInvisible,
	eCatch,
};

enum class WindowAlphaBlend{
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

public:
	virtual ~Window(){}

	const ChangeableValue<Offset2D<s32>>* position() {
		return &m_position;
	}
	const ChangeableValue<Extent2D<s32>>* size() {
		return &m_size;
	}
	const ChangeableValue<u32>* refreshrate() {
		return &m_refreshrate;
	}
	const ChangeableValue<bool>* maximized() {
		return &m_maximized;
	}
	const ChangeableValue<bool>* minimized() {
		return &m_minimized;
	}
	const ChangeableValue<bool>* focused() {
		return &m_focused;
	}
	const ChangeableValue<bool>* decorated() {
		return &m_decorated;
	}
	const ChangeableValue<bool>* visible() {
		return &m_visible;
	}
	const ChangeableValue<bool>* resizable() {
		return &m_resizable;
	}
	const ChangeableValue<WindowAlphaBlend>* alphablend() {
		return &m_alphablend;
	}
	const ChangeableValue<CursorMode>* cursor_mode() {
		return &m_cursormode;
	}
	const ChangeableValue<Monitor*>* fullscreen_monitor() {
		return &m_fullscreen_monitor;
	}

	virtual RendResult set_root_section ( WindowSection* section ) = 0;
	virtual RendResult update() = 0;
	virtual RendResult destroy() = 0;
	
	//u32 max_buffer_images() = 0;
	//ChangeableValue<u32> buffered_image_count;

};
