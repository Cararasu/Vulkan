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

void gen_pipeline_layout ( VInstance* v_instance, SubPassInput* subpass_input, PipelineStruct* p_struct ) {
	if ( !p_struct->pipeline_layout ) {
		v_logger.log<LogLevel::eTrace> ( "Rebuild Pipeline Layouts" );

		DynArray<vk::DescriptorSetLayout> v_descriptor_set_layouts;
		if ( subpass_input->ds_layout ) v_descriptor_set_layouts.push_back ( subpass_input->ds_layout );
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
		VK_FALSE, VK_TRUE, { //depthBoundsTestEnable, stencilTestEnable
			vk::StencilOp::eKeep/*failOp*/, vk::StencilOp::eReplace/*passOp*/, vk::StencilOp::eKeep/*depthFailOp*/,
			vk::CompareOp::eAlways/*compareOp*/,
			0xFFFFFFFF/*compareMask*/, 0xFFFFFFFF/*writeMask*/, 1/*reference*/
		}, {}, //front, back
		0.0f, 1.0f //minDepthBounds, maxDepthBounds
		);

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_FALSE, //blendEnable
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			),
			vk::PipelineColorBlendAttachmentState (
			    VK_FALSE, //blendEnable
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			),
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
		    3, colorBlendAttachments, // attachments
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
		VK_FALSE, VK_TRUE, { //depthBoundsTestEnable, stencilTestEnable
			vk::StencilOp::eKeep/*failOp*/, vk::StencilOp::eReplace/*passOp*/, vk::StencilOp::eKeep/*depthFailOp*/,
			vk::CompareOp::eAlways/*compareOp*/,
			0xFFFFFFFF/*compareMask*/, 0xFFFFFFFF/*writeMask*/, 1/*reference*/
		}, {}, //front, back
		0.0f, 1.0f //minDepthBounds, maxDepthBounds
		);

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_FALSE, //blendEnable
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			),
			vk::PipelineColorBlendAttachmentState (
			    VK_FALSE, //blendEnable
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			),
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
		    3, colorBlendAttachments, // attachments
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

		std::array<vk::VertexInputBindingDescription, 1> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex )
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

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_FALSE, //blendEnable
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			),
			vk::PipelineColorBlendAttachmentState (
			    VK_FALSE, //blendEnable
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			),
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
		    3, colorBlendAttachments, // attachments
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
		    VK_TRUE, VK_TRUE, //depthTestEnable, depthWriteEnable
		    vk::CompareOp::eLess, //depthCompareOp
		VK_FALSE, VK_TRUE, { //depthBoundsTestEnable, stencilTestEnable
			vk::StencilOp::eKeep/*failOp*/, vk::StencilOp::eReplace/*passOp*/, vk::StencilOp::eKeep/*depthFailOp*/,
			vk::CompareOp::eAlways/*compareOp*/,
			0xFFFFFFFF/*compareMask*/, 0xFFFFFFFF/*writeMask*/, 0/*reference*/
		}, {}, //front, back
		0.0f, 1.0f //minDepthBounds, maxDepthBounds
		);

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_TRUE, //blendEnable
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //eOneMinusSrcAlpha, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			),
			vk::PipelineColorBlendAttachmentState (
			    VK_TRUE, //blendEnable
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //eOneMinusSrcAlpha, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			),
			vk::PipelineColorBlendAttachmentState (
			    VK_TRUE, //blendEnable
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //eOneMinusSrcAlpha, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			)
		};

		vk::PipelineColorBlendStateCreateInfo colorBlending (
		    vk::PipelineColorBlendStateCreateFlags(),
		    VK_FALSE, vk::LogicOp::eCopy,//logicOpEnable, logicOp
		    3, colorBlendAttachments, // attachments
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

void gen_engine_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass ) {
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

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_TRUE, //blendEnable
			    vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			),
			vk::PipelineColorBlendAttachmentState (
			    VK_TRUE, //blendEnable
			    vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, //srcColorBlendFactor, dstColorBlendFactor
			    vk::BlendOp::eAdd,//colorBlendOp
			    vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			    vk::BlendOp::eAdd,//alphaBlendOp
			    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			),
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
		    3, colorBlendAttachments, // attachments
		{0.0f, 0.0f, 0.0f, 0.0f} //blendConstants
		);

		VShaderModule* vmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "vert_shot_shader" ) );
		VShaderModule* gmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "geom_shot_shader" ) );
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "frag_engine_shader" ) );

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
void gen_lightless_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass ) {
	if ( !p_struct->pipeline ) {
		v_logger.log<LogLevel::eTrace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 1> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex )
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
		    VK_FALSE, VK_FALSE, //depthTestEnable, depthWriteEnable
		    vk::CompareOp::eLess, //depthCompareOp
			VK_FALSE, VK_TRUE, { //depthBoundsTestEnable, stencilTestEnable
				vk::StencilOp::eKeep/*failOp*/, vk::StencilOp::eKeep/*passOp*/, vk::StencilOp::eKeep/*depthFailOp*/,
				vk::CompareOp::eEqual/*compareOp*/,
				0xFFFFFFFF/*compareMask*/, 0xFFFFFFFF/*writeMask*/, 0/*reference*/
			}, {}, //front, back
			0.0f, 1.0f //minDepthBounds, maxDepthBounds
		);

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_TRUE, //blendEnable
			    vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOne, //srcColorBlendFactor, dstColorBlendFactor
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

		VShaderModule* vmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "passthrough_shader" ) );
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "lightless_shader" ) );

		vk::PipelineShaderStageCreateInfo shaderStages[3] = {
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
		    1,/*subpass*/
		    vk::Pipeline(),
		    -1
		);
		p_struct->pipeline = v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), {pipelineInfo}, nullptr ) [0];
	}
}
void gen_dirlight_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass ) {
	if ( !p_struct->pipeline ) {
		v_logger.log<LogLevel::eTrace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 1> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex )
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
		    VK_FALSE, VK_FALSE, //depthTestEnable, depthWriteEnable
		    vk::CompareOp::eLess, //depthCompareOp
		VK_FALSE, VK_TRUE, { //depthBoundsTestEnable, stencilTestEnable
			vk::StencilOp::eKeep/*failOp*/, vk::StencilOp::eKeep/*passOp*/, vk::StencilOp::eKeep/*depthFailOp*/,
			vk::CompareOp::eEqual/*compareOp*/,
			0xFFFFFFFF/*compareMask*/, 0xFFFFFFFF/*writeMask*/, 1/*reference*/
		}, {}, //front, back
		0.0f, 1.0f //minDepthBounds, maxDepthBounds
		);

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_TRUE, //blendEnable
			    vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOne, //srcColorBlendFactor, dstColorBlendFactor
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

		VShaderModule* vmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "passthrough_shader" ) );
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "dirlight_shader" ) );

		vk::PipelineShaderStageCreateInfo shaderStages[3] = {
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
		    1,/*subpass*/
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
void render_pipeline ( VInstance* v_instance, VInstanceGroup* igroup, VContextGroup* cgroup, PipelineStruct* p_struct, SubPassInput* renderpass_struct, vk::CommandBuffer cmdbuffer ) {

	DynArray<InstanceBlock>& instanceblocks = igroup->instance_to_data_map[p_struct->instancebase_id];

	u32 descriptor_offset = 0;

	if ( renderpass_struct->ds_set ) {
		cmdbuffer.bindDescriptorSets ( vk::PipelineBindPoint::eGraphics, p_struct->pipeline_layout, descriptor_offset, 1, &renderpass_struct->ds_set, 0, nullptr );
		descriptor_offset += 1;
	}

	DynArray<vk::DescriptorSet> descriptorSets;
	for ( ContextBaseId id : p_struct->contextBaseId ) {
		VContext* context_ptr = cgroup->context_map[id];
		assert ( context_ptr );
		descriptorSets.push_back ( context_ptr->descriptor_set );
	}
	if ( descriptorSets.size() ) {
		cmdbuffer.bindDescriptorSets ( vk::PipelineBindPoint::eGraphics, p_struct->pipeline_layout, descriptor_offset, descriptorSets, {} );
		descriptor_offset += descriptorSets.size();
	}
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
		if ( model_descriptorSets.size() ) {
			cmdbuffer.bindDescriptorSets ( vk::PipelineBindPoint::eGraphics, p_struct->pipeline_layout, descriptor_offset, model_descriptorSets, {} );
		}

		cmdbuffer.bindIndexBuffer ( v_model->indexbuffer.buffer, 0, v_model->index_is_2byte ? vk::IndexType::eUint16 : vk::IndexType::eUint32 );

		if ( instanceblock.data )
			cmdbuffer.bindVertexBuffers ( 0, {v_model->vertexbuffer.buffer, igroup->buffer_storeage.buffer.buffer}, {0, instanceblock.offset} );
		else
			cmdbuffer.bindVertexBuffers ( 0, {v_model->vertexbuffer.buffer}, {0} );
		cmdbuffer.drawIndexed ( v_model->indexcount, instanceblock.count, 0, 0, 0 );
	}
}
VMainRenderStage::VMainRenderStage ( VInstance* instance, InstanceGroup* igroup, ContextGroup* cgroup ) :
	VRenderStage ( RenderStageType::eRendering ),
	v_instance ( instance ),
	v_igroup ( static_cast<VInstanceGroup*> ( igroup ) ),
	v_cgroup ( static_cast<VContextGroup*> ( cgroup ) ),
	tex_pipeline ( {
	simple_modelbase_id, textured_instance_base_id, { camera_context_base_id, lightvector_base_id }, {tex_simplemodel_context_base_id}
} ),
flat_pipeline ( {simple_modelbase_id, flat_instance_base_id, { camera_context_base_id, lightvector_base_id }, {flat_simplemodel_context_base_id}} ),
skybox_pipeline ( {simple_modelbase_id, skybox_instance_base_id, { }, {skybox_context_base_id}} ),
shot_pipeline ( {dot_modelbase_id, shot_instance_base_id, {camera_context_base_id}, {}} ),
engine_pipeline ( {dot_modelbase_id, engine_instance_base_id, {camera_context_base_id}, {}} ),
dirlight_pipeline ( {fullscreen_modelbase_id, dirlight_instance_base_id, {inverse_camera_context_base_id, lightvector_base_id}, {}} ),
lightless_pipeline ( {fullscreen_modelbase_id, lightless_instance_base_id, {}, {}} ),
v_per_frame_data ( MAX_PRESENTIMAGE_COUNT ),
subpass_inputs ( 2 ) {

	v_bundlestates.resize ( 5 );
	if ( !commandpool ) {
		vk::CommandPoolCreateInfo createInfo ( vk::CommandPoolCreateFlagBits::eResetCommandBuffer, v_instance->queues.graphics_queue_id );
		v_instance->vk_device ().createCommandPool ( &createInfo, nullptr, &commandpool );
	}

	Array<vk::DescriptorPoolSize> poolsizes ( 1 );
	poolsizes[0].descriptorCount = 10;
	poolsizes[0].type = vk::DescriptorType::eInputAttachment;
	vk::DescriptorPoolCreateInfo poolInfo ( vk::DescriptorPoolCreateFlags(), 2, poolsizes.size, poolsizes.data );
	input_ds_pool = v_instance->vk_device().createDescriptorPool ( poolInfo );

	{
		subpass_inputs[0].ds_layout = vk::DescriptorSetLayout();
		subpass_inputs[0].ds_set = vk::DescriptorSet();
	}
	{
		vk::DescriptorSetLayoutBinding binding[4] = {
			vk::DescriptorSetLayoutBinding ( 0, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment, nullptr ),
			vk::DescriptorSetLayoutBinding ( 1, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment, nullptr ),
			vk::DescriptorSetLayoutBinding ( 2, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment, nullptr ),
			vk::DescriptorSetLayoutBinding ( 3, vk::DescriptorType::eInputAttachment, 1, vk::ShaderStageFlagBits::eFragment, nullptr )
		};
		subpass_inputs[1].ds_layout = v_instance->vk_device ().createDescriptorSetLayout ( vk::DescriptorSetLayoutCreateInfo ( vk::DescriptorSetLayoutCreateFlags(), 4, binding ), nullptr );

		vk::DescriptorSetAllocateInfo allocInfo ( input_ds_pool, 1, &subpass_inputs[1].ds_layout );
		v_instance->vk_device().allocateDescriptorSets ( &allocInfo, &subpass_inputs[1].ds_set );
		
		subpass_inputs[1].images_used.resize(4);
		
		subpass_inputs[1].images_used[0] = {
			0, nullptr,
			{0, 1}, {0, 1},
			vk::ImageView(), vk::ImageAspectFlagBits::eColor
		};
		subpass_inputs[1].images_used[1] = {
			0, nullptr,
			{0, 1}, {0, 1},
			vk::ImageView(), vk::ImageAspectFlagBits::eColor
		};
		subpass_inputs[1].images_used[2] = {
			0, nullptr,
			{0, 1}, {0, 1},
			vk::ImageView(), vk::ImageAspectFlagBits::eColor
		};
		subpass_inputs[1].images_used[3] = {
			0, nullptr,
			{0, 1}, {0, 1},
			vk::ImageView(), vk::ImageAspectFlagBits::eDepth
		};
	}
}

