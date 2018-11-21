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

/*
VPipelineObject::VPipelineObject ( InstanceBaseId mib_id, Array<ContextBaseId>& contextbase_list ) :
	mib_id(mib_id), contextbase_list(contextbase_list) {

}
VPipelineObject::~VPipelineObject ( ) {
	destroy_all();
}

void VPipelineObject::destroy_pipeline_layouts() {
	destroy_commandbuffers();
	destroy_pipelines();

	if ( v_pipeline_layout ) {
		v_instance->vk_device ().destroyPipelineLayout ( v_pipeline_layout );
		v_pipeline_layout = vk::PipelineLayout();
	}
}
void VPipelineObject::destroy_pipelines() {
	destroy_commandbuffers();
	if ( v_pipeline ) {
		v_instance->vk_device ().destroyPipeline ( v_pipeline );
		v_pipeline = vk::Pipeline();
	}
}
void VPipelineObject::destroy_commandbuffers() {

}
void VPipelineObject::destroy_all() {
	destroy_commandbuffers();
	destroy_pipelines();
	destroy_pipeline_layouts();
}
void VPipelineObject::rebuild ( vk::RenderPass renderpass, Array<vk::Framebuffer>& framebuffers, Viewport<f32> viewport ) {
	if(current_renderpass != renderpass || current_viewport != viewport){
		destroy_pipelines();
	}
	bool framebuffer_changed = false;
	if(framebuffers.size == per_instance_data.size) {
		for(u32 i = 0; i < framebuffers.size; i++) {
			if(per_instance_data[i].framebuffer != framebuffers[i]){
				framebuffer_changed = true;
				break;
			}
		}
		if(framebuffer_changed) destroy_commandbuffers();
	} else {
		destroy_commandbuffers();
	}
}
vk::CommandBuffer VPipelineObject::get_command_buffer ( u32 index ) {
	return commandbuffers[index];
}*/





