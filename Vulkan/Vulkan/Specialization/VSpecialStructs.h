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

struct PipelineStruct {
	ModelBaseId modelbase_id;
	InstanceBaseId instancebase_id;
	Array<ContextBaseId> contextBaseId;
	Array<ContextBaseId> model_contextBaseId;
	vk::PipelineLayout pipeline_layout;
	vk::Pipeline pipeline;
	
	PipelineStruct(ModelBaseId modelbase_id, InstanceBaseId instancebase_id, Array<ContextBaseId> contextBaseId, Array<ContextBaseId> model_contextBaseId) :
		modelbase_id(modelbase_id), instancebase_id(instancebase_id), contextBaseId(contextBaseId), model_contextBaseId(model_contextBaseId), pipeline_layout(), pipeline() {
			
		}
};

struct SubPassInput {
	vk::DescriptorSetLayout ds_layout;
	vk::DescriptorSet ds_set;
	Array<VImageUseRef> images_used;
};