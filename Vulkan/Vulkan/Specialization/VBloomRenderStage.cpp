#include "VBloomRenderStage.h"
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

void gen_hbloom_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass ) {
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
					v_logger.log<LogLevel::eTrace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
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
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "hbloom_shader" ) );

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
		    0,/*subpass*/
		    vk::Pipeline(),
		    -1
		);
		p_struct->pipeline = v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), {pipelineInfo}, nullptr ) [0];
	}
}
void gen_vbloom_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass ) {
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
					v_logger.log<LogLevel::eTrace> ( "Value: %s %d, %d, %d", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
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
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "vbloom_shader" ) );

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
		    0,/*subpass*/
		    vk::Pipeline(),
		    -1
		);
		p_struct->pipeline = v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), {pipelineInfo}, nullptr ) [0];
	}
}

VBloomRenderStage::VBloomRenderStage ( VInstance* instance, InstanceGroup* igroup, ContextGroup* cgroup ) :
	VRenderStage ( RenderStageType::eRendering ),
	v_instance ( instance ),
	v_igroup ( static_cast<VInstanceGroup*> ( igroup ) ),
	v_cgroup ( static_cast<VContextGroup*> ( cgroup ) ),
	bloom_pipeline ( fullscreen_modelbase_id, single_instance_base_id, {}, {} ),
                v_per_frame_data ( MAX_PRESENTIMAGE_COUNT ),
subpass_inputs(1) {

	v_bundlestates.resize ( 1 );
}

VBloomRenderStage::~VBloomRenderStage() {

	v_destroy_framebuffers();
	v_destroy_pipeline_layouts();
}
void VBloomRenderStage::v_destroy_pipeline_layouts() {
	v_destroy_renderpasses();
	v_destroy_pipelines();

	destroy_pipeline_layout ( v_instance, &bloom_pipeline );
}
void VBloomRenderStage::v_destroy_pipelines() {
	v_destroy_framebuffers();
	destroy_pipeline ( v_instance, &bloom_pipeline );
}
void VBloomRenderStage::v_destroy_renderpasses() {
	if ( v_renderpass ) {
		v_instance->vk_device ().destroyRenderPass ( v_renderpass );
		v_renderpass = vk::RenderPass();
	}
}
void VBloomRenderStage::v_destroy_framebuffers() {
	for ( PerFrameRenderObj& data : v_per_frame_data ) {
		if ( data.framebuffer ) {
			v_instance->vk_device ().destroyFramebuffer ( data.framebuffer );
			data.framebuffer = vk::Framebuffer();
		}
	}
}

