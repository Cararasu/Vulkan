#include "VMainBundle.h"
#include "../VImage.h"
#include "../VModel.h"
#include "../VContext.h"
#include "../VImage.h"
#include "../VInstance.h"
#include <render/Specialization.h>
#include "../VShader.h"
#include "../VResourceManager.h"
#include "../VTransformEnums.h"
#include "../VWindow.h"

void gen_pipeline_layout ( VInstance* v_instance, PipelineStruct* p_struct ) {
	if ( !p_struct->pipeline_layout ) {
		v_logger.log<LogLevel::eTrace> ( "Rebuild Pipeline Layouts" );

		DynArray<vk::DescriptorSetLayout> v_descriptor_set_layouts;
		for ( ContextBaseId id : p_struct->contextBaseId ) {
			VContextBase& v_contextbase = v_instance->contextbase_map[id];
			if ( v_contextbase.descriptorset_layout ) v_descriptor_set_layouts.push_back ( v_contextbase.descriptorset_layout );
		}
		for ( ContextBaseId id : p_struct->model_contextBaseId ) {
			VContextBase& v_contextbase = v_instance->contextbase_map[id];
			if ( v_contextbase.descriptorset_layout ) v_descriptor_set_layouts.push_back ( v_contextbase.descriptorset_layout );
		}

		std::array<vk::PushConstantRange, 0> pushConstRanges = {};//{vk::PushConstantRange ( vk::ShaderStageFlagBits::eFragment, 0, sizeof ( ObjectPartData ) ) };

		vk::PipelineLayoutCreateInfo createInfo (
		    vk::PipelineLayoutCreateFlags(),
		    0, nullptr,//setLayouts
		    0, nullptr//pushConstRanges
		);
		if ( v_descriptor_set_layouts.size() != 0 ) {
			createInfo.setLayoutCount = v_descriptor_set_layouts.size();
			createInfo.pSetLayouts = v_descriptor_set_layouts.data();
		}
		if ( pushConstRanges.size() != 0 ) {
			createInfo.pushConstantRangeCount = pushConstRanges.size();
			createInfo.pPushConstantRanges = pushConstRanges.data();
		}
		p_struct->pipeline_layout = v_instance->vk_device ().createPipelineLayout ( createInfo, nullptr );
	}
}
void gen_tex_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass ) {
	if ( !p_struct->pipeline ) {
		v_logger.log<LogLevel::eTrace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex ),
			vk::VertexInputBindingDescription ( 1, instancebase->instance_datagroup.size, vk::VertexInputRate::eInstance )
		};

		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
			valuecount += to_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
			valuecount += to_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		vertexInputAttributes.resize ( valuecount );
		{
			u32 index = 0;
			u32 bindingindex = 0;
			for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
				VFormatData formatdata = to_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::eTrace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 0, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
			for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
				VFormatData formatdata = to_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::eTrace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 1, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 1, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
		}

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo ( vk::PipelineVertexInputStateCreateFlags(),
		        vertexInputBindings.size(), vertexInputBindings.data(),
		        vertexInputAttributes.size, vertexInputAttributes.data );

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly ( vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE );

		vk::Viewport viewports[] = {
			vk::Viewport ( viewport.offset.x, viewport.offset.y, viewport.extend.width, viewport.extend.height, viewport.depth.min, viewport.depth.max )
		};

		vk::Rect2D scissors[] = {
			vk::Rect2D ( vk::Offset2D ( 0, 0 ), vk::Extent2D ( viewport.extend.width, viewport.extend.height ) ),
		};

		vk::PipelineViewportStateCreateInfo viewportState ( vk::PipelineViewportStateCreateFlags(), 1, viewports, 1, scissors );

		vk::PipelineRasterizationStateCreateInfo rasterizer ( vk::PipelineRasterizationStateCreateFlags(),
		        VK_FALSE, VK_FALSE, //depthClampEnable, rasterizerDiscardEnable
		        vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
		        VK_FALSE, //depthBiasEnable
		        0.0f, //depthBiasConstantFactor
		        0.0f, //depthBiasClamp
		        0.0f, //depthBiasSlopeFactor
		        1.0f ); //lineWidth

		vk::PipelineMultisampleStateCreateInfo multisampling (
		    vk::PipelineMultisampleStateCreateFlags(),
		    vk::SampleCountFlagBits::e1,
		    VK_FALSE,//sampleShadingEnable
		    1.0f, nullptr, //minSampleShading, pSampleMask
		    VK_FALSE, VK_FALSE //alphaToCoverageEnable, alphaToOneEnable
		);

		vk::PipelineDepthStencilStateCreateInfo depthStencil (
		    vk::PipelineDepthStencilStateCreateFlags(),
		    VK_TRUE, VK_TRUE, //depthTestEnable, depthWriteEnable
		    vk::CompareOp::eLess, //depthCompareOp
		    VK_FALSE, VK_FALSE, //depthBoundsTestEnable, stencilTestEnable
		    {}, {}, //front, back
		    0.0f, 1.0f //minDepthBounds, maxDepthBounds
		);

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[1] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_FALSE, //blendEnable
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			)
		};

		vk::PipelineColorBlendStateCreateInfo colorBlending (
		    vk::PipelineColorBlendStateCreateFlags(),
		    VK_FALSE, vk::LogicOp::eCopy,//logicOpEnable, logicOp
		    1, colorBlendAttachments, // attachments
		{0.0f, 0.0f, 0.0f, 0.0f} //blendConstants
		);

		VShaderModule* vmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "vert_textured_shader" ) );
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "frag_textured_shader" ) );

		vk::PipelineShaderStageCreateInfo shaderStages[2] = {
			vk::PipelineShaderStageCreateInfo (
			    vk::PipelineShaderStageCreateFlags(),
			    vk::ShaderStageFlagBits::eVertex, vmod->shadermodule,
			    "main", nullptr//name, specialization
			),
			vk::PipelineShaderStageCreateInfo (
			    vk::PipelineShaderStageCreateFlags(),
			    vk::ShaderStageFlagBits::eFragment, fmod->shadermodule,
			    "main", nullptr//name, specialization
			),
		};
		vk::GraphicsPipelineCreateInfo pipelineInfo (
		    vk::PipelineCreateFlags(),
		    2, shaderStages,
		    &vertexInputInfo, &inputAssembly, nullptr, &viewportState, &rasterizer, &multisampling, &depthStencil, &colorBlending,
		    nullptr,
		    p_struct->pipeline_layout,
		    renderpass,
		    0,
		    vk::Pipeline(),
		    -1
		);
		p_struct->pipeline = v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), {pipelineInfo}, nullptr ) [0];
	}
}

