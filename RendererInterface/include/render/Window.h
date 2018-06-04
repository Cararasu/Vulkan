#pragma once

#include "Dimensions.h"
#include "Monitor.h"

struct UINode {
	
};

struct RenderTargetWrapper{
	
	Extend2D getSize();
	
};


struct WindowSection {
	
	virtual void update_viewport(Viewport viewport, RenderTargetWrapper* targetWrapper) = 0;
	
	virtual void render_frame() = 0;
};

struct UIWindowSection : public WindowSection {
	
	virtual void set_root_node(UINode* node) = 0;
};

struct StackWindowSection : public WindowSection {
	
	virtual void set_stack_size(uint32_t size) = 0;
	virtual void set_layer(u32 layer, WindowSection* section) = 0;
};

struct WorldWindowSection : public WindowSection {

	void set_world();
	
};

struct Window {
	
	virtual void set_root_section(WindowSection* section) = 0;
	
	inline void set_dimensions(u32 x, u32 y, u32 width, u32 height) {
		set_dimensions(Offset2D(x, y), Extend2D(width, height));
	}
	inline void set_dimensions(Offset2D offset, Extend2D extend) {
		set_dimensions(nullptr, offset, extend);
	}
	inline void set_dimensions(Monitor* monitor, u32 x, u32 y, u32 width, u32 height) {
		set_dimensions(monitor, Offset2D(x, y), Extend2D(width, height));
	}
	virtual void set_dimensions(Monitor* monitor, Offset2D offset, Extend2D extend) = 0;
	
	inline void set_size(u32 width, u32 height) {
		set_size(Extend2D(width, height));
	}
	virtual void set_size(Extend2D extend) = 0;
	
	virtual Extend2D get_size() = 0;
	
	inline void set_offset(u32 x, u32 y) {
		set_offset(Offset2D(x, y));
	}
	inline void set_offset(Offset2D offset) {
		set_offset(nullptr, offset);
	}
	inline void set_offset(Monitor* monitor, u32 x, u32 y) {
		set_offset(monitor, Offset2D(x, y));
	}
	virtual void set_offset(Monitor* monitor, Offset2D offset) = 0;
	
	inline Offset2D get_offset() {
		return get_offset(nullptr);
	}
	virtual Offset2D get_offset(Monitor* monitor) = 0;
};