void VBloomRenderStage::set_renderimage ( u32 index, Image* image, Range<u32> miprange, Range<u32> layers ) {
	assert ( index < v_bundlestates.size );
	VBundleImageState& imagestate = v_bundlestates[index];
	if ( imagestate.use.imageview ) {
		imagestate.actual_image->v_deregister_use ( imagestate.use.id );
		imagestate.use.imageview = vk::ImageView();
	}
	imagestate.actual_image = static_cast<VBaseImage*> ( image );
	imagestate.miprange = miprange;
	imagestate.layers = layers;
	if ( imagestate.actual_image->v_format != imagestate.current_format ) {
		imagestate.current_format = imagestate.actual_image->v_format;
		v_destroy_renderpasses();
	}
	v_destroy_framebuffers();
}
void VBloomRenderStage::set_renderwindow ( u32 index, Window* window ) {
	//TODO implement
	assert ( false );
}
void VBloomRenderStage::v_check_rebuild() {
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

void VBloomRenderStage::v_rebuild_pipelines() {
	gen_pipeline_layout ( v_instance, &subpass_inputs[0], &bloom_pipeline );

	if ( !v_renderpass ) {
		v_logger.log<LogLevel::eDebug> ( "Rebuild Renderpasses" );
		std::array<vk::AttachmentDescription, 1> attachments = {
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            v_bundlestates[0].current_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eDontCare,//loadOp
			                            vk::AttachmentStoreOp::eStore,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eColorAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eColorAttachmentOptimal//finalLayout
			                          )
		};

		vk::AttachmentReference colorAttachmentRefs1[] = {
			vk::AttachmentReference ( 0, vk::ImageLayout::eColorAttachmentOptimal )
		};

		std::array<vk::SubpassDescription, 1> subpasses = {
			vk::SubpassDescription (
			    vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics,
			    0, nullptr/*inputAttachments*/,
			    1, colorAttachmentRefs1/*colorAttachments*/,
			    nullptr,/*resolveAttachments*/
			    nullptr,/*depthAttackment*/
			    0, nullptr/*preserveAttachments*/
			)
		};

		vk::RenderPassCreateInfo renderPassInfo ( vk::RenderPassCreateFlags(),
		        attachments.size(), attachments.data(),
		        subpasses.size(), subpasses.data(),
		        0, nullptr /*dependencies*/ );

		v_renderpass = v_instance->vk_device ().createRenderPass ( renderPassInfo, nullptr );

	}
	gen_vbloom_pipeline ( v_instance, &bloom_pipeline, viewport, v_renderpass );
	last_frame_index_pipeline_built = v_instance->frame_index;
}
void VBloomRenderStage::v_dispatch ( vk::CommandBuffer buffer, u32 index ) {
	v_check_rebuild();
	v_rebuild_pipelines();

	PerFrameRenderObj& data = v_per_frame_data[index];
	if ( !data.framebuffer ) {
		for(int i = 0; i < v_bundlestates.size; i++) {
			if(!data.images[i]) {
				data.images[i] = v_bundlestates[i].actual_image->v_create_use(v_bundlestates[i].actual_image->aspect, {v_bundlestates[i].miprange.min, 1}, {v_bundlestates[i].layers.min, 1});
			}
		}
		
		vk::ImageView attachments[] = {
			data.images[0].imageview()
		};
		vk::FramebufferCreateInfo frameBufferCreateInfo = {
			vk::FramebufferCreateFlags(), v_renderpass,
			1, attachments,
			( u32 ) viewport.extend.width, ( u32 ) viewport.extend.height, 1
		};
		data.framebuffer = v_instance->vk_device ().createFramebuffer ( frameBufferCreateInfo );
	}
	
	update_instancegroup(v_instance, v_igroup, buffer);
	update_contexts(v_instance, v_cgroup, buffer);

	vk::ClearValue clearColors[] = {
		vk::ClearValue ( vk::ClearColorValue ( std::array<float, 4> ( {0.0f, 0.0f, 0.0f, 0.0f} ) ) )
	};
	vk::RenderPassBeginInfo renderPassBeginInfo = {
		v_renderpass, data.framebuffer,
		vk::Rect2D ( vk::Offset2D ( viewport.offset.x, viewport.offset.y ), vk::Extent2D ( viewport.extend.x, viewport.extend.y ) ),
		1, clearColors
	};
	v_bundlestates[0].actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, buffer, index );

	buffer.beginRenderPass ( renderPassBeginInfo, vk::SubpassContents::eInline );
	render_pipeline ( v_instance, v_igroup, v_cgroup, &bloom_pipeline, &subpass_inputs[0], buffer );
	buffer.endRenderPass();
}

//--------------------


HBloomRenderStage::HBloomRenderStage ( VInstance* instance, InstanceGroup* igroup, ContextGroup* cgroup ) :
	VRenderStage ( RenderStageType::eRendering ),
	v_instance ( instance ),
	v_igroup ( static_cast<VInstanceGroup*> ( igroup ) ),
	v_cgroup ( static_cast<VContextGroup*> ( cgroup ) ),
	bloom_pipeline ( fullscreen_modelbase_id, single_instance_base_id, {}, {} ),
                v_per_frame_data ( MAX_PRESENTIMAGE_COUNT ),
subpass_inputs(1) {

	v_bundlestates.resize ( 1 );
}

HBloomRenderStage::~HBloomRenderStage() {

	v_destroy_framebuffers();
	v_destroy_pipeline_layouts();
}
void HBloomRenderStage::v_destroy_pipeline_layouts() {
	v_destroy_renderpasses();
	v_destroy_pipelines();

	destroy_pipeline_layout ( v_instance, &bloom_pipeline );
}
void HBloomRenderStage::v_destroy_pipelines() {
	v_destroy_framebuffers();
	destroy_pipeline ( v_instance, &bloom_pipeline );
}
void HBloomRenderStage::v_destroy_renderpasses() {
	if ( v_renderpass ) {
		v_instance->vk_device ().destroyRenderPass ( v_renderpass );
		v_renderpass = vk::RenderPass();
	}
}
void HBloomRenderStage::v_destroy_framebuffers() {
	for ( PerFrameRenderObj& data : v_per_frame_data ) {
		if ( data.framebuffer ) {
			v_instance->vk_device ().destroyFramebuffer ( data.framebuffer );
			data.framebuffer = vk::Framebuffer();
		}
	}
}

