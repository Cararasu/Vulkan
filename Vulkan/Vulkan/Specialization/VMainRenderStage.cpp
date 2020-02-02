#include "VMainRenderStage.h"
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

const u32 LIGHTING_MASK = 0x1;
const u32 SHADOW_MASK = 0x6;

void gen_tex_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index ) {
	if ( !p_struct->pipelines[pipeline_index] ) {
		v_logger.log<LogLevel::Trace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex ),
			vk::VertexInputBindingDescription ( 1, instancebase->instance_datagroup.size, vk::VertexInputRate::eInstance )
		};

		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		vertexInputAttributes.resize ( valuecount );
		{
			u32 index = 0;
			u32 bindingindex = 0;
			for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 0, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
			for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 1, offset );
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
			LIGHTING_MASK/*compareMask*/, LIGHTING_MASK/*writeMask*/, 1/*reference*/
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
		vk::GraphicsPipelineCreateInfo create_info[1] = {pipelineInfo};
		V_CHECKCALL(v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), 1, create_info, nullptr, &p_struct->pipelines[pipeline_index] ), 
			printf("Could not create graphicspipeline"));
	}
}

void gen_flat_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index ) {
	if ( !p_struct->pipelines[pipeline_index] ) {
		v_logger.log<LogLevel::Trace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex ),
			vk::VertexInputBindingDescription ( 1, instancebase->instance_datagroup.size, vk::VertexInputRate::eInstance )
		};

		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		vertexInputAttributes.resize ( valuecount );
		{
			u32 index = 0;
			u32 bindingindex = 0;
			for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 0, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
			for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 1, offset );
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
			LIGHTING_MASK/*compareMask*/, LIGHTING_MASK/*writeMask*/, 1/*reference*/
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
		vk::GraphicsPipelineCreateInfo create_info[1] = {pipelineInfo};
		V_CHECKCALL(v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), 1, create_info, nullptr, &p_struct->pipelines[pipeline_index] ), 
			printf("Could not create graphicspipeline"));
	}
}
void gen_skybox_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index ) {
	if ( !p_struct->pipelines[pipeline_index] ) {
		v_logger.log<LogLevel::Trace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 1> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex )
		};

		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		vertexInputAttributes.resize ( valuecount );
		{
			u32 index = 0;
			u32 bindingindex = 0;
			for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 0, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
			for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 1, offset );
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
		        VK_TRUE, VK_FALSE, //depthClampEnable, rasterizerDiscardEnable
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
		vk::GraphicsPipelineCreateInfo create_info[1] = {pipelineInfo};
		V_CHECKCALL(v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), 1, create_info, nullptr, &p_struct->pipelines[pipeline_index] ), 
			printf("Could not create graphicspipeline"));
	}
}
void gen_locallight_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index ) {
	if ( !p_struct->pipelines[pipeline_index] ) {
		v_logger.log<LogLevel::Trace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex ),
			vk::VertexInputBindingDescription ( 1, instancebase->instance_datagroup.size, vk::VertexInputRate::eInstance )
		};

		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		vertexInputAttributes.resize ( valuecount );
		{
			u32 index = 0;
			u32 bindingindex = 0;
			for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 0, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
			for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 1, offset );
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
		        vk::PolygonMode::eFill, vk::CullModeFlagBits::eFront, vk::FrontFace::eCounterClockwise,
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
				LIGHTING_MASK/*compareMask*/, LIGHTING_MASK/*writeMask*/, 1/*reference*/
			}, {
				vk::StencilOp::eKeep/*failOp*/, vk::StencilOp::eKeep/*passOp*/, vk::StencilOp::eKeep/*depthFailOp*/,
				vk::CompareOp::eEqual/*compareOp*/,
				LIGHTING_MASK/*compareMask*/, LIGHTING_MASK/*writeMask*/, 1/*reference*/
			}, //front, back
			0.0f, 1.0f //minDepthBounds, maxDepthBounds
		);

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_TRUE, //blendEnable
			    vk::BlendFactor::eOne, vk::BlendFactor::eOne, //srcColorBlendFactor, dstColorBlendFactor
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

		VShaderModule* vmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "vert_locallight_shader" ) );
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "frag_locallight_shader" ) );

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
		    1,
		    vk::Pipeline(),
		    -1
		);
		vk::GraphicsPipelineCreateInfo create_info[1] = {pipelineInfo};
		V_CHECKCALL(v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), 1, create_info, nullptr, &p_struct->pipelines[pipeline_index] ), 
			printf("Could not create graphicspipeline"));
	}
}
void gen_shot_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index ) {
	if ( !p_struct->pipelines[pipeline_index] ) {
		v_logger.log<LogLevel::Trace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex ),
			vk::VertexInputBindingDescription ( 1, instancebase->instance_datagroup.size, vk::VertexInputRate::eInstance )
		};

		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		vertexInputAttributes.resize ( valuecount );
		{
			u32 index = 0;
			u32 bindingindex = 0;
			for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 0, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
			for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 1, offset );
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
		    VK_TRUE, VK_FALSE, //depthTestEnable, depthWriteEnable
		    vk::CompareOp::eLess, //depthCompareOp
			VK_FALSE, VK_FALSE, {}, {}, //front, back
			0.0f, 1.0f //minDepthBounds, maxDepthBounds
		);

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_TRUE, //blendEnable
			    vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOne, //eOneMinusSrcAlpha, //srcColorBlendFactor, dstColorBlendFactor
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
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "frag_shot_shader" ) );

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
		    1,
		    vk::Pipeline(),
		    -1
		);
		vk::GraphicsPipelineCreateInfo create_info[1] = {pipelineInfo};
		V_CHECKCALL(v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), 1, create_info, nullptr, &p_struct->pipelines[pipeline_index] ), 
			printf("Could not create graphicspipeline"));
	}
}