VMainRenderStage::~VMainRenderStage() {
	v_instance->vk_device ().destroyCommandPool ( commandpool );
	commandpool = vk::CommandPool();

	v_instance->vk_device().destroyDescriptorPool ( input_ds_pool );

	for ( SubPassInput& subpass_input : subpass_inputs ) {
		v_instance->vk_device().destroyDescriptorSetLayout ( subpass_input.ds_layout );
	}

	v_destroy_framebuffers();
	v_destroy_pipeline_layouts();
}
void VMainRenderStage::v_destroy_pipeline_layouts() {
	v_destroy_renderpasses();
	v_destroy_pipelines();

	destroy_pipeline_layout ( v_instance, &tex_pipeline );
	destroy_pipeline_layout ( v_instance, &flat_pipeline );
	destroy_pipeline_layout ( v_instance, &skybox_pipeline );
	destroy_pipeline_layout ( v_instance, &shot_pipeline );
	destroy_pipeline_layout ( v_instance, &engine_pipeline );
	destroy_pipeline_layout ( v_instance, &lightless_pipeline );
	destroy_pipeline_layout ( v_instance, &dirlight_pipeline );
}
void VMainRenderStage::v_destroy_pipelines() {
	v_destroy_framebuffers();
	destroy_pipeline ( v_instance, &tex_pipeline );
	destroy_pipeline ( v_instance, &flat_pipeline );
	destroy_pipeline ( v_instance, &skybox_pipeline );
	destroy_pipeline ( v_instance, &shot_pipeline );
	destroy_pipeline ( v_instance, &engine_pipeline );
	destroy_pipeline ( v_instance, &lightless_pipeline );
	destroy_pipeline ( v_instance, &dirlight_pipeline );
	for ( PerFrameMainBundleRenderObj& data : v_per_frame_data ) {
		data.command.should_reset = true;
	}
}
void VMainRenderStage::v_destroy_renderpasses() {
	if ( v_renderpass ) {
		v_instance->vk_device ().destroyRenderPass ( v_renderpass );
		v_renderpass = vk::RenderPass();
	}
}
void VMainRenderStage::v_destroy_framebuffers() {
	for ( PerFrameMainBundleRenderObj& data : v_per_frame_data ) {
		if ( data.framebuffer ) {
			v_instance->vk_device ().destroyFramebuffer ( data.framebuffer );
			data.framebuffer = vk::Framebuffer();
		}
	}
}