void HBloomRenderStage::set_renderimage ( u32 index, Image* image, Range<u32> miprange, Range<u32> layers ) {
	assert ( index < v_bundlestates.size );
	VBundleImageState& imagestate = v_bundlestates[index];
	if ( imagestate.use.imageview ) {
		imagestate.actual_image->v_deregister_use ( imagestate.use.id );
		imagestate.use.imageview = vk::ImageView();
	}
	imagestate.actual_image = static_cast<VBaseImage*> ( image );
	imagestate.miprange = miprange;
	imagestate.layers = layers;
	if ( imagestate.actual_image->v_format != imagestate.current_format ) {
		imagestate.current_format = imagestate.actual_image->v_format;
		v_destroy_renderpasses();
	}
	v_destroy_framebuffers();
}
void HBloomRenderStage::set_renderwindow ( u32 index, Window* window ) {
	//TODO implement
	assert ( false );
}
void HBloomRenderStage::v_check_rebuild() {
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

void HBloomRenderStage::v_rebuild_pipelines() {
	gen_pipeline_layout ( v_instance, &subpass_inputs[0], &bloom_pipeline );

	if ( !v_renderpass ) {
		v_logger.log<LogLevel::eDebug> ( "Rebuild Renderpasses" );
		std::array<vk::AttachmentDescription, 1> attachments = {
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            v_bundlestates[0].current_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eDontCare,//loadOp
			                            vk::AttachmentStoreOp::eStore,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eColorAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eColorAttachmentOptimal//finalLayout
			                          )
		};

		vk::AttachmentReference colorAttachmentRefs1[] = {
			vk::AttachmentReference ( 0, vk::ImageLayout::eColorAttachmentOptimal )
		};

		std::array<vk::SubpassDescription, 1> subpasses = {
			vk::SubpassDescription (
			    vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics,
			    0, nullptr/*inputAttachments*/,
			    1, colorAttachmentRefs1/*colorAttachments*/,
			    nullptr,/*resolveAttachments*/
			    nullptr,/*depthAttackment*/
			    0, nullptr/*preserveAttachments*/
			)
		};

		vk::RenderPassCreateInfo renderPassInfo ( vk::RenderPassCreateFlags(),
		        attachments.size(), attachments.data(),
		        subpasses.size(), subpasses.data(),
		        0, nullptr /*dependencies*/ );

		v_renderpass = v_instance->vk_device ().createRenderPass ( renderPassInfo, nullptr );

	}

	gen_hbloom_pipeline ( v_instance, &bloom_pipeline, viewport, v_renderpass );
	last_frame_index_pipeline_built = v_instance->frame_index;
}
void HBloomRenderStage::v_dispatch ( vk::CommandBuffer buffer, u32 index ) {
	v_check_rebuild();
	v_rebuild_pipelines();

	PerFrameRenderObj& data = v_per_frame_data[index];
	if ( !data.framebuffer ) {
		for(int i = 0; i < v_bundlestates.size; i++) {
			if(!data.images[i]) {
				data.images[i] = v_bundlestates[i].actual_image->v_create_use(v_bundlestates[i].actual_image->aspect, {v_bundlestates[i].miprange.min, 1}, {v_bundlestates[i].layers.min, 1});
			}
		}
		
		vk::ImageView attachments[] = {
			data.images[0].imageview()
		};
		vk::FramebufferCreateInfo frameBufferCreateInfo = {
			vk::FramebufferCreateFlags(), v_renderpass,
			1, attachments,
			( u32 ) viewport.extend.width, ( u32 ) viewport.extend.height, 1
		};
		data.framebuffer = v_instance->vk_device ().createFramebuffer ( frameBufferCreateInfo );
	}

	update_instancegroup(v_instance, v_igroup, buffer);
	update_contexts(v_instance, v_cgroup, buffer);

	vk::ClearValue clearColors[] = {
		vk::ClearValue ( vk::ClearColorValue ( std::array<float, 4> ( {0.0f, 0.0f, 0.0f, 0.0f} ) ) )
	};
	vk::RenderPassBeginInfo renderPassBeginInfo = {
		v_renderpass, data.framebuffer,
		vk::Rect2D ( vk::Offset2D ( viewport.offset.x, viewport.offset.y ), vk::Extent2D ( viewport.extend.x, viewport.extend.y ) ),
		1, clearColors
	};
	v_bundlestates[0].actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, buffer, index );

	buffer.beginRenderPass ( renderPassBeginInfo, vk::SubpassContents::eInline );
	render_pipeline ( v_instance, v_igroup, v_cgroup, &bloom_pipeline, &subpass_inputs[0], buffer );
	buffer.endRenderPass();
}



