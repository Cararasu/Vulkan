#pragma once

#include "VInstance.h"
#include "VHeader.h"
#include "render/Dimensions.h"
#include "render/RenderPass.h"
#include "VWindow.h"

struct VInstance;
struct SubmitStore;

struct VRenderTarget;

struct VBundleImageState {
	vk::Format current_format = vk::Format::eUndefined;
	VBaseImage* actual_image = nullptr;
	u32 miplayer, arraylayer;
	VImageUse use;
};
struct VRenderStage : public RenderStage {
	Array<VBundleImageState> v_bundlestates;
	VContextGroup* v_contextgroup;
	
	VRenderStage ( RenderStageType type ) : RenderStage(type) {}
	virtual ~VRenderStage() {}

	virtual void set_contextgroup ( ContextGroup* contextgroup ) {
		v_contextgroup = static_cast<VContextGroup*> (contextgroup);
	}
	virtual void v_dispatch ( vk::CommandBuffer buffer, u32 index ) = 0;
};

struct VRenderer {
	VInstance* const v_instance;
	bool needs_rebuild_pipeline_layout;
	bool needs_rebuild_renderpass;
	bool needs_rebuild_pipeline;

	Array<VBaseImage*> images;
	Viewport<f32> viewport;

	Array<vk::DescriptorSetLayout> descriptor_set_layouts;
	vk::DescriptorPool descriptorPool;

	vk::PipelineLayout pipeline_layout;
	vk::RenderPass renderpass;
	vk::Pipeline pipeline;

	VRenderer ( VInstance* instance, InstanceBaseId instance_base_id, const Array<const ContextBase*> contexts ) : v_instance ( instance ) {}
	~VRenderer() {}

	void v_update_image ( u32 index, VBaseImage* image );

};