void gen_billboard_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index ) {
	if ( !p_struct->pipelines[pipeline_index] ) {
		v_logger.log<LogLevel::Trace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex ),
			vk::VertexInputBindingDescription ( 1, instancebase->instance_datagroup.size, vk::VertexInputRate::eInstance )
		};

		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		vertexInputAttributes.resize ( valuecount );
		{
			u32 index = 0;
			u32 bindingindex = 0;
			for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 0, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
			for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 1, offset );
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
		    VK_TRUE, VK_FALSE, //depthTestEnable, depthWriteEnable
		    vk::CompareOp::eLess, //depthCompareOp
			VK_FALSE, VK_FALSE, {}, {}, //front, back
			0.0f, 1.0f //minDepthBounds, maxDepthBounds
		);

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[] = {
			vk::PipelineColorBlendAttachmentState (
			    VK_TRUE, //blendEnable
			    vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOne, //eOneMinusSrcAlpha, //srcColorBlendFactor, dstColorBlendFactor
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

		VShaderModule* vmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "vert_billboard_shader" ) );
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "frag_billboard_shader" ) );

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
		    1,
		    vk::Pipeline(),
		    -1
		);
		vk::GraphicsPipelineCreateInfo create_info[1] = {pipelineInfo};
		V_CHECKCALL(v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), 1, create_info, nullptr, &p_struct->pipelines[pipeline_index] ), 
			printf("Could not create graphicspipeline"));
	}
}

