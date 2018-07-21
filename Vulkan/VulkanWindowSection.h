#pragma once

#include "render/Window.h"
#include "VulkanHeader.h"
#include "VulkanImage.h"
#include "VulkanQuadRenderer.h"

struct VulkanWindow;
struct VulkanInstance;

struct ImageRenderTarget {
	vk::ImageView imageview;
};

struct VulkanRenderTarget : public RenderTarget {
	Array<VulkanBaseImage*> images;
	VulkanBaseImage* depth_image;
};

struct VulkanQuadRenderer;

struct VulkanWindowSection {
	VulkanInstance* const v_instance;
	WindowSection* const window_section;

	Viewport<f32> viewport;
	VulkanWindow* m_parent_window;
	VulkanWindowSection* m_parent;

	vk::Semaphore finish_sem;

	VulkanWindowSection ( VulkanInstance* instance, WindowSection* window_section );
	virtual ~VulkanWindowSection();

	RendResult register_parent ( VulkanWindow* parent_window, VulkanWindowSection* parent ) {
		if ( m_parent_window || m_parent )
			return RendResult::eAlreadyRegistered;
		m_parent_window = parent_window;
		m_parent = parent;
		return RendResult::eSuccess;
	}
	void unregister_parent() {
		m_parent_window = nullptr;
		m_parent = nullptr;
	}
	
	virtual void render_frame ( u32 index ) = 0;

	virtual void resize ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) = 0;
	virtual void reset ( ) = 0;

	void v_update_viewport ( Viewport<f32> viewport, VulkanRenderTarget* render_targets ) {
		if ( render_targets ) {
			this->viewport = viewport;
			printf ( "Viewport changed %g x %g | %g x %g | %g - %g\n", viewport.offset.x, viewport.offset.y, viewport.extend.width, viewport.extend.height, viewport.depth.min, viewport.depth.max );
			resize ( viewport, render_targets );
		} else {
			reset();
		}
	}
};
/*struct UIVulkanSectionFrameData {
	vk::CommandBuffer clearcmd;
};
struct UIVulkanWindowSection : public WindowSection {
	
	Array<UIVulkanSectionFrameData> per_frame_data;
	VulkanImageWrapper* depth_image = nullptr;
	vk::ImageView depth_image_view;

	UIVulkanWindowSection ( VulkanInstance* instance ) : VulkanWindowSection ( WindowSectionType::eUI, instance ), v_quad_renderer(instance)  {}
	virtual ~UIVulkanWindowSection();

	virtual void render_frame ( u32 index );
	virtual void set_root_node ( UINode* node ) {}
	virtual void set_stack_size ( uint32_t size ) {}
	virtual void set_layer ( u32 layer, WindowSection* section ) {}
	virtual void set_world() {}

	virtual void resize ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper );
	virtual void reset ( );
};*/
struct VulkanWorldWindowSection : public VulkanWindowSection {

	VulkanWorldWindowSection ( VulkanInstance* instance, WindowSection* window_section ) : VulkanWindowSection(instance, window_section) {}
	virtual ~VulkanWorldWindowSection() {}

	virtual void render_frame ( u32 index );

	virtual void resize ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) {}
};
struct VulkanLayeredWindowSection : public VulkanWindowSection {

	VulkanLayeredWindowSection ( VulkanInstance* instance, WindowSection* window_section ) : VulkanWindowSection(instance, window_section) {}
	virtual ~VulkanLayeredWindowSection() {}

	virtual void render_frame ( u32 index );

	virtual void resize ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) {}
};
struct VulkanStackWindowSection : public VulkanWindowSection {

	VulkanStackWindowSection ( VulkanInstance* instance, WindowSection* window_section ) : VulkanWindowSection(instance, window_section) {}
	virtual ~VulkanStackWindowSection() {}

	virtual void render_frame ( u32 index );

	virtual void resize ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) {}
};
struct VulkanBorderWindowSection : public VulkanWindowSection {

	VulkanBorderWindowSection ( VulkanInstance* instance, WindowSection* window_section ) : VulkanWindowSection(instance, window_section) {}
	virtual ~VulkanBorderWindowSection() {}

	virtual void render_frame ( u32 index );

	virtual void resize ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) {}
};


struct V_WorldWindowSection : public WorldWindowSection {
	VulkanWorldWindowSection* v_section;
};
struct V_LayeredWindowSection : public LayeredWindowSection {
	VulkanLayeredWindowSection* v_section;
};
struct V_StackWindowSection : public StackWindowSection {
	VulkanStackWindowSection* v_section;
};
struct V_BorderWindowSection : public BorderWindowSection {
	VulkanBorderWindowSection* v_section;
};