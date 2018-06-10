#pragma once

#include "Dimensions.h"
#include "Monitor.h"

struct UINode {

};

struct RenderTargetWrapper {

	Extend2D<u32> getSize();

};


struct WindowSection {

	virtual void update_viewport (Viewport viewport, RenderTargetWrapper* targetWrapper) = 0;

	virtual void render_frame() = 0;
};

struct UIWindowSection : public WindowSection {

	virtual void set_root_node (UINode* node) = 0;
};

struct StackWindowSection : public WindowSection {

	virtual void set_stack_size (uint32_t size) = 0;
	virtual void set_layer (u32 layer, WindowSection* section) = 0;
};

struct WorldWindowSection : public WindowSection {

	virtual void set_world() {}
};

class Window {
protected:
	ChangeableValue<Offset2D<s32>> m_offset = Offset2D<s32>(0, 0);
	ChangeableValue<Extend2D<s32>> m_size = Extend2D<s32>(100, 100);
	ChangeableValue<bool> m_maximized = false;
	ChangeableValue<bool> m_minimized = false;
	ChangeableValue<bool> m_focused = true;
	ChangeableValue<bool> m_decorated = true;
	ChangeableValue<bool> m_visible = false;
	ChangeableValue<bool> m_resizable = false;
	ChangeableValue<Monitor*> m_fullscreen_monitor = nullptr;
	ChangeableValue<VideoMode> m_fullscreen_mode = VideoMode();

public:
	virtual void set_root_section (WindowSection* section) = 0;

	const ChangeableValue<Offset2D<s32>>* offset(){
		return &m_offset;
	}
	const ChangeableValue<Extend2D<s32>>* size(){
		return &m_size;
	}
	const ChangeableValue<bool>* minimized(){
		return &m_minimized;
	}
	const ChangeableValue<bool>* visible(){
		return &m_visible;
	}
	const ChangeableValue<Monitor*>* fullscreen_monitor(){
		return &m_fullscreen_monitor;
	}
	const ChangeableValue<VideoMode>* fullscreen_mode(){
		return &m_fullscreen_mode;
	}

	virtual RendResult update() = 0;
	virtual RendResult destroy() = 0;

};
