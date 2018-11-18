#pragma once

#include "VInstance.h"
#include "VHeader.h"
#include "render/Dimensions.h"
#include "render/RenderPass.h"
#include "VWindow.h"

struct VInstance;
struct SubmitStore;

struct VRenderTarget;

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

	void v_update_image(u32 index, VBaseImage* image);
	
};

struct VRenderStage : public RenderStage {
	Array<RenderImageDef> input_image_defs;
	
	VRenderStage(Array<RenderImageDef> input_image_defs);
	
	virtual ~VRenderStage() {}
//    List of renderers
//    List of rendertargets
};