void gen_lightless_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index ) {
	if ( !p_struct->pipelines[pipeline_index] ) {
		v_logger.log<LogLevel::Trace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 1> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex )
		};

		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		vertexInputAttributes.resize ( valuecount );
		{
			u32 index = 0;
			u32 bindingindex = 0;
			for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 0, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
			for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 1, offset );
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
		        vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise,
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
			LIGHTING_MASK/*compareMask*/, LIGHTING_MASK/*writeMask*/, 0/*reference*/
		}, { //depthBoundsTestEnable, stencilTestEnable
			vk::StencilOp::eKeep/*failOp*/, vk::StencilOp::eKeep/*passOp*/, vk::StencilOp::eKeep/*depthFailOp*/,
			vk::CompareOp::eEqual/*compareOp*/,
			LIGHTING_MASK/*compareMask*/, LIGHTING_MASK/*writeMask*/, 0/*reference*/
		}, //front, back
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
		vk::GraphicsPipelineCreateInfo create_info[1] = {pipelineInfo};
		V_CHECKCALL(v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), 1, create_info, nullptr, &p_struct->pipelines[pipeline_index] ), 
			printf("Could not create graphicspipeline"));
	}
}
void gen_dirlight_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index ) {
	if ( !p_struct->pipelines[pipeline_index] ) {
		v_logger.log<LogLevel::Trace> ( "Rebuild Pipelines" );

		const ModelBase* modelbase = v_instance->modelbase ( p_struct->modelbase_id );
		const InstanceBase* instancebase = v_instance->instancebase ( p_struct->instancebase_id );

		std::array<vk::VertexInputBindingDescription, 1> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, modelbase->datagroup.size, vk::VertexInputRate::eVertex )
		};

		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
			valuecount += transform_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		vertexInputAttributes.resize ( valuecount );
		{
			u32 index = 0;
			u32 bindingindex = 0;
			for ( DataValueDef& valuedef : modelbase->datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
					vertexInputAttributes[index] = vk::VertexInputAttributeDescription ( bindingindex, 0, formatdata.format, offset/* + value*/ );
					offset += formatdata.bytesize;
					bindingindex += ( ( formatdata.bytesize - 1 ) / 16 ) + 1;
					index++;
				}
			}
			for ( DataValueDef& valuedef : instancebase->instance_datagroup.valuedefs ) {
				VFormatData formatdata = transform_v_format ( valuedef.type );
				u32 count = formatdata.count * valuedef.arraycount;
				u32 offset = valuedef.offset;
				for ( u32 i = 0; i < count; i++ ) {
					v_logger.log<LogLevel::Trace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 1, offset );
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
		        vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise,
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
				LIGHTING_MASK/*compareMask*/, LIGHTING_MASK/*writeMask*/, 1/*reference*/
			}, { //depthBoundsTestEnable, stencilTestEnable
				vk::StencilOp::eKeep/*failOp*/, vk::StencilOp::eKeep/*passOp*/, vk::StencilOp::eKeep/*depthFailOp*/,
				vk::CompareOp::eEqual/*compareOp*/,
				LIGHTING_MASK/*compareMask*/, LIGHTING_MASK/*writeMask*/, 1/*reference*/
			}, //front, back
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
		vk::GraphicsPipelineCreateInfo create_info[1] = {pipelineInfo};
		V_CHECKCALL(v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), 1, create_info, nullptr, &p_struct->pipelines[pipeline_index] ), 
			printf("Could not create GraphicsPipeline\n"));
	}
}


VMainRenderStage::VMainRenderStage ( VInstance* instance ) :
	VRenderStage ( RenderStageType::eRendering ),
	v_instance ( instance ),
	tex_pipeline ( simple_modelbase_id, textured_instance_base_id, { camera_context_base_id }, {tex_simplemodel_context_base_id} ),
	flat_pipeline ( simple_modelbase_id, flat_instance_base_id, { camera_context_base_id }, {flat_simplemodel_context_base_id} ),
	skybox_pipeline ( simple_modelbase_id, skybox_instance_base_id, {}, {skybox_context_base_id} ),
	dirlight_pipeline ( fullscreen_modelbase_id, dirlight_instance_base_id, {camera_context_base_id, lightvector_base_id, shadowmap_context_base_id}, {} ),
	lightless_pipeline ( fullscreen_modelbase_id, single_instance_base_id, {}, {} ),
	locallight_pipeline ( simple_modelbase_id, shot_instance_base_id, {camera_context_base_id}, {} ),
	shot_pipeline ( simple_modelbase_id, shot_instance_base_id, {camera_context_base_id}, {} ),
	billboard_pipeline ( simple_modelbase_id, billboard_instance_base_id, {camera_context_base_id}, {explosion_context_base_id} ),
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
	V_CHECKCALL(v_instance->vk_device().createDescriptorPool ( &poolInfo, nullptr, &input_ds_pool ), printf("Cannot create DescriptorPool\n"));

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
		vk::DescriptorSetLayoutCreateInfo create_info( vk::DescriptorSetLayoutCreateFlags(), 4, binding );
		V_CHECKCALL(v_instance->vk_device ().createDescriptorSetLayout ( &create_info, nullptr, &subpass_inputs[1].ds_layout ),
			printf("Cannot create DescriptorSet\n"));
		
	V_CHECKCALL(v_instance->vk_device().createDescriptorPool ( &poolInfo, nullptr, &input_ds_pool ), printf("Cannot create DescriptorPool"));

		vk::DescriptorSetAllocateInfo allocInfo ( input_ds_pool, 1, &subpass_inputs[1].ds_layout );
		v_instance->vk_device().allocateDescriptorSets ( &allocInfo, &subpass_inputs[1].ds_set );

		subpass_inputs[1].images_used.resize ( 4, {} );

	}
}