void gen_flat_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass ) {
	if ( !p_struct->pipeline ) {
		v_logger.log<LogLevel::eTrace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex ),
			vk::VertexInputBindingDescription ( 1, instancebase->instance_datagroup.size, vk::VertexInputRate::eInstance )
		};

		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
			valuecount += to_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
			valuecount += to_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		vertexInputAttributes.resize ( valuecount );
		{
			u32 index = 0;
			u32 bindingindex = 0;
			for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
				VFormatData formatdata = to_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::eTrace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 0, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
			for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
				VFormatData formatdata = to_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::eTrace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 1, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 1, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
		}

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo ( vk::PipelineVertexInputStateCreateFlags(),
		        vertexInputBindings.size(), vertexInputBindings.data(),
		        vertexInputAttributes.size, vertexInputAttributes.data );

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly ( vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE );

		vk::Viewport viewports[] = {
			vk::Viewport ( viewport.offset.x, viewport.offset.y, viewport.extend.width, viewport.extend.height, viewport.depth.min, viewport.depth.max )
		};

		vk::Rect2D scissors[] = {
			vk::Rect2D ( vk::Offset2D ( 0, 0 ), vk::Extent2D ( viewport.extend.width, viewport.extend.height ) ),
		};

		vk::PipelineViewportStateCreateInfo viewportState ( vk::PipelineViewportStateCreateFlags(), 1, viewports, 1, scissors );

		vk::PipelineRasterizationStateCreateInfo rasterizer ( vk::PipelineRasterizationStateCreateFlags(),
		        VK_FALSE, VK_FALSE, //depthClampEnable, rasterizerDiscardEnable
		        vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
		        VK_FALSE, //depthBiasEnable
		        0.0f, //depthBiasConstantFactor
		        0.0f, //depthBiasClamp
		        0.0f, //depthBiasSlopeFactor
		        1.0f ); //lineWidth

		vk::PipelineMultisampleStateCreateInfo multisampling (
		    vk::PipelineMultisampleStateCreateFlags(),
		    vk::SampleCountFlagBits::e1,
		    VK_FALSE,//sampleShadingEnable
		    1.0f, nullptr, //minSampleShading, pSampleMask
		    VK_FALSE, VK_FALSE //alphaToCoverageEnable, alphaToOneEnable
		);

		vk::PipelineDepthStencilStateCreateInfo depthStencil (
		    vk::PipelineDepthStencilStateCreateFlags(),
		    VK_TRUE, VK_TRUE, //depthTestEnable, depthWriteEnable
		    vk::CompareOp::eLess, //depthCompareOp
		    VK_FALSE, VK_FALSE, //depthBoundsTestEnable, stencilTestEnable
		    {}, {}, //front, back
		    0.0f, 1.0f //minDepthBounds, maxDepthBounds
		);

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[1] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_FALSE, //blendEnable
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			)
		};

		vk::PipelineColorBlendStateCreateInfo colorBlending (
		    vk::PipelineColorBlendStateCreateFlags(),
		    VK_FALSE, vk::LogicOp::eCopy,//logicOpEnable, logicOp
		    1, colorBlendAttachments, // attachments
		{0.0f, 0.0f, 0.0f, 0.0f} //blendConstants
		);

		VShaderModule* vmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "vert_flat_shader" ) );
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "frag_flat_shader" ) );

		vk::PipelineShaderStageCreateInfo shaderStages[2] = {
			vk::PipelineShaderStageCreateInfo (
			    vk::PipelineShaderStageCreateFlags(),
			    vk::ShaderStageFlagBits::eVertex, vmod->shadermodule,
			    "main", nullptr//name, specialization
			),
			vk::PipelineShaderStageCreateInfo (
			    vk::PipelineShaderStageCreateFlags(),
			    vk::ShaderStageFlagBits::eFragment, fmod->shadermodule,
			    "main", nullptr//name, specialization
			),
		};
		vk::GraphicsPipelineCreateInfo pipelineInfo (
		    vk::PipelineCreateFlags(),
		    2, shaderStages,
		    &vertexInputInfo, &inputAssembly, nullptr, &viewportState, &rasterizer, &multisampling, &depthStencil, &colorBlending,
		    nullptr,
		    p_struct->pipeline_layout,
		    renderpass,
		    0,
		    vk::Pipeline(),
		    -1
		);
		p_struct->pipeline = v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), {pipelineInfo}, nullptr ) [0];
	}
}
void gen_skybox_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass ) {
	if ( !p_struct->pipeline ) {
		v_logger.log<LogLevel::eTrace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex ),
			vk::VertexInputBindingDescription ( 1, instancebase->instance_datagroup.size, vk::VertexInputRate::eInstance )
		};

		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
			valuecount += to_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
			valuecount += to_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		vertexInputAttributes.resize ( valuecount );
		{
			u32 index = 0;
			u32 bindingindex = 0;
			for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
				VFormatData formatdata = to_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::eTrace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 0, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
			for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
				VFormatData formatdata = to_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::eTrace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 1, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 1, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
		}

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo ( vk::PipelineVertexInputStateCreateFlags(),
		        vertexInputBindings.size(), vertexInputBindings.data(),
		        vertexInputAttributes.size, vertexInputAttributes.data );

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly ( vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE );

		vk::Viewport viewports[] = {
			vk::Viewport ( viewport.offset.x, viewport.offset.y, viewport.extend.width, viewport.extend.height, viewport.depth.min, viewport.depth.max )
		};

		vk::Rect2D scissors[] = {
			vk::Rect2D ( vk::Offset2D ( 0, 0 ), vk::Extent2D ( viewport.extend.width, viewport.extend.height ) ),
		};

		vk::PipelineViewportStateCreateInfo viewportState ( vk::PipelineViewportStateCreateFlags(), 1, viewports, 1, scissors );

		vk::PipelineRasterizationStateCreateInfo rasterizer ( vk::PipelineRasterizationStateCreateFlags(),
		        VK_FALSE, VK_FALSE, //depthClampEnable, rasterizerDiscardEnable
		        vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise,
		        VK_FALSE, //depthBiasEnable
		        0.0f, //depthBiasConstantFactor
		        0.0f, //depthBiasClamp
		        0.0f, //depthBiasSlopeFactor
		        1.0f ); //lineWidth

		vk::PipelineMultisampleStateCreateInfo multisampling (
		    vk::PipelineMultisampleStateCreateFlags(),
		    vk::SampleCountFlagBits::e1,
		    VK_FALSE,//sampleShadingEnable
		    1.0f, nullptr, //minSampleShading, pSampleMask
		    VK_FALSE, VK_FALSE //alphaToCoverageEnable, alphaToOneEnable
		);

		vk::PipelineDepthStencilStateCreateInfo depthStencil (
		    vk::PipelineDepthStencilStateCreateFlags(),
		    VK_FALSE, VK_FALSE, //depthTestEnable, depthWriteEnable
		    vk::CompareOp::eLess, //depthCompareOp
		    VK_FALSE, VK_FALSE, //depthBoundsTestEnable, stencilTestEnable
		    {}, {}, //front, back
		    0.0f, 1.0f //minDepthBounds, maxDepthBounds
		);

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[1] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_FALSE, //blendEnable
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			)
		};

		vk::PipelineColorBlendStateCreateInfo colorBlending (
		    vk::PipelineColorBlendStateCreateFlags(),
		    VK_FALSE, vk::LogicOp::eCopy,//logicOpEnable, logicOp
		    1, colorBlendAttachments, // attachments
		{0.0f, 0.0f, 0.0f, 0.0f} //blendConstants
		);

		VShaderModule* vmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "vert_skybox_shader" ) );
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "frag_skybox_shader" ) );

		vk::PipelineShaderStageCreateInfo shaderStages[2] = {
			vk::PipelineShaderStageCreateInfo (
			    vk::PipelineShaderStageCreateFlags(),
			    vk::ShaderStageFlagBits::eVertex, vmod->shadermodule,
			    "main", nullptr//name, specialization
			),
			vk::PipelineShaderStageCreateInfo (
			    vk::PipelineShaderStageCreateFlags(),
			    vk::ShaderStageFlagBits::eFragment, fmod->shadermodule,
			    "main", nullptr//name, specialization
			),
		};
		vk::GraphicsPipelineCreateInfo pipelineInfo (
		    vk::PipelineCreateFlags(),
		    2, shaderStages,
		    &vertexInputInfo, &inputAssembly, nullptr, &viewportState, &rasterizer, &multisampling, &depthStencil, &colorBlending,
		    nullptr,
		    p_struct->pipeline_layout,
		    renderpass,
		    0,
		    vk::Pipeline(),
		    -1
		);
		p_struct->pipeline = v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), {pipelineInfo}, nullptr ) [0];
	}
}
void gen_shot_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass ) {
	if ( !p_struct->pipeline ) {
		v_logger.log<LogLevel::eTrace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex ),
			vk::VertexInputBindingDescription ( 1, instancebase->instance_datagroup.size, vk::VertexInputRate::eInstance )
		};

		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
			valuecount += to_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
			valuecount += to_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		vertexInputAttributes.resize ( valuecount );
		{
			u32 index = 0;
			u32 bindingindex = 0;
			for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
				VFormatData formatdata = to_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::eTrace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 0, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
			for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
				VFormatData formatdata = to_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::eTrace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 1, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 1, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
		}

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo ( vk::PipelineVertexInputStateCreateFlags(),
		        vertexInputBindings.size(), vertexInputBindings.data(),
		        vertexInputAttributes.size, vertexInputAttributes.data );

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly ( vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::ePointList, VK_FALSE );

		vk::Viewport viewports[] = {
			vk::Viewport ( viewport.offset.x, viewport.offset.y, viewport.extend.width, viewport.extend.height, viewport.depth.min, viewport.depth.max )
		};

		vk::Rect2D scissors[] = {
			vk::Rect2D ( vk::Offset2D ( 0, 0 ), vk::Extent2D ( viewport.extend.width, viewport.extend.height ) ),
		};

		vk::PipelineViewportStateCreateInfo viewportState ( vk::PipelineViewportStateCreateFlags(), 1, viewports, 1, scissors );

		vk::PipelineRasterizationStateCreateInfo rasterizer ( vk::PipelineRasterizationStateCreateFlags(),
		        VK_FALSE, VK_FALSE, //depthClampEnable, rasterizerDiscardEnable
		        vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
		        VK_FALSE, //depthBiasEnable
		        0.0f, //depthBiasConstantFactor
		        0.0f, //depthBiasClamp
		        0.0f, //depthBiasSlopeFactor
		        1.0f ); //lineWidth

		vk::PipelineMultisampleStateCreateInfo multisampling (
		    vk::PipelineMultisampleStateCreateFlags(),
		    vk::SampleCountFlagBits::e1,
		    VK_FALSE,//sampleShadingEnable
		    1.0f, nullptr, //minSampleShading, pSampleMask
		    VK_FALSE, VK_FALSE //alphaToCoverageEnable, alphaToOneEnable
		);

		vk::PipelineDepthStencilStateCreateInfo depthStencil (
		    vk::PipelineDepthStencilStateCreateFlags(),
		    VK_TRUE, VK_FALSE, //depthTestEnable, depthWriteEnable
		    vk::CompareOp::eLess, //depthCompareOp
		    VK_FALSE, VK_FALSE, //depthBoundsTestEnable, stencilTestEnable
		    {}, {}, //front, back
		    0.0f, 1.0f //minDepthBounds, maxDepthBounds
		);

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[1] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_TRUE, //blendEnable
			    vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			)
		};

		vk::PipelineColorBlendStateCreateInfo colorBlending (
		    vk::PipelineColorBlendStateCreateFlags(),
		    VK_FALSE, vk::LogicOp::eCopy,//logicOpEnable, logicOp
		    1, colorBlendAttachments, // attachments
		{0.0f, 0.0f, 0.0f, 0.0f} //blendConstants
		);

		VShaderModule* vmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "vert_shot_shader" ) );
		VShaderModule* gmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "geom_shot_shader" ) );
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "frag_shot_shader" ) );

		vk::PipelineShaderStageCreateInfo shaderStages[3] = {
			vk::PipelineShaderStageCreateInfo (
			    vk::PipelineShaderStageCreateFlags(),
			    vk::ShaderStageFlagBits::eVertex, vmod->shadermodule,
			    "main", nullptr//name, specialization
			),
			vk::PipelineShaderStageCreateInfo (
			    vk::PipelineShaderStageCreateFlags(),
			    vk::ShaderStageFlagBits::eGeometry, gmod->shadermodule,
			    "main", nullptr//name, specialization
			),
			vk::PipelineShaderStageCreateInfo (
			    vk::PipelineShaderStageCreateFlags(),
			    vk::ShaderStageFlagBits::eFragment, fmod->shadermodule,
			    "main", nullptr//name, specialization
			),
		};
		vk::GraphicsPipelineCreateInfo pipelineInfo (
		    vk::PipelineCreateFlags(),
		    3, shaderStages,
		    &vertexInputInfo, &inputAssembly, nullptr, &viewportState, &rasterizer, &multisampling, &depthStencil, &colorBlending,
		    nullptr,
		    p_struct->pipeline_layout,
		    renderpass,
		    0,
		    vk::Pipeline(),
		    -1
		);
		p_struct->pipeline = v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), {pipelineInfo}, nullptr ) [0];
	}
}

