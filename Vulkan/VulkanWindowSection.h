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
	Array<ImageRenderTarget> images;
	vk::Format color_format;
	vk::ImageView depthview;
	vk::Format depth_stencil_format;
	u32 targetcount;
};

struct VulkanQuadRenderer;

struct VulkanWindowSection : public WindowSection {
	VulkanInstance* const v_instance;
	Viewport<f32> m_viewport;
	vk::CommandPool commandpool;

	VulkanWindow* m_parent_window;
	VulkanWindowSection* m_parent;

	vk::Semaphore finish_sem;

	VulkanWindowSection ( WindowSectionType type, VulkanInstance* instance );
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

	virtual void resize ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) = 0;
	virtual void reset ( ) = 0;

	void v_update_viewport ( Viewport<f32> viewport, VulkanRenderTarget* render_targets ) {
		if ( render_targets ) {
			m_viewport = viewport;
			printf ( "Viewport changed %g x %g | %g x %g | %g - %g\n", viewport.offset.x, viewport.offset.y, viewport.extend.width, viewport.extend.height, viewport.depth.min, viewport.depth.max );
			resize ( viewport, render_targets );
		} else {
			reset();
		}
	}
	virtual void update_viewport ( Viewport<f32> viewport, RenderTarget* target_wrapper ) {
		v_update_viewport ( viewport, dynamic_cast<VulkanRenderTarget*> ( target_wrapper ) );
	}

};
struct UIVulkanSectionFrameData {
	vk::CommandBuffer clearcmd;
};
struct UIVulkanWindowSection : public VulkanWindowSection {
	//here an optimized quad renderer
	VulkanQuadRenderer v_quad_renderer;
	
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
};
struct WorldVulkanWindowSection : public VulkanWindowSection {
	//here an optimized quad renderer


	WorldVulkanWindowSection ( VulkanInstance* instance ) : VulkanWindowSection ( WindowSectionType::eWorld, instance ) {}
	virtual ~WorldVulkanWindowSection() {}

	virtual void render_frame ( u32 index );
	virtual void set_root_node ( UINode* node ) {}
	virtual void set_stack_size ( uint32_t size ) {}
	virtual void set_layer ( u32 layer, WindowSection* section ) {}
	virtual void set_world ( World* world ) {}

	virtual void resize ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) {}
};