void VMainRenderStage::set_rendertarget ( u32 index, Image* image ) {
	assert ( index < v_bundlestates.size );
	VBundleImageState& imagestate = v_bundlestates[index];
	if(imagestate.use.imageview) {
		imagestate.actual_image->delete_use(imagestate.use.id);
		imagestate.use.imageview = vk::ImageView();
	} 
	imagestate.actual_image = static_cast<VBaseImage*> ( image );
	if ( imagestate.actual_image->v_format != imagestate.current_format ) {
		imagestate.current_format = imagestate.actual_image->v_format;
		v_destroy_renderpasses();
	}
	v_destroy_framebuffers();
}
void VMainRenderStage::set_renderwindow ( u32 index, Window* window ) {
	//TODO implement
	assert(false);
}
void VMainRenderStage::v_check_rebuild() {
	u32 width = 0, height = 0;
	for ( VBundleImageState& imagestate : v_bundlestates ) {
		if ( !imagestate.actual_image ) {
			v_logger.log<LogLevel::eWarn>  ( "One or more Images not set for MainBundle" );
			continue;
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

void VMainRenderStage::v_rebuild_pipelines() {
	gen_pipeline_layout ( v_instance, &subpass_inputs[0], &tex_pipeline );
	gen_pipeline_layout ( v_instance, &subpass_inputs[0], &flat_pipeline );
	gen_pipeline_layout ( v_instance, &subpass_inputs[0], &skybox_pipeline );
	gen_pipeline_layout ( v_instance, &subpass_inputs[0], &shot_pipeline );
	gen_pipeline_layout ( v_instance, &subpass_inputs[0], &engine_pipeline );

	gen_pipeline_layout ( v_instance, &subpass_inputs[1], &lightless_pipeline );
	gen_pipeline_layout ( v_instance, &subpass_inputs[1], &dirlight_pipeline );

	if ( !v_renderpass ) {
		v_logger.log<LogLevel::eDebug> ( "Rebuild Renderpasses" );
		std::array<vk::AttachmentDescription, 5> attachments = {
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            v_bundlestates[0].current_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eClear,//loadOp
			                            vk::AttachmentStoreOp::eStore,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eColorAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eColorAttachmentOptimal//finalLayout
			                          ),
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            v_bundlestates[1].current_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eClear,//loadOp
			                            vk::AttachmentStoreOp::eStore,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eColorAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eColorAttachmentOptimal//finalLayout
			                          ),
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            v_bundlestates[2].current_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eClear,//loadOp
			                            vk::AttachmentStoreOp::eStore,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eColorAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eColorAttachmentOptimal//finalLayout
			                          ),
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            v_bundlestates[3].current_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eClear,//loadOp
			                            vk::AttachmentStoreOp::eStore,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eColorAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eColorAttachmentOptimal//finalLayout
			                          ),
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            v_bundlestates[4].current_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eClear,//loadOp
			                            vk::AttachmentStoreOp::eStore,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eDepthStencilAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eDepthStencilAttachmentOptimal//finalLayout
			                          )
		};

		vk::AttachmentReference colorAttachmentRefs1[] = {
			vk::AttachmentReference ( 0, vk::ImageLayout::eColorAttachmentOptimal ),
			vk::AttachmentReference ( 1, vk::ImageLayout::eColorAttachmentOptimal ),
			vk::AttachmentReference ( 2, vk::ImageLayout::eColorAttachmentOptimal )
		};
		vk::AttachmentReference depthAttachmentRef1 ( 4, vk::ImageLayout::eDepthStencilAttachmentOptimal );

		vk::AttachmentReference inputAttachmentRefs2[] = {
			vk::AttachmentReference ( 0, vk::ImageLayout::eShaderReadOnlyOptimal ),
			vk::AttachmentReference ( 1, vk::ImageLayout::eShaderReadOnlyOptimal ),
			vk::AttachmentReference ( 2, vk::ImageLayout::eShaderReadOnlyOptimal ),
			vk::AttachmentReference ( 4, vk::ImageLayout::eGeneral )
		};
		vk::AttachmentReference depthAttachmentRef2 ( 4, vk::ImageLayout::eGeneral );
		vk::AttachmentReference colorAttachmentRefs2[] = {
			vk::AttachmentReference ( 3, vk::ImageLayout::eColorAttachmentOptimal )
		};

		std::array<vk::SubpassDescription, 2> subpasses = {
			vk::SubpassDescription (
			    vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics,
			    0, nullptr/*inputAttachments*/,
			    3, colorAttachmentRefs1/*colorAttachments*/,
			    nullptr,/*resolveAttachments*/
			    &depthAttachmentRef1,/*depthAttackment*/
			    0, nullptr/*preserveAttachments*/
			),
			vk::SubpassDescription (
			    vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics,
			    4, inputAttachmentRefs2/*inputAttachments*/,
			    1, colorAttachmentRefs2/*colorAttachments*/,
			    nullptr,/*resolveAttachments*/
			    &depthAttachmentRef2,/*depthAttackment*/
			    0, nullptr/*preserveAttachments*/
			)
		};

		std::array<vk::SubpassDependency, 1> dependencies = {
			vk::SubpassDependency (
			    0, 1,/*srcSubpass, dstSubpass*/
			    vk::PipelineStageFlags() | vk::PipelineStageFlagBits::eLateFragmentTests | vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader,/*srcStageMask, dstStageMask*/
			    vk::AccessFlags() | vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::AccessFlags() | vk::AccessFlagBits::eInputAttachmentRead,/*srcAccessMask, dstAccessMask*/
			    vk::DependencyFlags() /*dependencyFlags*/
			)
		};

		vk::RenderPassCreateInfo renderPassInfo ( vk::RenderPassCreateFlags(),
		        attachments.size(), attachments.data(),
		        subpasses.size(), subpasses.data(),
		        dependencies.size(), dependencies.data() /*dependencies*/ );

		v_renderpass = v_instance->vk_device ().createRenderPass ( renderPassInfo, nullptr );
		
		v_bundlestates[0].actual_image->v_create_use(&subpass_inputs[1].images_used[0]);
		v_bundlestates[1].actual_image->v_create_use(&subpass_inputs[1].images_used[1]);
		v_bundlestates[2].actual_image->v_create_use(&subpass_inputs[1].images_used[2]);
		v_bundlestates[4].actual_image->v_create_use(&subpass_inputs[1].images_used[3]);
		
		std::array<vk::DescriptorImageInfo, 4> dsimageinfo1 = {
			vk::DescriptorImageInfo ( vk::Sampler(), subpass_inputs[1].images_used[0].imageview, vk::ImageLayout::eShaderReadOnlyOptimal ),
			vk::DescriptorImageInfo ( vk::Sampler(), subpass_inputs[1].images_used[1].imageview, vk::ImageLayout::eShaderReadOnlyOptimal ),
			vk::DescriptorImageInfo ( vk::Sampler(), subpass_inputs[1].images_used[2].imageview, vk::ImageLayout::eShaderReadOnlyOptimal ),
			vk::DescriptorImageInfo ( vk::Sampler(), subpass_inputs[1].images_used[3].imageview, vk::ImageLayout::eGeneral )
		};
		/*std::array<vk::DescriptorImageInfo, 5> dsimageinfo2 = {
			vk::DescriptorImageInfo(vk::Sampler(), v_bundlestates[0].actual_image->instance_imageview(), vk::ImageLayout::eShaderReadOnlyOptimal),
			vk::DescriptorImageInfo(vk::Sampler(), v_bundlestates[1].actual_image->instance_imageview(), vk::ImageLayout::eShaderReadOnlyOptimal),
			vk::DescriptorImageInfo(vk::Sampler(), v_bundlestates[2].actual_image->instance_imageview(), vk::ImageLayout::eShaderReadOnlyOptimal),
			vk::DescriptorImageInfo(vk::Sampler(), v_bundlestates[3].actual_image->instance_imageview(), vk::ImageLayout::eShaderReadOnlyOptimal),
			vk::DescriptorImageInfo(vk::Sampler(), v_bundlestates[4].actual_image->instance_depth_imageview(), vk::ImageLayout::eGeneral)
		};*/
		std::array<vk::WriteDescriptorSet, 1> writeDescriptorSets = {
			vk::WriteDescriptorSet (
			    subpass_inputs[1].ds_set,
			    0, 0, dsimageinfo1.size(),
			    vk::DescriptorType::eInputAttachment,
			    dsimageinfo1.data(),
			    nullptr, nullptr
			)
		};
		v_instance->vk_device().updateDescriptorSets ( writeDescriptorSets, {} );
	}
	gen_tex_pipeline ( v_instance, &tex_pipeline, viewport, v_renderpass );
	gen_flat_pipeline ( v_instance, &flat_pipeline, viewport, v_renderpass );
	gen_skybox_pipeline ( v_instance, &skybox_pipeline, viewport, v_renderpass );
	gen_shot_pipeline ( v_instance, &shot_pipeline, viewport, v_renderpass );
	gen_engine_pipeline ( v_instance, &engine_pipeline, viewport, v_renderpass );
	gen_lightless_pipeline ( v_instance, &lightless_pipeline, viewport, v_renderpass );
	gen_dirlight_pipeline ( v_instance, &dirlight_pipeline, viewport, v_renderpass );
	last_frame_index_pipeline_built = v_instance->frame_index;
}
void VMainRenderStage::v_dispatch ( vk::CommandBuffer buffer, u32 index ) {
	v_check_rebuild();
	v_rebuild_pipelines();

	PerFrameMainBundleRenderObj& data = v_per_frame_data[index];
	if ( !data.framebuffer ) {
		vk::ImageView attachments[] = {
			v_bundlestates[0].actual_image->v_create_use(ImagePart::eColor, {0, 1}, {0, 1}).imageview,
			v_bundlestates[1].actual_image->v_create_use(ImagePart::eColor, {0, 1}, {0, 1}).imageview,
			v_bundlestates[2].actual_image->v_create_use(ImagePart::eColor, {0, 1}, {0, 1}).imageview,
			v_bundlestates[3].actual_image->v_create_use(ImagePart::eColor, {0, 1}, {0, 1}).imageview,
			v_bundlestates[4].actual_image->v_create_use(ImagePart::eDepthStencil, {0, 1}, {0, 1}).imageview
		};
		vk::FramebufferCreateInfo frameBufferCreateInfo = {
			vk::FramebufferCreateFlags(), v_renderpass,
			5, attachments,
			( u32 ) viewport.extend.width, ( u32 ) viewport.extend.height, 1
		};
		data.framebuffer = v_instance->vk_device ().createFramebuffer ( frameBufferCreateInfo );
	}
	//update the instancedata
	void* instance_data = v_igroup->buffer_storeage.allocate_transfer_buffer();
	for ( auto it = v_igroup->instance_to_data_map.begin(); it != v_igroup->instance_to_data_map.end(); it++ ) {
		const InstanceBase* instancebase = v_instance->instancebase ( it->first );
		for ( InstanceBlock& block : it->second ) {
			if ( block.data ) memcpy ( instance_data + block.offset, block.data, instancebase->instance_datagroup.size * block.count );
		}
	}
	v_igroup->buffer_storeage.transfer_data ( buffer );

	//for each context in the contextgroup update them
	VUpdateableBufferStorage* bufferstorage = v_instance->context_bufferstorage;
	bufferstorage->fetch_transferbuffers();
	for ( auto it = v_instance->v_context_map.begin(); it != v_instance->v_context_map.end(); it++ ) {
		for ( VContext* v_context : it->second ) {
			for ( VImageUse& vimageuse : v_context->images ) {
				if ( vimageuse.id ) vimageuse.image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, buffer );
			}
			if ( v_context->data ) {
				VThinBuffer staging = bufferstorage->get_buffer_pair ( v_context->buffer_chunk.index ).second;
				memcpy ( staging.mapped_ptr + v_context->buffer_chunk.offset, v_context->data, v_context->buffer_chunk.size );
			}
		}
	}
	for ( std::pair<VBuffer*, VThinBuffer> p : bufferstorage->buffers ) {
		vk::BufferCopy buffercopy ( 0, 0, p.first->size );
		buffer.copyBuffer ( p.second.buffer, p.first->buffer, 1, &buffercopy );
	}
	bufferstorage->free_transferbuffers ( v_instance->frame_index );

	//barrier for access
	vk::BufferMemoryBarrier bufferMemoryBarrier[] = {
		vk::BufferMemoryBarrier ( vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eVertexAttributeRead,
		                          v_instance->queue_wrapper()->graphics_queue_id, v_instance->queue_wrapper()->graphics_queue_id,
		                          v_igroup->buffer_storeage.buffer.buffer, 0, v_igroup->buffer_storeage.buffer.size )
	};
	buffer.pipelineBarrier (
	    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eVertexInput,
	    vk::DependencyFlags(),
	    0, nullptr,//memoryBarrier
	    1, bufferMemoryBarrier,//bufferMemoryBarrier
	    0, nullptr//imageMemoryBarrier
	);

	vk::ClearValue clearColors[] = {
		vk::ClearValue ( vk::ClearColorValue ( std::array<float, 4> ( {0.0f, 0.0f, 0.0f, 0.0f} ) ) ),
		vk::ClearValue ( vk::ClearColorValue ( std::array<float, 4> ( {0.0f, 0.0f, 0.0f, 0.0f} ) ) ),
		vk::ClearValue ( vk::ClearColorValue ( std::array<float, 4> ( {0.0f, 0.0f, 0.0f, 0.0f} ) ) ),
		vk::ClearValue ( vk::ClearColorValue ( std::array<float, 4> ( {0.0f, 0.0f, 0.0f, 0.0f} ) ) ),
		vk::ClearValue ( vk::ClearDepthStencilValue ( 1.0f, 0 ) )
	};
	vk::RenderPassBeginInfo renderPassBeginInfo = {
		v_renderpass, data.framebuffer,
		vk::Rect2D ( vk::Offset2D ( viewport.offset.x, viewport.offset.y ), vk::Extent2D ( viewport.extend.x, viewport.extend.y ) ),
		5, clearColors
	};
	v_bundlestates[0].actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, buffer, index );
	v_bundlestates[1].actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, buffer, index );
	v_bundlestates[2].actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, buffer, index );
	v_bundlestates[3].actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, buffer, index );
	v_bundlestates[4].actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, buffer, index );

	buffer.beginRenderPass ( renderPassBeginInfo, vk::SubpassContents::eInline );

	render_pipeline ( v_instance, v_igroup, v_cgroup, &skybox_pipeline, &subpass_inputs[0], buffer );
	render_pipeline ( v_instance, v_igroup, v_cgroup, &tex_pipeline, &subpass_inputs[0], buffer );
	render_pipeline ( v_instance, v_igroup, v_cgroup, &flat_pipeline, &subpass_inputs[0], buffer );
	render_pipeline ( v_instance, v_igroup, v_cgroup, &shot_pipeline, &subpass_inputs[0], buffer );
	render_pipeline ( v_instance, v_igroup, v_cgroup, &engine_pipeline, &subpass_inputs[0], buffer );

	buffer.nextSubpass ( vk::SubpassContents::eInline );

	render_pipeline ( v_instance, v_igroup, v_cgroup, &lightless_pipeline, &subpass_inputs[1], buffer );
	render_pipeline ( v_instance, v_igroup, v_cgroup, &dirlight_pipeline, &subpass_inputs[1], buffer );

	buffer.endRenderPass();
}