void destroy_pipeline ( VInstance* v_instance, PipelineStruct* p_struct ) {
	if ( p_struct->pipeline ) {
		v_instance->vk_device ().destroyPipeline ( p_struct->pipeline );
		p_struct->pipeline = vk::Pipeline();
	}
}
void destroy_pipeline_layout ( VInstance* v_instance, PipelineStruct* p_struct ) {
	if ( p_struct->pipeline_layout ) {
		v_instance->vk_device ().destroyPipelineLayout ( p_struct->pipeline_layout );
		p_struct->pipeline_layout = vk::PipelineLayout();
	}
}
void render_pipeline ( VInstance* v_instance, VInstanceGroup* igroup, VContextGroup* cgroup, PipelineStruct* p_struct, vk::CommandBuffer cmdbuffer ) {

	DynArray<InstanceBlock>& instanceblocks = igroup->instance_to_data_map[p_struct->instancebase_id];

	DynArray<vk::DescriptorSet> descriptorSets;
	for ( ContextBaseId id : p_struct->contextBaseId ) {
		VContext* context_ptr = cgroup->context_map[id];
		assert ( context_ptr );
		descriptorSets.push_back ( context_ptr->descriptor_set );
	}
	u32 offset = 0;
	if ( descriptorSets.size() ) cmdbuffer.bindDescriptorSets ( vk::PipelineBindPoint::eGraphics, p_struct->pipeline_layout, offset, descriptorSets.size(), descriptorSets.data(), 0, nullptr );
	offset += descriptorSets.size();
	cmdbuffer.bindPipeline ( vk::PipelineBindPoint::eGraphics, p_struct->pipeline );
	IdPtrArray<VModel>& models = v_instance->v_model_map[p_struct->modelbase_id];
	const ModelBase* modelbase_ptr = v_instance->modelbase ( p_struct->modelbase_id );
	for ( InstanceBlock& instanceblock : instanceblocks ) {
		if ( instanceblock.modelbase_id != p_struct->modelbase_id ) continue;

		VModel* v_model = models[instanceblock.model_id];

		v_logger.log<LogLevel::eDebug> ( "Instance: 0x%x ModelBase: 0x%x Model-Index: 0x%x Offset: 0x%x Count: %d", instanceblock.base_id, instanceblock.modelbase_id, instanceblock.model_id, instanceblock.offset, instanceblock.count );
		v_logger.log<LogLevel::eDebug> ( "Vertices: %d", v_model->indexcount );
		
		DynArray<vk::DescriptorSet> model_descriptorSets;
		for ( ContextBaseId id : p_struct->model_contextBaseId ) {
			bool found = false;
			for ( u32 i = 0; i < modelbase_ptr->contextbase_ids.size; i++ ) {
				if ( modelbase_ptr->contextbase_ids[i] == id ) {
					assert ( v_model->v_contexts[i] );
					model_descriptorSets.push_back ( v_model->v_contexts[i]->descriptor_set );
					found = true;
					break;
				}
			}
			assert ( found );
		}
		if ( model_descriptorSets.size() ) cmdbuffer.bindDescriptorSets ( vk::PipelineBindPoint::eGraphics, p_struct->pipeline_layout, offset, model_descriptorSets.size(), model_descriptorSets.data(), 0, nullptr );

		cmdbuffer.bindIndexBuffer ( v_model->indexbuffer.buffer, 0, v_model->index_is_2byte ? vk::IndexType::eUint16 : vk::IndexType::eUint32 );
		cmdbuffer.bindVertexBuffers ( 0, {v_model->vertexbuffer.buffer, igroup->buffer_storeage.buffer.buffer}, {0, instanceblock.offset} );
		cmdbuffer.drawIndexed ( v_model->indexcount, instanceblock.count, 0, 0, 0 );
	}
}
VMainBundle::VMainBundle ( VInstance* instance, InstanceGroup* igroup, ContextGroup* cgroup ) :
	v_instance ( instance ),
	v_igroup ( static_cast<VInstanceGroup*> ( igroup ) ),
	v_cgroup ( static_cast<VContextGroup*> ( cgroup ) ),
	v_bundleStates ( 2 ),
	tex_pipeline ( {
	simple_modelbase_id, textured_instance_base_id, { camera_context_base_id, lightvector_base_id }, {tex_simplemodel_context_base_id}
} ),
flat_pipeline ( {simple_modelbase_id, flat_instance_base_id, { camera_context_base_id, lightvector_base_id }, {flat_simplemodel_context_base_id}} ),
skybox_pipeline ( {simple_modelbase_id, skybox_instance_base_id, { }, {skybox_context_base_id}} ),
shot_pipeline ( {shot_modelbase_id, shot_instance_base_id, {camera_context_base_id}, {}} ),
v_per_frame_data ( MAX_PRESENTIMAGE_COUNT ) {

	if ( !commandpool ) {
		vk::CommandPoolCreateInfo createInfo ( vk::CommandPoolCreateFlagBits::eResetCommandBuffer, v_instance->queues.graphics_queue_id );
		v_instance->vk_device ().createCommandPool ( &createInfo, nullptr, &commandpool );
	}
}

