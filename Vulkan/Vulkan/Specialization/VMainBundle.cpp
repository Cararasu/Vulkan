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

VMainBundle::VMainBundle ( VInstance* instance, InstanceGroup* igroup, ContextGroup* cgroup ) :
	v_instance ( instance ),
	v_igroup ( static_cast<VInstanceGroup*> ( igroup ) ),
	v_cgroup ( static_cast<VContextGroup*> ( cgroup ) ),
	v_bundleStates ( 2 ),
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
	for ( u32 i = 0; i < v_descriptor_set_layouts.size; i++ ) {
		if ( v_descriptor_set_layouts[i] ) {
			v_instance->vk_device().destroyDescriptorSetLayout ( v_descriptor_set_layouts[i] );
			v_descriptor_set_layouts[i] = vk::DescriptorSetLayout();
		}
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
		vk::DescriptorSetLayoutBinding bindings1[] = {
			vk::DescriptorSetLayoutBinding ( 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr ),
		};
		vk::DescriptorSetLayoutBinding bindings2[] = {
			vk::DescriptorSetLayoutBinding ( 0, vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr ),
			vk::DescriptorSetLayoutBinding ( 1, vk::DescriptorType::eSampledImage, 3, vk::ShaderStageFlagBits::eFragment, nullptr )
		};

		v_descriptor_set_layouts = {
			v_instance->vk_device ().createDescriptorSetLayout ( vk::DescriptorSetLayoutCreateInfo ( vk::DescriptorSetLayoutCreateFlags(), 1, bindings1 ), nullptr ),
			//v_instance->vk_device ().createDescriptorSetLayout ( vk::DescriptorSetLayoutCreateInfo ( vk::DescriptorSetLayoutCreateFlags(), 2, bindings2 ), nullptr )
		};

		std::array<vk::PushConstantRange, 0> pushConstRanges = {};//{vk::PushConstantRange ( vk::ShaderStageFlagBits::eFragment, 0, sizeof ( ObjectPartData ) ) };

		vk::PipelineLayoutCreateInfo createInfo (
		    vk::PipelineLayoutCreateFlags(),
		    0, nullptr,//setLayouts
		    0, nullptr//pushConstRanges
		);
		if ( v_descriptor_set_layouts.size != 0 ) {
			createInfo.setLayoutCount = v_descriptor_set_layouts.size;
			createInfo.pSetLayouts = v_descriptor_set_layouts.data;
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
			                            vk::AttachmentLoadOp::eLoad,//loadOp
			                            vk::AttachmentStoreOp::eStore,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eColorAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eColorAttachmentOptimal//finalLayout
			                          ),
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            v_bundleStates[1].current_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eLoad,//loadOp
			                            vk::AttachmentStoreOp::eDontCare,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eDepthStencilAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eDepthStencilAttachmentOptimal//finalLayout
			                          )
		};
		MAX_PRESENTIMAGE_COUNT;

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

		const ModelInstanceBase* modelinstancebase = v_instance->modelinstancebase ( simplemodel_base_id );

		const ModelBase* modelbase = v_instance->modelbase ( modelinstancebase->modelbase_id );

		const DataGroupDef* vertex_datagroup = v_instance->datagroupdef ( modelbase->datagroup_id );
		const DataGroupDef* instance_datagroup = v_instance->datagroupdef ( modelinstancebase->instance_datagroup_id );

		std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, vertex_datagroup->size, vk::VertexInputRate::eVertex ),
			vk::VertexInputBindingDescription ( 1, instance_datagroup->size, vk::VertexInputRate::eInstance )
		};

		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for ( DataValueDef& valuedef : vertex_datagroup->valuedefs ) {
			valuecount += to_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		for ( DataValueDef& valuedef : instance_datagroup->valuedefs ) {
			valuecount += to_v_format ( valuedef.type ).count * valuedef.arraycount;
		}
		vertexInputAttributes.resize ( valuecount );
		{
			u32 index = 0;
			u32 bindingindex = 0;
			for ( DataValueDef& valuedef : vertex_datagroup->valuedefs ) {
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
			for ( DataValueDef& valuedef : instance_datagroup->valuedefs ) {
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
		    {}, {}, //fron, back
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
	v_rebuild_commandbuffers();
}
void VMainBundle::v_rebuild_commandbuffers() {
	for ( u32 i = 0; i < v_per_frame_data.size; i++ ) {
		PerFrameMainBundleRenderObj& data = v_per_frame_data[i];
		if ( !data.framebuffer ) {
			vk::ImageView attachments[2] = {v_bundleStates[0].actual_image->instance_imageview(i), v_bundleStates[1].actual_image->instance_imageview(i)};
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
		}
		vk::CommandBufferBeginInfo begininfo = {
			vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr
		};
		data.command.buffer.begin ( begininfo );

		vk::ClearValue clearColors[2] = {
			vk::ClearValue ( vk::ClearColorValue ( std::array<float, 4> ( {1.0f, 0.0f, 0.0f, 0.0f} ) ) ),
			vk::ClearValue ( vk::ClearDepthStencilValue ( 1.0f, 0 ) )
		};
		vk::RenderPassBeginInfo renderPassBeginInfo = {
			v_renderpass, data.framebuffer,
			vk::Rect2D ( vk::Offset2D ( viewport.offset.x, viewport.offset.y ), vk::Extent2D ( viewport.extend.x, viewport.extend.y ) ),
			2, clearColors
		};
		v_bundleStates[0].actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, data.command.buffer );
		v_bundleStates[1].actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, data.command.buffer );
		
		data.command.buffer.beginRenderPass ( renderPassBeginInfo, vk::SubpassContents::eInline );

		data.command.buffer.bindPipeline ( vk::PipelineBindPoint::eGraphics, this->v_object_pipeline );
		//data.command.buffer.bindVertexBuffers ( 0, {vertex_buffer->buffer, vertex_buffer->buffer}, {0, sizeof ( QuadVertex ) * 6} );
		//data.command.buffer.draw ( 6, 2, 0, 0 );

		/*VkViewport viewport = vks::initializers::view^port((float)width, (float)height, 0.0f, 1.0f);
		vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
		vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);*/

		//1. simple textured quads
		//2. simple filled quads
		//3. fonts
		
		data.command.buffer.endRenderPass();
		if(v_bundleStates[0].actual_image->window_target){
			v_bundleStates[0].actual_image->transition_layout ( vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, data.command.buffer );
			v_bundleStates[1].actual_image->transition_layout ( vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eGeneral, data.command.buffer );
		}
		data.command.buffer.end();
	}
}
void VMainBundle::v_dispatch() {
	v_check_rebuild();
	v_rebuild_pipelines();

	QueueWrapper* queue_wrapper = &v_instance->queues;
	
/*
	vk::PipelineStageFlags waitDstStageMask ( vk::PipelineStageFlagBits::eBottomOfPipe );

	SubmitStore submit_store;

	SubmitInfo si;
	si.wait_dst_stage_mask = vk::PipelineStageFlags ( vk::PipelineStageFlagBits::eTransfer );
	si.need_sem_index = submit_store.semaphores.size();
	si.need_sem_count = 1;
	submit_store.semaphores.push_back ( image_available_guard_sem );
	si.comm_buff_index = submit_store.commandbuffers.size();
	si.comm_buff_count = 1;
	submit_store.commandbuffers.push_back ( data->clear_command_buffer );
	si.sig_sem_index = submit_store.semaphores.size();
	si.sig_sem_count = 1;
	submit_store.semaphores.push_back ( data->render_ready_sem );
	submit_store.submitinfos.push_back ( si );

	u32 sem_index = 1;
	quad_renderer->render ( present_image_index, &submit_store, 1, &sem_index );

	si.wait_dst_stage_mask = vk::PipelineStageFlags ( vk::PipelineStageFlagBits::eColorAttachmentOutput );
	si.need_sem_index = sem_index;
	si.need_sem_count = 1;
	si.comm_buff_index = submit_store.commandbuffers.size();
	si.comm_buff_count = 1;
	submit_store.commandbuffers.push_back ( data->present_command_buffer );
	si.sig_sem_index = submit_store.semaphores.size();
	si.sig_sem_count = 1;
	submit_store.semaphores.push_back ( data->present_ready_sem );
	submit_store.submitinfos.push_back ( si );

	submit_store.signal_fence = data->image_presented_fence;

	DynArray<vk::SubmitInfo> submitinfos;
	for ( SubmitInfo& submit_info : submit_store.submitinfos ) {
		submitinfos.push_back (
		    vk::SubmitInfo (
		        submit_info.need_sem_count, submit_store.semaphores.data() + submit_info.need_sem_index, //waitSem
		        &submit_info.wait_dst_stage_mask,
		        submit_info.comm_buff_count, submit_store.commandbuffers.data() + submit_info.comm_buff_index,//commandbuffers
		        submit_info.sig_sem_count, submit_store.semaphores.data() + submit_info.sig_sem_index//signalSem
		    )
		);
	}
	queue_wrapper->graphics_queue.submit ( submitinfos, submit_store.signal_fence );

	if ( !queue_wrapper->combined_graphics_present_queue ) {
		//@TODO synchronize
		assert(false);
	}
	//present image
	vk::Result results;
	vk::PresentInfoKHR presentInfo (
	    1, &data->present_ready_sem,
	    //active_sems.size(), active_sems.data(),
	    1, swapChains,
	    &present_image_index, &results );
	queue_wrapper->present_queue.presentKHR ( &presentInfo );
	active_sems.clear();
*/
	last_used = v_instance->frame_index;
}