VMainRenderStage::~VMainRenderStage() {
	v_instance->vk_device ().destroyCommandPool ( commandpool, nullptr );
	commandpool = vk::CommandPool();

	v_instance->vk_device().destroyDescriptorPool ( input_ds_pool, nullptr );

	for ( SubPassInput& subpass_input : subpass_inputs ) {
		v_instance->vk_device().destroyDescriptorSetLayout ( subpass_input.ds_layout, nullptr );
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
	
	destroy_pipeline_layout ( v_instance, &lightless_pipeline );
	destroy_pipeline_layout ( v_instance, &dirlight_pipeline );
	destroy_pipeline_layout ( v_instance, &locallight_pipeline );
	destroy_pipeline_layout ( v_instance, &shot_pipeline );
	destroy_pipeline_layout ( v_instance, &billboard_pipeline );
}
void VMainRenderStage::v_destroy_pipelines() {
	v_destroy_framebuffers();
	destroy_pipeline ( v_instance, &tex_pipeline );
	destroy_pipeline ( v_instance, &flat_pipeline );
	destroy_pipeline ( v_instance, &skybox_pipeline );
	
	destroy_pipeline ( v_instance, &lightless_pipeline );
	destroy_pipeline ( v_instance, &dirlight_pipeline );
	destroy_pipeline ( v_instance, &locallight_pipeline );
	destroy_pipeline ( v_instance, &shot_pipeline );
	destroy_pipeline ( v_instance, &billboard_pipeline );
	
	for ( PerFrameRenderObj& data : v_per_frame_data ) {
		data.command.should_reset = true;
	}
}
void VMainRenderStage::v_destroy_renderpasses() {
	if ( v_renderpass ) {
		v_instance->vk_device ().destroyRenderPass ( v_renderpass, nullptr );
		v_renderpass = vk::RenderPass();
	}
}
void VMainRenderStage::v_destroy_framebuffers() {
	for ( PerFrameRenderObj& data : v_per_frame_data ) {
		if ( data.framebuffer ) {
			v_instance->vk_device ().destroyFramebuffer ( data.framebuffer, nullptr );
			data.framebuffer = vk::Framebuffer();
		}
	}
}

void VMainRenderStage::set_renderimage ( u32 index, Image* image, u32 miplayer, u32 arraylayer ) {
	assert ( index < v_bundlestates.size );
	VBundleImageState& imagestate = v_bundlestates[index];
	if ( imagestate.use.imageview ) {
		imagestate.actual_image->v_deregister_use ( imagestate.use.id );
		imagestate.use.imageview = vk::ImageView();
	}
	imagestate.actual_image = static_cast<VBaseImage*> ( image );
	imagestate.miplayer = miplayer;
	imagestate.arraylayer = arraylayer;
	if ( imagestate.actual_image->v_format != imagestate.current_format ) {
		imagestate.current_format = imagestate.actual_image->v_format;
		v_destroy_renderpasses();
	}
	v_destroy_framebuffers();
}
void VMainRenderStage::set_renderwindow ( u32 index, Window* window ) {
	//TODO implement
	assert ( false );
}
void VMainRenderStage::v_check_rebuild() {
	u32 width = 0, height = 0;
	for ( VBundleImageState& imagestate : v_bundlestates ) {
		if ( !imagestate.actual_image ) {
			v_logger.log<LogLevel::Warn>  ( "One or more Images not set for MainBundle" );
			continue;
		}
		if ( last_frame_index_pipeline_built < imagestate.actual_image->created_frame_index ) {
			v_logger.log<LogLevel::Trace> ( "Last Frame Index Pipeline Built %" PRId64 "", last_frame_index_pipeline_built );
			v_logger.log<LogLevel::Trace> ( "Last Image built index %" PRId64 "", imagestate.actual_image->created_frame_index );
			v_destroy_pipelines();
		}

		u32 thewidth = imagestate.actual_image->extent.width / ( 1 << imagestate.miplayer );
		u32 theheight = imagestate.actual_image->extent.height / ( 1 << imagestate.miplayer );

		assert ( !width || width == thewidth );
		assert ( !height || height == theheight );
		width = thewidth;
		height = theheight;

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

	gen_pipeline_layout ( v_instance, &subpass_inputs[1], &lightless_pipeline );
	gen_pipeline_layout ( v_instance, &subpass_inputs[1], &dirlight_pipeline );
	gen_pipeline_layout ( v_instance, &subpass_inputs[1], &locallight_pipeline );
	gen_pipeline_layout ( v_instance, &subpass_inputs[1], &shot_pipeline );
	gen_pipeline_layout ( v_instance, &subpass_inputs[1], &billboard_pipeline );

	if ( !v_renderpass ) {
		v_logger.log<LogLevel::Debug> ( "Rebuild Renderpasses" );
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
			                            vk::AttachmentLoadOp::eClear,//stencilLoadOp
			                            vk::AttachmentStoreOp::eStore,//stencilStoreOp
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

		V_CHECKCALL(v_instance->vk_device ().createRenderPass ( &renderPassInfo, nullptr, &v_renderpass ), printf("Cannot create RenderPass\n"));


		if ( !subpass_inputs[1].images_used[0] ) subpass_inputs[1].images_used[0] = v_bundlestates[0].actual_image->v_create_use ( vk::ImageAspectFlagBits::eColor, {0, 1}, {0, 1} );
		if ( !subpass_inputs[1].images_used[1] ) subpass_inputs[1].images_used[1] = v_bundlestates[1].actual_image->v_create_use ( vk::ImageAspectFlagBits::eColor, {0, 1}, {0, 1} );
		if ( !subpass_inputs[1].images_used[2] ) subpass_inputs[1].images_used[2] = v_bundlestates[2].actual_image->v_create_use ( vk::ImageAspectFlagBits::eColor, {0, 1}, {0, 1} );
		if ( !subpass_inputs[1].images_used[3] ) subpass_inputs[1].images_used[3] = v_bundlestates[4].actual_image->v_create_use ( vk::ImageAspectFlagBits::eDepth, {0, 1}, {0, 1} );
		std::array<vk::DescriptorImageInfo, 4> dsimageinfo1 = {
			vk::DescriptorImageInfo ( vk::Sampler(), subpass_inputs[1].images_used[0].imageview(), vk::ImageLayout::eShaderReadOnlyOptimal ),
			vk::DescriptorImageInfo ( vk::Sampler(), subpass_inputs[1].images_used[1].imageview(), vk::ImageLayout::eShaderReadOnlyOptimal ),
			vk::DescriptorImageInfo ( vk::Sampler(), subpass_inputs[1].images_used[2].imageview(), vk::ImageLayout::eShaderReadOnlyOptimal ),
			vk::DescriptorImageInfo ( vk::Sampler(), subpass_inputs[1].images_used[3].imageview(), vk::ImageLayout::eGeneral )
		};
		std::array<vk::WriteDescriptorSet, 1> writeDescriptorSets = {
			vk::WriteDescriptorSet (
			    subpass_inputs[1].ds_set,
			    0, 0, dsimageinfo1.size(),
			    vk::DescriptorType::eInputAttachment,
			    dsimageinfo1.data(),
			    nullptr, nullptr
			)
		};
		v_instance->vk_device().updateDescriptorSets ( 1, writeDescriptorSets.data(), 0, nullptr );

	} else if ( subpass_inputs[1].images_used[0].is_updated() || subpass_inputs[1].images_used[1].is_updated() || subpass_inputs[1].images_used[2].is_updated() || subpass_inputs[1].images_used[3].is_updated() ) {
		subpass_inputs[1].images_used[0].set_updated();
		subpass_inputs[1].images_used[1].set_updated();
		subpass_inputs[1].images_used[2].set_updated();
		subpass_inputs[1].images_used[3].set_updated();
		std::array<vk::DescriptorImageInfo, 4> dsimageinfo1 = {
			vk::DescriptorImageInfo ( vk::Sampler(), subpass_inputs[1].images_used[0].imageview(), vk::ImageLayout::eShaderReadOnlyOptimal ),
			vk::DescriptorImageInfo ( vk::Sampler(), subpass_inputs[1].images_used[1].imageview(), vk::ImageLayout::eShaderReadOnlyOptimal ),
			vk::DescriptorImageInfo ( vk::Sampler(), subpass_inputs[1].images_used[2].imageview(), vk::ImageLayout::eShaderReadOnlyOptimal ),
			vk::DescriptorImageInfo ( vk::Sampler(), subpass_inputs[1].images_used[3].imageview(), vk::ImageLayout::eGeneral )
		};
		std::array<vk::WriteDescriptorSet, 1> writeDescriptorSets = {
			vk::WriteDescriptorSet (
			    subpass_inputs[1].ds_set,
			    0, 0, dsimageinfo1.size(),
			    vk::DescriptorType::eInputAttachment,
			    dsimageinfo1.data(),
			    nullptr, nullptr
			)
		};
		v_instance->vk_device().updateDescriptorSets ( writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr );
	}


	gen_tex_pipeline ( v_instance, &tex_pipeline, viewport, v_renderpass );
	gen_flat_pipeline ( v_instance, &flat_pipeline, viewport, v_renderpass );
	gen_skybox_pipeline ( v_instance, &skybox_pipeline, viewport, v_renderpass );
	
	gen_lightless_pipeline ( v_instance, &lightless_pipeline, viewport, v_renderpass );
	gen_dirlight_pipeline ( v_instance, &dirlight_pipeline, viewport, v_renderpass );
	gen_locallight_pipeline ( v_instance, &locallight_pipeline, viewport, v_renderpass );
	gen_shot_pipeline ( v_instance, &shot_pipeline, viewport, v_renderpass );
	gen_billboard_pipeline ( v_instance, &billboard_pipeline, viewport, v_renderpass );
	last_frame_index_pipeline_built = v_instance->frame_index;
}
void VMainRenderStage::v_dispatch ( vk::CommandBuffer buffer, u32 index ) {
	v_check_rebuild();
	v_rebuild_pipelines();

	PerFrameRenderObj& data = v_per_frame_data[index];
	if ( !data.framebuffer ) {
		for ( int i = 0; i < v_bundlestates.size; i++ ) {
			if ( !data.images[i] ) {
				data.images[i] = v_bundlestates[i].actual_image->v_create_use (
				                     v_bundlestates[i].actual_image->aspect,
				{v_bundlestates[i].miplayer, v_bundlestates[i].miplayer + 1},
				{v_bundlestates[i].arraylayer, v_bundlestates[i].arraylayer + 1} );
			}
		}

		vk::ImageView attachments[] = {
			data.images[0].imageview(),
			data.images[1].imageview(),
			data.images[2].imageview(),
			data.images[3].imageview(),
			data.images[4].imageview()
		};
		vk::FramebufferCreateInfo frameBufferCreateInfo = {
			vk::FramebufferCreateFlags(), v_renderpass,
			5, attachments,
			( u32 ) viewport.extend.width, ( u32 ) viewport.extend.height, 1
		};
		V_CHECKCALL(v_instance->vk_device ().createFramebuffer ( &frameBufferCreateInfo, nullptr,  &data.framebuffer ), printf("Cannot create FrameBuffer\n"));
	}

	update_instancegroup ( v_instance, v_instancegroup, buffer );
	update_contexts ( v_instance, v_contextgroup, buffer );

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
	{
		vk::PipelineStageFlags sourceStage, destinationStage;
			
		vk::ImageMemoryBarrier barriers[5] = {
			v_bundlestates[0].actual_image->transition_layout_impl ( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 
							{v_bundlestates[0].miplayer, v_bundlestates[0].miplayer + 1}, {v_bundlestates[0].arraylayer, v_bundlestates[0].arraylayer + 1}, &sourceStage, &destinationStage ),
			v_bundlestates[1].actual_image->transition_layout_impl ( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 
							{v_bundlestates[1].miplayer, v_bundlestates[1].miplayer + 1}, {v_bundlestates[1].arraylayer, v_bundlestates[1].arraylayer + 1}, &sourceStage, &destinationStage ),
			v_bundlestates[2].actual_image->transition_layout_impl ( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 
							{v_bundlestates[2].miplayer, v_bundlestates[2].miplayer + 1}, {v_bundlestates[2].arraylayer, v_bundlestates[2].arraylayer + 1}, &sourceStage, &destinationStage ),
			v_bundlestates[3].actual_image->transition_layout_impl ( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, 
							{v_bundlestates[3].miplayer, v_bundlestates[3].miplayer + 1}, {v_bundlestates[3].arraylayer, v_bundlestates[3].arraylayer + 1}, &sourceStage, &destinationStage ),
			v_bundlestates[4].actual_image->transition_layout_impl ( vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, 
							{v_bundlestates[4].miplayer, v_bundlestates[4].miplayer + 1}, {v_bundlestates[4].arraylayer, v_bundlestates[4].arraylayer + 1}, &sourceStage, &destinationStage )
		};
		buffer.pipelineBarrier (
			sourceStage, destinationStage,
			vk::DependencyFlags(),
			0, nullptr,//memoryBarriers
			0, nullptr,//bufferBarriers
			5, barriers//imageBarriers
		);
	}

	buffer.beginRenderPass ( &renderPassBeginInfo, vk::SubpassContents::eInline );

	render_pipeline ( v_instance, v_instancegroup, v_contextgroup, &skybox_pipeline, &subpass_inputs[0], buffer );
	render_pipeline ( v_instance, v_instancegroup, v_contextgroup, &tex_pipeline, &subpass_inputs[0], buffer );
	render_pipeline ( v_instance, v_instancegroup, v_contextgroup, &flat_pipeline, &subpass_inputs[0], buffer );

	buffer.nextSubpass ( vk::SubpassContents::eInline );

	render_pipeline ( v_instance, v_instancegroup, v_contextgroup, &lightless_pipeline, &subpass_inputs[1], buffer );
	render_pipeline ( v_instance, v_instancegroup, v_contextgroup, &dirlight_pipeline, &subpass_inputs[1], buffer );
	render_pipeline ( v_instance, v_instancegroup, v_contextgroup, &locallight_pipeline, &subpass_inputs[1], buffer );
	render_pipeline ( v_instance, v_instancegroup, v_contextgroup, &shot_pipeline, &subpass_inputs[1], buffer );
	render_pipeline ( v_instance, v_instancegroup, v_contextgroup, &billboard_pipeline, &subpass_inputs[1], buffer );
	//render_pipeline ( v_instance, v_instancegroup, v_contextgroup, &engine_pipeline, &subpass_inputs[1], buffer );

	buffer.endRenderPass();
}