VMainBundle::~VMainBundle() {
	v_instance->vk_device ().destroyCommandPool ( commandpool );
	commandpool = vk::CommandPool();

	v_destroy_framebuffers();
	v_destroy_pipeline_layouts();
}
void VMainBundle::v_destroy_pipeline_layouts() {
	v_destroy_renderpasses();
	v_destroy_pipelines();

	destroy_pipeline_layout ( v_instance, &tex_pipeline );
	destroy_pipeline_layout ( v_instance, &flat_pipeline );
	destroy_pipeline_layout ( v_instance, &skybox_pipeline );
	destroy_pipeline_layout ( v_instance, &shot_pipeline );
}
void VMainBundle::v_destroy_pipelines() {
	v_destroy_framebuffers();
	destroy_pipeline ( v_instance, &tex_pipeline );
	destroy_pipeline ( v_instance, &flat_pipeline );
	destroy_pipeline ( v_instance, &skybox_pipeline );
	destroy_pipeline ( v_instance, &shot_pipeline );
	for ( PerFrameMainBundleRenderObj& data : v_per_frame_data ) {
		data.command.should_reset = true;
	}
}
void VMainBundle::v_destroy_renderpasses() {
	if ( v_renderpass ) {
		v_instance->vk_device ().destroyRenderPass ( v_renderpass );
		v_renderpass = vk::RenderPass();
	}
}
void VMainBundle::v_destroy_framebuffers() {
	for ( PerFrameMainBundleRenderObj& data : v_per_frame_data ) {
		if ( data.framebuffer ) {
			v_instance->vk_device ().destroyFramebuffer ( data.framebuffer );
			data.framebuffer = vk::Framebuffer();
		}
	}
}