VMainBundle::VMainBundle ( VInstance* instance, InstanceGroup* igroup, ContextGroup* cgroup ) :
	v_instance ( instance ),
	v_igroup ( static_cast<VInstanceGroup*> ( igroup ) ),
	v_cgroup ( static_cast<VContextGroup*> ( cgroup ) ),
	v_bundleStates ( 2 ),
	contextBaseId ( {
	camera_context_base_id, lightvector_base_id
} ),
model_contextBaseId ( {simplemodel_context_base_id} ),
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


	if ( v_object_pipeline_layout ) {
		v_instance->vk_device ().destroyPipelineLayout ( v_object_pipeline_layout );
		v_object_pipeline_layout = vk::PipelineLayout();
	}
}
void VMainBundle::v_destroy_pipelines() {
	v_destroy_framebuffers();
	if ( v_object_pipeline ) {
		v_instance->vk_device ().destroyPipeline ( v_object_pipeline );
		v_object_pipeline = vk::Pipeline();
	}
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
			printf ( "One or more Images not set for MainBundle\n" );
			return;
		}
		if ( last_frame_index_pipeline_built < imagestate.actual_image->created_frame_index ) {
			printf ( "Last Frame Index Pipeline Built %" PRId64 "\n", last_frame_index_pipeline_built );
			printf ( "Last Image built index %" PRId64 "\n", imagestate.actual_image->created_frame_index );
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
	if ( !v_object_pipeline_layout ) {
		printf ( "Rebuild Pipeline Layouts\n" );

		DynArray<vk::DescriptorSetLayout> v_descriptor_set_layouts;
		for ( ContextBaseId id : contextBaseId ) {
			VContextBase& v_contextbase = v_instance->contextbase_map[id];
			v_descriptor_set_layouts.push_back ( v_contextbase.descriptorset_layout );
		}
		for ( ContextBaseId id : model_contextBaseId ) {
			VContextBase& v_contextbase = v_instance->contextbase_map[id];
			v_descriptor_set_layouts.push_back ( v_contextbase.descriptorset_layout );
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
		v_object_pipeline_layout = v_instance->vk_device ().createPipelineLayout ( createInfo, nullptr );
	}
	if ( !v_renderpass ) {
		printf ( "Rebuild Renderpasses\n" );
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
	if ( !v_object_pipeline ) {
		printf ( "Rebuild Pipelines\n" );

		const InstanceBase* instancebase = v_instance->instancebase ( simplemodel_instance_base_id );
		const ModelBase* modelbase = v_instance->modelbase ( simplemodel_base_id );

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
					printf ( "Value: %s %d, %d, %d\n", to_string ( formatdata.format ).c_str(), bindingindex, 0, offset );
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
					printf ( "Value: %s %d, %d, %d\n", to_string ( formatdata.format ).c_str(), bindingindex, 1, offset );
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
		        vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise,
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

		VShaderModule* vmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "vert_shader" ) );
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader ( StringReference ( "frag_shader" ) );

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
		    v_object_pipeline_layout,
		    v_renderpass,
		    0,
		    vk::Pipeline(),
		    -1
		);

		v_object_pipeline = v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), {pipelineInfo}, nullptr ) [0];

	}
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

		DynArray<InstanceBlock>& instanceblocks = v_igroup->instance_to_data_map[simplemodel_instance_base_id];
		IdPtrArray<VModel>& models = v_instance->v_model_map[simplemodel_instance_base_id];

		{
			void* instance_data = v_igroup->buffer_storeage.allocate_transfer_buffer();
			for ( auto it = v_igroup->instance_to_data_map.begin(); it != v_igroup->instance_to_data_map.end(); it++ ) {
				const InstanceBase* instancebase = v_instance->instancebase ( it->first );
				for ( InstanceBlock& block : it->second ) {
					if ( block.data ) memcpy ( instance_data + block.offset, block.data, instancebase->instance_datagroup.size * block.count );
				}
			}
			v_igroup->buffer_storeage.transfer_data ( data.command.buffer );

			for ( auto it = v_cgroup->context_map.begin(); it != v_cgroup->context_map.end(); it++ ) {
				VContext* context = it->second;
				for(VBaseImage* v_image : context->images) {
					if(v_image) v_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, data.command.buffer );
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
			}
			for ( InstanceBlock& instanceblock : instanceblocks ) {
				VModel* v_model = models[instanceblock.model_id];
				for(VContext* context : v_model->v_contexts) {
					if(!context) continue;
					for(VBaseImage* v_image : context->images) {
						if(v_image) v_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, data.command.buffer );
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
				}
			}

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

		data.command.buffer.bindPipeline ( vk::PipelineBindPoint::eGraphics, v_object_pipeline );


		DynArray<vk::DescriptorSet> descriptorSets;
		for ( ContextBaseId id : contextBaseId ) {
			VContext* context_ptr = v_cgroup->context_map[id];
			assert ( context_ptr );
			descriptorSets.push_back ( context_ptr->descriptor_set );
		}

		u32 offset = 0;
		data.command.buffer.bindDescriptorSets ( vk::PipelineBindPoint::eGraphics, v_object_pipeline_layout, offset, descriptorSets.size(), descriptorSets.data(), 0, nullptr );
		offset += descriptorSets.size();

		for ( InstanceBlock& instanceblock : instanceblocks ) {
			printf ( "Instance: 0x%x ModelBase: 0x%x Model-Index: 0x%x Offset: 0x%x Count: %d\n", instanceblock.base_id, instanceblock.modelbase_id, instanceblock.model_id, instanceblock.offset, instanceblock.count );
			VModel* v_model = models[instanceblock.model_id];
			const ModelBase* modelbase_ptr = v_instance->modelbase ( v_model->modelbase_id );

			DynArray<vk::DescriptorSet> model_descriptorSets;
			for ( ContextBaseId id : model_contextBaseId ) {
				VContext* context_ptr = nullptr;
				for ( u32 i = 0; i < modelbase_ptr->contextbase_ids.size; i++ ) {
					if ( modelbase_ptr->contextbase_ids[i] == id ) {
						assert ( v_model->v_contexts[i] );
						context_ptr = v_model->v_contexts[i];
						break;
					}
				}
				assert ( context_ptr );
				model_descriptorSets.push_back ( context_ptr->descriptor_set );
			}
			data.command.buffer.bindDescriptorSets ( vk::PipelineBindPoint::eGraphics, v_object_pipeline_layout, offset, model_descriptorSets.size(), model_descriptorSets.data(), 0, nullptr );

			printf ( "Vertices: %d\n", v_model->indexcount );
			data.command.buffer.bindIndexBuffer ( v_model->indexbuffer.buffer, 0, v_model->index_is_2byte ? vk::IndexType::eUint16 : vk::IndexType::eUint32 );
			data.command.buffer.bindVertexBuffers ( 0, {v_model->vertexbuffer.buffer, v_igroup->buffer_storeage.buffer.buffer}, {0, instanceblock.offset} );
			data.command.buffer.drawIndexed ( v_model->indexcount, instanceblock.count, 0, 0, 0 );
		}
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