VScaleDownRenderStage::VScaleDownRenderStage ( VInstance* v_instance ) : VRenderStage(RenderStageType::eDownSample), v_instance(v_instance) {
	v_bundlestates.resize ( 1 );
}
VScaleDownRenderStage::~VScaleDownRenderStage() {
	
}
void VScaleDownRenderStage::set_rendertarget ( u32 index, Image* image ) {
	assert ( index < v_bundlestates.size );
	VBundleImageState& imagestate = v_bundlestates[index];
	imagestate.actual_image = static_cast<VBaseImage*> ( image );
	if ( imagestate.actual_image->v_format != imagestate.current_format ) {
		imagestate.current_format = imagestate.actual_image->v_format;
	}
}
void VScaleDownRenderStage::set_renderwindow ( u32 index, Window* window ) {
	assert(false);
}
void VScaleDownRenderStage::v_dispatch ( vk::CommandBuffer buffer, u32 index ) {
	VBundleImageState& imagestate = v_bundlestates[0];
	if(imagestate.actual_image->mipmap_layers > 1) {
		imagestate.actual_image->generate_mipmaps(vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, buffer);
	}
}
VCopyToScreenRenderStage::VCopyToScreenRenderStage ( VInstance* v_instance ) : VRenderStage(RenderStageType::eCopyToScreen), v_instance(v_instance) {
	v_bundlestates.resize ( 1 );
}
VCopyToScreenRenderStage::~VCopyToScreenRenderStage() {
	
}
void VCopyToScreenRenderStage::set_rendertarget ( u32 index, Image* image ) {
	assert ( index < v_bundlestates.size );
	VBundleImageState& imagestate = v_bundlestates[index];
	imagestate.actual_image = static_cast<VBaseImage*> ( image );
	if ( imagestate.actual_image->v_format != imagestate.current_format ) {
		imagestate.current_format = imagestate.actual_image->v_format;
	}
}
void VCopyToScreenRenderStage::set_renderwindow ( u32 index, Window* window ) {
	v_window = static_cast<VWindow*> (window);
}
void VCopyToScreenRenderStage::v_dispatch ( vk::CommandBuffer buffer, u32 index ) {
	
	VBundleImageState& imagestate = v_bundlestates[0];
	VBaseImage* window_image = v_window->present_images[v_window->present_image_index];
	vk::Extent3D window_extent = window_image->extent, 
		image_extent = imagestate.actual_image->extent;
	
	imagestate.actual_image->transition_layout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal, buffer);
	window_image->transition_layout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, buffer);
	vk::Filter filter = vk::Filter::eNearest;
	if(window_extent == image_extent) { // if it is the same nearest is enough
		filter = vk::Filter::eNearest;
	} else {
		filter = vk::Filter::eLinear;
	}
	vk::ImageBlit imageBlit ( vk::ImageSubresourceLayers ( vk::ImageAspectFlagBits::eColor, 0, 0, 1 ), {
		vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( image_extent.width, image_extent.height, image_extent.depth )
	}, vk::ImageSubresourceLayers ( vk::ImageAspectFlagBits::eColor, 0, 0, 1 ), {
		vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( window_extent.width, window_extent.height, window_extent.depth )
	} );
	buffer.blitImage(
		imagestate.actual_image->image, vk::ImageLayout::eTransferSrcOptimal, 
		window_image->image, vk::ImageLayout::eTransferDstOptimal,
		1, &imageBlit, filter
	);
	window_image->transition_layout ( vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR, buffer);
}
VMainBundle::VMainBundle ( VInstance* instance, InstanceGroup* igroup, ContextGroup* cgroup ) :
	stages ( 3 ),
	dependencies(),
	window_dependency ( nullptr ),
	v_instance ( instance ),
	commandpool(),
	v_per_frame_data ( MAX_PRESENTIMAGE_COUNT ),
	last_used ( 0 ) {

	if ( !commandpool ) {
		vk::CommandPoolCreateInfo createInfo ( vk::CommandPoolCreateFlagBits::eResetCommandBuffer, v_instance->queues.graphics_queue_id );
		v_instance->vk_device ().createCommandPool ( &createInfo, nullptr, &commandpool );
	}
	stages[0] = new VMainRenderStage ( instance, igroup, cgroup );
	stages[1] = new VScaleDownRenderStage ( instance );
	stages[2] = new VCopyToScreenRenderStage ( instance );
}
VMainBundle::~VMainBundle() {
	v_instance->vk_device ().destroyCommandPool ( commandpool );
	commandpool = vk::CommandPool();
	for ( VRenderStage* renderstage : stages ) {
		delete renderstage;
	}
}