void VMainBundle::set_rendertarget ( u32 index, Image* image ) {
	assert ( index < v_bundleStates.size );
	VBundleImageState& imagestate = v_bundleStates[index];
	imagestate.actual_image = static_cast<VBaseImage*> ( image );

	if ( imagestate.actual_image->v_format != imagestate.current_format ) {
		imagestate.current_format = imagestate.actual_image->v_format;
		v_destroy_renderpasses();
	}
	v_destroy_framebuffers();
}
void VMainBundle::v_check_rebuild() {
	u32 width = 0, height = 0;
	for ( VBundleImageState& imagestate : v_bundleStates ) {
		if ( !imagestate.actual_image ) {
			v_logger.log<LogLevel::eWarn>  ( "One or more Images not set for MainBundle" );
			return;
		}
		if ( last_frame_index_pipeline_built < imagestate.actual_image->created_frame_index ) {
			v_logger.log<LogLevel::eTrace> ( "Last Frame Index Pipeline Built %" PRId64 "", last_frame_index_pipeline_built );
			v_logger.log<LogLevel::eTrace> ( "Last Image built index %" PRId64 "", imagestate.actual_image->created_frame_index );
			v_destroy_pipelines();
		}

		assert ( !width || width == imagestate.actual_image->extent.width );
		assert ( !height || height == imagestate.actual_image->extent.height );
		width = imagestate.actual_image->extent.width;
		height = imagestate.actual_image->extent.height;

	}
	if ( viewport.extend.width != width || viewport.extend.height != height ) {
		viewport = Viewport<f32> ( 0.0f, 0.0f, width, height, 0.0f, 1.0f );
		v_destroy_pipelines();
	}
}

