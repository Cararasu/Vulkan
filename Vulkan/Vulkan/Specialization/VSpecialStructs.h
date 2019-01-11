#pragma once

#include <render/Model.h>
#include <render/Resources.h>

#include "../VHeader.h"
#include "../VRenderer.h"
#include <render/Dimensions.h>
#include "../VWindow.h"

struct VInstanceGroup;
struct VContextGroup;
struct VBaseImage;
struct VInstance;

struct PerFrameRenderObj {
	vk::Framebuffer framebuffer;
	std::array<VImageUseRef, 5> images;
	ResettableCommandBuffer command;
};
struct RenderPassWrapper {
	vk::RenderPass renderpass;
	vk::Framebuffer framebuffer;
	std::array<VImageUseRef, 5> images;
};

struct PipelineStruct {
	ModelBaseId modelbase_id;
	InstanceBaseId instancebase_id;
	Array<ContextBaseId> contextBaseId;
	Array<ContextBaseId> model_contextBaseId;
	vk::PipelineLayout pipeline_layout;
	Array<vk::Pipeline> pipelines;
	
	PipelineStruct(ModelBaseId modelbase_id, InstanceBaseId instancebase_id, Array<ContextBaseId> contextBaseId, Array<ContextBaseId> model_contextBaseId, u32 pipeline_count = 1) :
		modelbase_id(modelbase_id), instancebase_id(instancebase_id), contextBaseId(contextBaseId), model_contextBaseId(model_contextBaseId), pipeline_layout(), pipelines(pipeline_count) {
			
		}
};

struct SubPassInput {
	vk::DescriptorSetLayout ds_layout;
	vk::DescriptorSet ds_set;
	Array<VImageUseRef> images_used;
};
struct PushConstUsed {
	u32 offset;
	u32 size;
};