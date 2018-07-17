#include "VulkanQuadRenderer.h"
#include "VulkanWindowSection.h"
#include "VulkanWindow.h"


VulkanQuadRenderer::VulkanQuadRenderer ( VulkanInstance* instance ) : v_instance ( instance ) {

}
VulkanQuadRenderer::~VulkanQuadRenderer() {

}
void VulkanQuadRenderer::destroy ( ) {
	if ( pipeline ) {
		vulkan_device ( v_instance ).destroyPipeline ( pipeline );
		pipeline = vk::Pipeline();
	}
	if ( g_commandpool ) {
		vulkan_device ( v_instance ).destroyCommandPool ( g_commandpool );
		g_commandpool = vk::CommandPool();
	}
	if ( t_commandpool ) {
		vulkan_device ( v_instance ).destroyCommandPool ( t_commandpool );
		t_commandpool = vk::CommandPool();
	}
	if ( renderpass ) {
		vulkan_device ( v_instance ).destroyRenderPass ( renderpass );
		renderpass = vk::RenderPass();
	}
	if ( vertex_shader ) {
		vulkan_device ( v_instance ).destroyShaderModule ( vertex_shader );
		vertex_shader = vk::ShaderModule();
	}
	if ( fragment_shader ) {
		vulkan_device ( v_instance ).destroyShaderModule ( fragment_shader );
		fragment_shader = vk::ShaderModule();
	}
	if ( pipeline_layout ) {
		vulkan_device ( v_instance ).destroyPipelineLayout ( pipeline_layout );
		pipeline_layout = vk::PipelineLayout();
	}
	destroy_framebuffers();
	if ( vertex_buffer ) {
		delete vertex_buffer;
		vertex_buffer = nullptr;
	}
	if ( staging_vertex_buffer ) {
		delete staging_vertex_buffer;
		staging_vertex_buffer = nullptr;
	}
	for ( auto dsl : descriptor_set_layouts ) {
		vulkan_device ( v_instance ).destroyDescriptorSetLayout ( dsl );
		dsl = vk::DescriptorSetLayout();
	}
}
void VulkanQuadRenderer::destroy_framebuffers() {
	for ( auto& fb : per_target_data ) {
		if ( fb.framebuffer ) {
			vulkan_device ( v_instance ).destroyFramebuffer ( fb.framebuffer );
			fb.framebuffer = vk::Framebuffer();
		}
	}
	per_target_data.clear();
}