void VMainBundle::v_rebuild_pipelines() {
	gen_pipeline_layout ( v_instance, &tex_pipeline );
	gen_pipeline_layout ( v_instance, &flat_pipeline );
	gen_pipeline_layout ( v_instance, &skybox_pipeline );
	gen_pipeline_layout ( v_instance, &shot_pipeline );
	if ( !v_renderpass ) {
		v_logger.log<LogLevel::eDebug> ( "Rebuild Renderpasses" );
		vk::AttachmentDescription attachments[2] = {
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            v_bundleStates[0].current_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eClear,//loadOp
			                            vk::AttachmentStoreOp::eStore,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eColorAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eColorAttachmentOptimal//finalLayout
			                          ),
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            v_bundleStates[1].current_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eClear,//loadOp
			                            vk::AttachmentStoreOp::eDontCare,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eDepthStencilAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eDepthStencilAttachmentOptimal//finalLayout
			                          )
		};

		vk::AttachmentReference colorAttachmentRefs[] = {
			vk::AttachmentReference ( 0, vk::ImageLayout::eColorAttachmentOptimal )
		};
		vk::AttachmentReference depthAttachmentRef ( 1, vk::ImageLayout::eDepthStencilAttachmentOptimal );

		vk::SubpassDescription subpasses[] = {
			vk::SubpassDescription (
			    vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics,
			    0, nullptr/*inputAttachments*/,
			    1, colorAttachmentRefs/*colorAttachments*/,
			    nullptr,/*resolveAttachments*/
			    &depthAttachmentRef,/*depthAttackment*/
			    0, nullptr/*preserveAttachments*/
			)
		};

		vk::RenderPassCreateInfo renderPassInfo ( vk::RenderPassCreateFlags(), 2, attachments, 1, subpasses, 0, nullptr/*dependencies*/ );

		v_renderpass = v_instance->vk_device ().createRenderPass ( renderPassInfo, nullptr );
	}
	gen_tex_pipeline ( v_instance, &tex_pipeline, viewport, v_renderpass );
	gen_flat_pipeline ( v_instance, &flat_pipeline, viewport, v_renderpass );
	gen_skybox_pipeline ( v_instance, &skybox_pipeline, viewport, v_renderpass );
	gen_shot_pipeline ( v_instance, &shot_pipeline, viewport, v_renderpass );
	last_frame_index_pipeline_built = v_instance->frame_index;
}
void VMainBundle::v_rebuild_commandbuffer ( u32 index ) {
	PerFrameMainBundleRenderObj& data = v_per_frame_data[index];
	if ( !data.framebuffer ) {
		vk::ImageView attachments[2] = {v_bundleStates[0].actual_image->instance_imageview ( index ), v_bundleStates[1].actual_image->instance_imageview ( index ) };
		vk::FramebufferCreateInfo frameBufferCreateInfo = {
			vk::FramebufferCreateFlags(), v_renderpass,
			2, attachments,
			( u32 ) viewport.extend.width, ( u32 ) viewport.extend.height, 1
		};
		data.framebuffer = v_instance->vk_device ().createFramebuffer ( frameBufferCreateInfo );
	}
	if ( data.command.should_reset ) {
		if ( data.command.buffer ) {
			data.command.buffer.reset ( vk::CommandBufferResetFlags() );
		} else {
			data.command.buffer = v_instance->createCommandBuffer ( commandpool, vk::CommandBufferLevel::ePrimary );
		}
		vk::CommandBufferBeginInfo begininfo = {
			vk::CommandBufferUsageFlags(), nullptr
		};
		data.command.buffer.begin ( begininfo );

		DynArray<InstanceBlock>& instanceblocks = v_igroup->instance_to_data_map[textured_instance_base_id];

		{
			//update the incstancedata
			void* instance_data = v_igroup->buffer_storeage.allocate_transfer_buffer();
			for ( auto it = v_igroup->instance_to_data_map.begin(); it != v_igroup->instance_to_data_map.end(); it++ ) {
				const InstanceBase* instancebase = v_instance->instancebase ( it->first );
				for ( InstanceBlock& block : it->second ) {
					if ( block.data ) memcpy ( instance_data + block.offset, block.data, instancebase->instance_datagroup.size * block.count );
				}
			}
			v_igroup->buffer_storeage.transfer_data ( data.command.buffer );

			//for each context in the contextgroup update them
			for ( auto it = v_cgroup->context_map.begin(); it != v_cgroup->context_map.end(); it++ ) {
				VContext* context = it->second;
				if ( context->last_frame_index_updated == v_instance->frame_index ) continue;
				for ( VBaseImage* v_image : context->images ) {
					if ( v_image ) v_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, data.command.buffer );
				}
				if ( context->data ) {
					u64 size = context->v_buffer.size;
					VBuffer* staging_buffer = v_instance->request_staging_buffer ( size );
					staging_buffer->map_mem();
					memcpy ( staging_buffer->mapped_ptr, context->data, size );
					vk::BufferCopy buffercopy ( 0, 0, size );
					data.command.buffer.copyBuffer ( staging_buffer->buffer, context->v_buffer.buffer, 1, &buffercopy );
					v_instance->free_staging_buffer ( staging_buffer );
					context->data = nullptr;
				}
				context->last_frame_index_updated = v_instance->frame_index;
			}
			//for every modelbaseid
			for ( auto it = v_instance->v_model_map.begin(); it != v_instance->v_model_map.end(); it++ ) {
				//for each model
				for ( VModel* v_model : it->second ) {
					if ( !v_model ) continue;
					//update each context
					for ( VContext* context : v_model->v_contexts ) {
						if ( !context ) continue;
						if ( context->last_frame_index_updated == v_instance->frame_index ) continue;
						for ( VBaseImage* v_image : context->images ) {
							if ( v_image ) v_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, data.command.buffer );
						}
						if ( context->data ) {
							u64 size = context->v_buffer.size;
							VBuffer* staging_buffer = v_instance->request_staging_buffer ( size );
							staging_buffer->map_mem();
							memcpy ( staging_buffer->mapped_ptr, context->data, size );
							vk::BufferCopy buffercopy ( 0, 0, size );
							data.command.buffer.copyBuffer ( staging_buffer->buffer, context->v_buffer.buffer, 1, &buffercopy );
							v_instance->free_staging_buffer ( staging_buffer );
							context->data = nullptr;
						}
						context->last_frame_index_updated = v_instance->frame_index;
					}
				}
			}
			//barrier for access
			vk::BufferMemoryBarrier bufferMemoryBarrier[] = {
				vk::BufferMemoryBarrier ( vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eVertexAttributeRead,
				                          v_instance->queue_wrapper()->graphics_queue_id, v_instance->queue_wrapper()->graphics_queue_id,
				                          v_igroup->buffer_storeage.buffer.buffer, 0, v_igroup->buffer_storeage.buffer.size )
			};
			data.command.buffer.pipelineBarrier (
			    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eVertexInput,
			    vk::DependencyFlags(),
			    0, nullptr,//memoryBarrier
			    1, bufferMemoryBarrier,//bufferMemoryBarrier
			    0, nullptr//imageMemoryBarrier
			);
		}

		vk::ClearValue clearColors[2] = {
			vk::ClearValue ( vk::ClearColorValue ( std::array<float, 4> ( {0.0f, 0.0f, 0.5f, 0.0f} ) ) ),
			vk::ClearValue ( vk::ClearDepthStencilValue ( 1.0f, 0 ) )
		};
		vk::RenderPassBeginInfo renderPassBeginInfo = {
			v_renderpass, data.framebuffer,
			vk::Rect2D ( vk::Offset2D ( viewport.offset.x, viewport.offset.y ), vk::Extent2D ( viewport.extend.x, viewport.extend.y ) ),
			2, clearColors
		};
		v_bundleStates[0].actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, data.command.buffer, index );
		v_bundleStates[1].actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, data.command.buffer, index );

		data.command.buffer.beginRenderPass ( renderPassBeginInfo, vk::SubpassContents::eInline );

		render_pipeline ( v_instance, v_igroup, v_cgroup, &skybox_pipeline, data.command.buffer );
		render_pipeline ( v_instance, v_igroup, v_cgroup, &tex_pipeline, data.command.buffer );
		render_pipeline ( v_instance, v_igroup, v_cgroup, &flat_pipeline, data.command.buffer );
		render_pipeline ( v_instance, v_igroup, v_cgroup, &shot_pipeline, data.command.buffer );

		data.command.buffer.endRenderPass();
		if ( v_bundleStates[0].actual_image->window_target ) {
			v_bundleStates[0].actual_image->transition_layout ( vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, data.command.buffer, index );
			v_bundleStates[1].actual_image->transition_layout ( vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eGeneral, data.command.buffer, index );
		}
		data.command.buffer.end();
		//data.command.should_reset = false;
	}
}
void VMainBundle::v_dispatch() {
	v_check_rebuild();
	v_rebuild_pipelines();

	v_logger.log<LogLevel::eWarn> ( "--------------- FrameBoundary %d ---------------", v_instance->frame_index );
	VWindow* window = v_bundleStates[0].actual_image->window;
	window->prepare_frame();
	window->current_framelocal_data()->frame_index = v_instance->frame_index;

	QueueWrapper* queue_wrapper = &v_instance->queues;

	vk::PipelineStageFlags waitDstStageMask ( vk::PipelineStageFlagBits::eBottomOfPipe );

	FrameLocalData* data = window->current_framelocal_data();

	v_rebuild_commandbuffer ( v_bundleStates[0].actual_image->current_index );

	vk::SubmitInfo submitinfos[1] = { vk::SubmitInfo (
	                                      1, &window->image_available_guard_sem, //waitSem
	                                      &waitDstStageMask,
	                                      1, &v_per_frame_data[v_bundleStates[0].actual_image->current_index].command.buffer,//commandbuffers
	                                      1, &data->present_ready_sem//signalSem
	                                  )
	                                };

	queue_wrapper->graphics_queue.submit ( 1, submitinfos, data->image_presented_fence );

	if ( !queue_wrapper->combined_graphics_present_queue ) {
		//@TODO synchronize
		assert ( false );
	}
	//present image
	vk::Result results;
	vk::PresentInfoKHR presentInfo (
	    1, &data->present_ready_sem,
	    //active_sems.size(), active_sems.data(),
	    1, &window->swap_chain,
	    &v_bundleStates[0].actual_image->current_index, &results );
	queue_wrapper->present_queue.presentKHR ( &presentInfo );
	last_used = v_instance->frame_index;

	v_instance->frame_index++;
}
