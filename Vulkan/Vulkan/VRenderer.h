#pragma once

#include "VInstance.h"
#include "VHeader.h"
#include "render/Dimensions.h"
#include "render/RenderPass.h"
#include "VWindow.h"

struct VInstance;
struct SubmitStore;

struct VRenderStage : public RenderStage {
	virtual ~VRenderStage() {}
//    List of renderers
//    List of rendertargets
};

struct VRenderTarget;

//can use secondary command buffer
//or just add 
struct VRenderer : public Renderer {
	//one ModelId
	//or none for empty invocation

	VInstance* const v_instance;
	bool rebuild_on_instance_change;
	bool rebuild_every_time;//is this 
	
	Viewport<f32> viewport;

	Array<vk::DescriptorSetLayout> descriptor_set_layouts;
	vk::DescriptorPool descriptorPool;

	vk::PipelineLayout pipeline_layout;
	vk::RenderPass renderpass;
	vk::Pipeline pipeline;

	vk::CommandPool commandpool;

	VRenderer ( VInstance* instance, const ModelInstanceBase* model_instance_base, const Array<const ContextBase*> contexts ) : Renderer(model_instance_base, contexts), v_instance ( instance ) {}
	virtual ~VRenderer() {}

	//virtual RendResult render(u32 frame_index, vk::CommandBuffer commandbuffer) = 0;
	//virtual RendResult render(u32 frame_index, vk::CommandBuffer commandbuffer) = 0;
};
