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
	ChangeableValue<Offset2D<s32>> m_position = Offset2D<s32>(0, 0);
	ChangeableValue<Extend2D<s32>> m_size = Extend2D<s32>(100, 100);
	ChangeableValue<u32> m_refreshrate = 60;
	ChangeableValue<bool> m_maximized = false;
	ChangeableValue<bool> m_minimized = false;
	ChangeableValue<bool> m_focused = true;
	ChangeableValue<bool> m_decorated = true;
	ChangeableValue<bool> m_visible = false;
	ChangeableValue<bool> m_resizable = true;
	ChangeableValue<Monitor*> m_fullscreen_monitor = nullptr;

public:
	const ChangeableValue<Offset2D<s32>>* position(){
		return &m_position;
	}
	const ChangeableValue<Extend2D<s32>>* size(){
		return &m_size;
	}
	const ChangeableValue<u32>* refreshrate(){
		return &m_refreshrate;
	}
	const ChangeableValue<bool>* maximized(){
		return &m_maximized;
	}
	const ChangeableValue<bool>* minimized(){
		return &m_minimized;
	}
	const ChangeableValue<bool>* focused(){
		return &m_focused;
	}
	const ChangeableValue<bool>* decorated(){
		return &m_decorated;
	}
	const ChangeableValue<bool>* visible(){
		return &m_visible;
	}
	const ChangeableValue<bool>* resizable(){
		return &m_resizable;
	}
	const ChangeableValue<Monitor*>* fullscreen_monitor(){
		return &m_fullscreen_monitor;
	}

	virtual void set_root_section (WindowSection* section) = 0;
	virtual RendResult update() = 0;
	virtual RendResult destroy() = 0;

};