RendResult VulkanQuadRenderer::update_extend ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) {
	this->viewport = viewport;
	//TODO move most of the things here to the resourcemanager
	//@Refactor
	if ( !pipeline_layout ) {
		vk::DescriptorSetLayoutBinding bindings1[] = {
			vk::DescriptorSetLayoutBinding ( 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr ),
		};
		vk::DescriptorSetLayoutBinding bindings2[] = {
			vk::DescriptorSetLayoutBinding ( 0, vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr ),
			vk::DescriptorSetLayoutBinding ( 1, vk::DescriptorType::eSampledImage, 3, vk::ShaderStageFlagBits::eFragment, nullptr )
		};

		descriptor_set_layouts = {
			//vulkan_device ( v_instance ).createDescriptorSetLayout ( vk::DescriptorSetLayoutCreateInfo ( vk::DescriptorSetLayoutCreateFlags(), 1, bindings1 ), nullptr ),
			//vulkan_device ( v_instance ).createDescriptorSetLayout ( vk::DescriptorSetLayoutCreateInfo ( vk::DescriptorSetLayoutCreateFlags(), 2, bindings2 ), nullptr )
		};

		std::array<vk::PushConstantRange, 0> pushConstRanges = {};//{vk::PushConstantRange ( vk::ShaderStageFlagBits::eFragment, 0, sizeof ( ObjectPartData ) ) };

		vk::PipelineLayoutCreateInfo createInfo (
		    vk::PipelineLayoutCreateFlags(),
		    0, nullptr,//setLayouts
		    0, nullptr//pushConstRanges
		);
		if ( descriptor_set_layouts.size() != 0 ) {
			createInfo.setLayoutCount = descriptor_set_layouts.size();
			createInfo.pSetLayouts = descriptor_set_layouts.data();
		}
		if ( pushConstRanges.size() != 0 ) {
			createInfo.pushConstantRangeCount = pushConstRanges.size();
			createInfo.pPushConstantRanges = pushConstRanges.data();
		}
		pipeline_layout = vulkan_device ( v_instance ).createPipelineLayout ( createInfo, nullptr );
	}


	if ( !vertex_shader ) {
		vertex_shader = v_instance->v_resource_manager->load_shader_from_file ( "shader/quad.vert.sprv" );
	}
	if ( !fragment_shader ) {
		fragment_shader = v_instance->v_resource_manager->load_shader_from_file ( "shader/quad.frag.sprv" );
	}
	if ( !renderpass ) {// Color and depth-stencil buffers
		vk::AttachmentDescription attachments[2] = {
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            target_wrapper->color_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eDontCare,//loadOp
			                            vk::AttachmentStoreOp::eStore,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eColorAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eColorAttachmentOptimal//finalLayout
			                          ),
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            target_wrapper->depth_stencil_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eDontCare,//loadOp
			                            vk::AttachmentStoreOp::eDontCare,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eDepthStencilAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eDepthStencilAttachmentOptimal//finalLayout
			                          )
		};

		vk::AttachmentReference colorAttachmentRef ( 0, vk::ImageLayout::eColorAttachmentOptimal );
		vk::AttachmentReference depthAttachmentRef ( 1, vk::ImageLayout::eDepthStencilAttachmentOptimal );

		vk::SubpassDescription subpass (
		    vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics,
		    0, nullptr/*inputAttackments*/,
		    1, &colorAttachmentRef/*colorAttachments*/,
		    nullptr,/*resolveAttachments*/
		    &depthAttachmentRef,/*depthAttackment*/
		    0, nullptr/*preserveAttachments*/
		);

		vk::RenderPassCreateInfo renderPassInfo ( vk::RenderPassCreateFlags(), 2, attachments, 1, &subpass, 0, nullptr/*dependencies*/ );

		renderpass = vulkan_device ( v_instance ).createRenderPass ( renderPassInfo, nullptr );
	}
	destroy_framebuffers();
	per_target_data.resize ( target_wrapper->images.size() );
	vk::ImageView attachments[2] = {vk::ImageView(), target_wrapper->depthview};
	vk::FramebufferCreateInfo frameBufferCreateInfo = {
		vk::FramebufferCreateFlags(), renderpass,
		2, attachments,
		viewport.extend.width, viewport.extend.height, 1
	};
	for ( size_t i = 0; i < target_wrapper->images.size(); i++ ) {
		attachments[0] = target_wrapper->images[i].imageview;
		per_target_data[i].framebuffer = vulkan_device ( v_instance ).createFramebuffer ( frameBufferCreateInfo );
	}


	if ( pipeline ) {
		vulkan_device ( v_instance ).destroyPipeline ( pipeline );
	}


	std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
		vk::VertexInputBindingDescription ( 0, sizeof ( QuadVertex ), vk::VertexInputRate::eVertex ),
		vk::VertexInputBindingDescription ( 1, sizeof ( QuadInstance ), vk::VertexInputRate::eInstance )
	};

	std::array<vk::VertexInputAttributeDescription, 5> vertexInputAttributes = {
		vk::VertexInputAttributeDescription ( 0, 0, vk::Format::eR32G32Sfloat, offsetof ( QuadVertex, pos ) ),

		vk::VertexInputAttributeDescription ( 1, 1, vk::Format::eR32G32B32A32Sfloat, offsetof ( QuadInstance, dim ) ),
		vk::VertexInputAttributeDescription ( 2, 1, vk::Format::eR32G32B32A32Sfloat, offsetof ( QuadInstance, uvdim ) ),
		vk::VertexInputAttributeDescription ( 3, 1, vk::Format::eR32G32Sfloat, offsetof ( QuadInstance, depth ) ),
		vk::VertexInputAttributeDescription ( 4, 1, vk::Format::eR32G32Sfloat, offsetof ( QuadInstance, color ) ),
	};

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo ( vk::PipelineVertexInputStateCreateFlags(),
	        vertexInputBindings.size(), vertexInputBindings.data(),
	        vertexInputAttributes.size(), vertexInputAttributes.data() );

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

	vk::PipelineShaderStageCreateInfo shaderStages[2] = {
		vk::PipelineShaderStageCreateInfo (
		    vk::PipelineShaderStageCreateFlags(),
		    vk::ShaderStageFlagBits::eVertex, vertex_shader,
		    "main", nullptr//name, specialization
		),
		vk::PipelineShaderStageCreateInfo (
		    vk::PipelineShaderStageCreateFlags(),
		    vk::ShaderStageFlagBits::eFragment, fragment_shader,
		    "main", nullptr//name, specialization
		),
	};
	vk::GraphicsPipelineCreateInfo pipelineInfo (
	    vk::PipelineCreateFlags(),
	    2, shaderStages,
	    &vertexInputInfo, &inputAssembly, nullptr, &viewportState, &rasterizer, &multisampling, &depthStencil, &colorBlending,
	    nullptr,
	    pipeline_layout,
	    renderpass,
	    0,
	    vk::Pipeline(),
	    -1
	);

	pipeline = vulkan_device ( v_instance ).createGraphicsPipelines ( vk::PipelineCache(), {pipelineInfo}, nullptr ) [0];

	printf ( "Update Quad-Renderer\n" );
}