void VMainBundle::add_dependency ( u32 src_index, u32 dst_index ) {
	dependencies.push_back ( std::make_pair ( src_index, dst_index ) );
}
void VMainBundle::remove_dependency ( u32 src_index, u32 dst_index ) {
	for ( auto it = dependencies.begin(); it != dependencies.end(); it++ ) {
		if ( it->first == src_index && it->second == dst_index ) it = dependencies.erase ( it );
	}
}
void VMainBundle::set_window_dependency ( Window* window ) {
	window_dependency = static_cast<VWindow*> ( window );
}
void VMainBundle::clear_window_dependency( ) {
	window_dependency = nullptr;
}

void VMainBundle::set_renderstage ( u32 index, RenderStage* renderstage ) {
	//TODO
}
RenderStage* VMainBundle::get_renderstage ( u32 index ) {
	return stages[index];
}
RenderStage* VMainBundle::remove_renderstage ( u32 index ) {
	//TODO
	return nullptr;
}
void VMainBundle::v_dispatch ( ) {

	v_logger.log<LogLevel::eWarn> ( "--------------- FrameBoundary %d ---------------", v_instance->frame_index );

	u32 index = 0;
	if ( window_dependency ) {
		window_dependency->rendering_mutex.lock();
		window_dependency->prepare_frame();
		index = window_dependency->present_image_index;
	}

	QueueWrapper* queue_wrapper = &v_instance->queues;

	vk::PipelineStageFlags waitDstStageMask ( vk::PipelineStageFlagBits::eBottomOfPipe );

	PerFrameMainBundleRenderObj& data = v_per_frame_data[index];
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

		for ( VRenderStage* renderstage : stages ) {
			renderstage->v_dispatch ( data.command.buffer, index );
		}
		
		data.command.buffer.end();
		//data.command.should_reset = false;
	}

	vk::SubmitInfo submitinfos[1] = { vk::SubmitInfo (
	                                      1, &window_dependency->image_available_guard_sem, //waitSem
	                                      &waitDstStageMask,
	                                      1, &data.command.buffer,//commandbuffers
	                                      1, &window_dependency->current_framelocal_data()->present_ready_sem//signalSem
	                                  )
	                                };

	queue_wrapper->graphics_queue.submit ( 1, submitinfos, window_dependency->current_framelocal_data()->image_presented_fence );

	if ( !queue_wrapper->combined_graphics_present_queue ) {
		//@TODO synchronize
		assert ( false );
	}
	//present image
	vk::Result results;
	vk::PresentInfoKHR presentInfo (
	    1, &window_dependency->current_framelocal_data()->present_ready_sem,
	    //active_sems.size(), active_sems.data(),
	    1, &window_dependency->swap_chain,
	    &index, &results );
	queue_wrapper->present_queue.presentKHR ( &presentInfo );
	last_used = v_instance->frame_index;

	v_instance->frame_index++;
	window_dependency->rendering_mutex.unlock();
	v_logger.log<LogLevel::eWarn> ( "---------------     End Frame    ---------------" );
}