void VulkanQuadRenderer::init() {

	const static u32 vertexbuffer_size = sizeof ( QuadVertex ) * 6 + sizeof ( QuadInstance ) * 100;
	if ( !vertex_buffer ) {
		vertex_buffer = new VulkanBuffer (
		    v_instance, vertexbuffer_size,
		    vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		    vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );
	}
	if ( !staging_vertex_buffer ) {
		staging_vertex_buffer = new VulkanBuffer (
		    v_instance, vertexbuffer_size,
		    vk::BufferUsageFlagBits::eTransferSrc,
		    vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
		staging_vertex_buffer->map_mem();
		memset ( staging_vertex_buffer->mapped_ptr, 0, vertexbuffer_size );
		QuadVertex* vertex_ptr = ( QuadVertex* ) staging_vertex_buffer->mapped_ptr;
		vertex_ptr[0].pos = glm::vec2 ( 1.0f, 1.0f );
		vertex_ptr[1].pos = glm::vec2 ( 0.0f, 1.0f );
		vertex_ptr[2].pos = glm::vec2 ( 0.0f, 0.0f );
		vertex_ptr[3].pos = glm::vec2 ( 0.0f, 0.0f );
		vertex_ptr[4].pos = glm::vec2 ( 1.0f, 0.0f );
		vertex_ptr[5].pos = glm::vec2 ( 1.0f, 1.0f );

		QuadInstance* instance_ptr = ( QuadInstance* ) ( staging_vertex_buffer->mapped_ptr + sizeof ( QuadVertex ) * 6 );
		instance_ptr[0].dim = glm::vec4 ( 0.1f, 0.1f, 0.3f, 0.3f );
		instance_ptr[0].uvdim = glm::vec4 ( 0.1f, 0.1f, 0.5f, 0.5f );
		instance_ptr[0].depth = glm::vec4 ( 0.5f, 0.0f, 0.0f, 0.0f);
		instance_ptr[0].color = glm::vec4 ( 0.2f, 1.0f, 1.0f, 0.0f );
		instance_ptr[1].dim = glm::vec4 ( 0.2f, 0.2f, 0.18f, 0.2f );
		instance_ptr[1].uvdim = glm::vec4 ( 0.1f, 0.1f, 0.5f, 0.5f );
		instance_ptr[1].depth = glm::vec4 ( 0.2f, 1.0f, 0.0f, 0.0f );
		instance_ptr[1].color = glm::vec4 ( 0.2f, 0.5f, 0.0f, 0.0f );
		staging_vertex_buffer->unmap_mem();
	}
	if ( !g_commandpool ) {
		vk::CommandPoolCreateInfo createInfo ( vk::CommandPoolCreateFlagBits::eResetCommandBuffer, v_instance->vulkan_pgc_queue ( 0 )->graphics_queue_id );
		vulkan_device ( v_instance ).createCommandPool ( &createInfo, nullptr, &g_commandpool );
	}
	if ( !t_commandpool && v_instance->queues.dedicated_transfer_queue ) {
		vk::CommandPoolCreateInfo createInfo ( vk::CommandPoolCreateFlagBits::eResetCommandBuffer, v_instance->queues.transfer_queue_id );
		vulkan_device ( v_instance ).createCommandPool ( &createInfo, nullptr, &t_commandpool );
	}
}
vk::CommandBuffer VulkanQuadRenderer::render_quads ( u32 index ) {
	
	init();
	
	PerFrameQuadRenderObj& per_frame = per_target_data[index];
	if ( per_frame.commandbuffer ) {
		per_frame.commandbuffer.reset ( vk::CommandBufferResetFlags() );
	} else {
		per_frame.commandbuffer = v_instance->createCommandBuffer ( g_commandpool, vk::CommandBufferLevel::ePrimary );
	}
	vk::CommandBufferBeginInfo begininfo = {
		vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr
	};
	per_frame.commandbuffer.begin ( begininfo );

	per_frame.commandbuffer.pipelineBarrier (
	    vk::PipelineStageFlags ( vk::PipelineStageFlagBits::eHost ), vk::PipelineStageFlags ( vk::PipelineStageFlagBits::eTransfer ),
	    vk::DependencyFlags(),
	{}/*memoryBarrier*/, {
		vk::BufferMemoryBarrier (
		    vk::AccessFlagBits::eHostWrite, vk::AccessFlagBits::eTransferRead,
		    v_instance->queues.pgc[0].graphics_queue_id, v_instance->queues.pgc[0].graphics_queue_id,
		    staging_vertex_buffer->buffer, 0, staging_vertex_buffer->size
		)
	}/*bufferBarrier*/, {}/*imageBarrier*/ );

	staging_vertex_buffer->transfer_to ( vertex_buffer, per_frame.commandbuffer );

	per_frame.commandbuffer.pipelineBarrier (
	    vk::PipelineStageFlags ( vk::PipelineStageFlagBits::eTransfer ), vk::PipelineStageFlags ( vk::PipelineStageFlagBits::eVertexInput ),
	    vk::DependencyFlags(),
	{}/*memoryBarrier*/, {
		vk::BufferMemoryBarrier (
		    vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eVertexAttributeRead,
		    v_instance->queues.pgc[0].graphics_queue_id, v_instance->queues.pgc[0].graphics_queue_id,
		    vertex_buffer->buffer, 0, vertex_buffer->size
		)
	}/*bufferBarrier*/, {}/*imageBarrier*/ );

	vk::ClearValue clearColors[2] = {
		vk::ClearValue ( ),
		vk::ClearValue ( )
	};
	vk::RenderPassBeginInfo renderPassBeginInfo = {
		renderpass, per_frame.framebuffer,
		vk::Rect2D ( vk::Offset2D ( viewport.offset.x, viewport.offset.y ), vk::Extent2D ( viewport.extend.x, viewport.extend.y ) ),
		2, clearColors
	};
	per_frame.commandbuffer.beginRenderPass ( renderPassBeginInfo, vk::SubpassContents::eInline );

	per_frame.commandbuffer.bindPipeline ( vk::PipelineBindPoint::eGraphics, pipeline );
	per_frame.commandbuffer.bindVertexBuffers ( 0, {vertex_buffer->buffer, vertex_buffer->buffer}, {0, sizeof ( QuadVertex ) * 6} );
	per_frame.commandbuffer.draw ( 6, 2, 0, 0 );

	/*VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
	vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

	VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
	vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);*/

	//1. simple textured quads
	//2. simple filled quads
	//3. fonts

	per_frame.commandbuffer.endRenderPass();
	per_frame.commandbuffer.end();

	return per_frame.commandbuffer;
}
RendResult VulkanQuadRenderer::render ( u32 frame_index, SubmitStore* state, u32 wait_sem_index, u32* final_sem_index ) {

	init();
	
	PerFrameQuadRenderObj& per_frame = per_target_data[frame_index];
	if ( per_frame.commandbuffer ) {
		per_frame.commandbuffer.reset ( vk::CommandBufferResetFlags() );
	} else {
		per_frame.commandbuffer = v_instance->createCommandBuffer ( g_commandpool, vk::CommandBufferLevel::ePrimary );
	}
	vk::CommandBufferBeginInfo begininfo = {
		vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr
	};
	per_frame.commandbuffer.begin ( begininfo );

	per_frame.commandbuffer.pipelineBarrier (
	    vk::PipelineStageFlags ( vk::PipelineStageFlagBits::eHost ), vk::PipelineStageFlags ( vk::PipelineStageFlagBits::eTransfer ),
	    vk::DependencyFlags(),
	{}/*memoryBarrier*/, {
		vk::BufferMemoryBarrier (
		    vk::AccessFlagBits::eHostWrite, vk::AccessFlagBits::eTransferRead,
		    v_instance->queues.pgc[0].graphics_queue_id, v_instance->queues.pgc[0].graphics_queue_id,
		    staging_vertex_buffer->buffer, 0, staging_vertex_buffer->size
		)
	}/*bufferBarrier*/, {}/*imageBarrier*/ );

	staging_vertex_buffer->transfer_to ( vertex_buffer, per_frame.commandbuffer );

	per_frame.commandbuffer.pipelineBarrier (
	    vk::PipelineStageFlags ( vk::PipelineStageFlagBits::eTransfer ), vk::PipelineStageFlags ( vk::PipelineStageFlagBits::eVertexInput ),
	    vk::DependencyFlags(),
	{}/*memoryBarrier*/, {
		vk::BufferMemoryBarrier (
		    vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eVertexAttributeRead,
		    v_instance->queues.pgc[0].graphics_queue_id, v_instance->queues.pgc[0].graphics_queue_id,
		    vertex_buffer->buffer, 0, vertex_buffer->size
		)
	}/*bufferBarrier*/, {}/*imageBarrier*/ );

	vk::ClearValue clearColors[2] = {
		vk::ClearValue ( ),
		vk::ClearValue ( )
	};
	vk::RenderPassBeginInfo renderPassBeginInfo = {
		renderpass, per_frame.framebuffer,
		vk::Rect2D ( vk::Offset2D ( viewport.offset.x, viewport.offset.y ), vk::Extent2D ( viewport.extend.x, viewport.extend.y ) ),
		2, clearColors
	};
	per_frame.commandbuffer.beginRenderPass ( renderPassBeginInfo, vk::SubpassContents::eInline );

	per_frame.commandbuffer.bindPipeline ( vk::PipelineBindPoint::eGraphics, pipeline );
	per_frame.commandbuffer.bindVertexBuffers ( 0, {vertex_buffer->buffer, vertex_buffer->buffer}, {0, sizeof ( QuadVertex ) * 6} );
	per_frame.commandbuffer.draw ( 6, 2, 0, 0 );

	/*VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
	vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

	VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
	vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);*/

	//1. simple textured quads
	//2. simple filled quads
	//3. fonts
	
	per_frame.commandbuffer.endRenderPass();
	per_frame.commandbuffer.end();

	SubmitInfo submitinfo = {
		vk::PipelineStageFlags() | vk::PipelineStageFlagBits::eHost | vk::PipelineStageFlagBits::eVertexInput,
		wait_sem_index, 1,
		state->commandbuffers.size(), 1,
		wait_sem_index, 1
	};
	
	state->commandbuffers.push_back(per_frame.commandbuffer);
	state->submitinfos.push_back(submitinfo);
	
	*final_sem_index = wait_sem_index;
	return RendResult::eSuccess;
}

void VulkanQuadRenderer::inherit ( VulkanQuadRenderer* old_quad_renderer ) {

	pipeline = vk::Pipeline();
	g_commandpool = vk::CommandPool();
	t_commandpool = vk::CommandPool();
	renderpass = vk::RenderPass();

	vertex_shader = old_quad_renderer->vertex_shader;
	old_quad_renderer->vertex_shader = nullptr;
	fragment_shader = old_quad_renderer->fragment_shader;
	old_quad_renderer->fragment_shader = nullptr;

	pipeline_layout = vk::PipelineLayout();

	per_target_data.clear();

	vertex_buffer = old_quad_renderer->vertex_buffer;
	old_quad_renderer->vertex_buffer = nullptr;
	staging_vertex_buffer = old_quad_renderer->staging_vertex_buffer;
	old_quad_renderer->staging_vertex_buffer = nullptr;

	for ( auto dsl : descriptor_set_layouts ) {
		dsl = vk::DescriptorSetLayout();
	}
}
