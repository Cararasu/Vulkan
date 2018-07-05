#include "VulkanQuadRenderer.h"


VulkanQuadRenderer::~VulkanQuadRenderer() {
	if ( pipeline )
		vulkan_device ( v_instance ).destroyPipeline ( pipeline );
	destroy_framebuffers();
	if(commandpool)
		vulkan_device(v_instance).destroyCommandPool(commandpool);
	if ( renderpass )
		vulkan_device ( v_instance ).destroyRenderPass ( renderpass );
	if ( vertex_shader )
		vulkan_device ( v_instance ).destroyShaderModule ( vertex_shader );
	if ( fragment_shader )
		vulkan_device ( v_instance ).destroyShaderModule ( fragment_shader );
	if ( pipeline_layout )
		vulkan_device ( v_instance ).destroyPipelineLayout ( pipeline_layout );
	for ( auto dsl : descriptor_set_layouts ) {
		vulkan_device ( v_instance ).destroyDescriptorSetLayout ( dsl );
	}
}
void VulkanQuadRenderer::destroy_framebuffers(){
	for ( auto& fb : per_target_data ) {
		vulkan_device ( v_instance ).destroyFramebuffer ( fb.framebuffer );
	}
}

RendResult VulkanQuadRenderer::update_extend ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) {
	
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
			vulkan_device ( v_instance ).createDescriptorSetLayout ( vk::DescriptorSetLayoutCreateInfo ( vk::DescriptorSetLayoutCreateFlags(), 1, bindings1 ), nullptr ),
			vulkan_device ( v_instance ).createDescriptorSetLayout ( vk::DescriptorSetLayoutCreateInfo ( vk::DescriptorSetLayoutCreateFlags(), 2, bindings2 ), nullptr )
		};

		std::array<vk::PushConstantRange, 1> pushConstRanges = {vk::PushConstantRange ( vk::ShaderStageFlagBits::eFragment, 0, sizeof ( ObjectPartData ) ) };

		vk::PipelineLayoutCreateInfo createInfo (
		    vk::PipelineLayoutCreateFlags(),
		    0, nullptr,/*setLayouts*/
		    0, nullptr/*pushConstRanges*/
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
		vertex_shader = v_instance->v_resource_manager->load_shader_from_file ( "shader/quad.vert" );
	}
	if ( !fragment_shader ) {
		fragment_shader = v_instance->v_resource_manager->load_shader_from_file ( "shader/quad.frag" );
	}
	if ( !renderpass ) {
		vk::AttachmentDescription attachments[2] = {
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            target_wrapper->color_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eClear,//loadOp
			                            vk::AttachmentStoreOp::eStore,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eUndefined,//initialLaylout
			                            vk::ImageLayout::ePresentSrcKHR//finalLayout
			                          ),
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            target_wrapper->depth_stencil_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eClear,//loadOp
			                            vk::AttachmentStoreOp::eDontCare,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eUndefined,//initialLaylout
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
		vk::VertexInputBindingDescription ( 0, sizeof ( Vertex ), vk::VertexInputRate::eVertex ),
		vk::VertexInputBindingDescription ( 1, sizeof ( InstanceObj ), vk::VertexInputRate::eInstance )
	};

	std::array<vk::VertexInputAttributeDescription, 7> vertexInputAttributes = {
		vk::VertexInputAttributeDescription ( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof ( Vertex, pos ) ),
		vk::VertexInputAttributeDescription ( 1, 0, vk::Format::eR32G32B32Sfloat, offsetof ( Vertex, uv ) ),
		vk::VertexInputAttributeDescription ( 2, 0, vk::Format::eR32G32B32Sfloat, offsetof ( Vertex, normal ) ),

		vk::VertexInputAttributeDescription ( 4, 1, vk::Format::eR32G32B32A32Sfloat, offsetof ( InstanceObj, m2wMatrix ) ),
		vk::VertexInputAttributeDescription ( 5, 1, vk::Format::eR32G32B32A32Sfloat, offsetof ( InstanceObj, m2wMatrix ) + sizeof ( glm::vec4 ) ),
		vk::VertexInputAttributeDescription ( 6, 1, vk::Format::eR32G32B32A32Sfloat, offsetof ( InstanceObj, m2wMatrix ) + sizeof ( glm::vec4 ) * 2 ),
		vk::VertexInputAttributeDescription ( 7, 1, vk::Format::eR32G32B32A32Sfloat, offsetof ( InstanceObj, m2wMatrix ) + sizeof ( glm::vec4 ) * 3 )
	};

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo ( vk::PipelineVertexInputStateCreateFlags(), vertexInputBindings.size(), vertexInputBindings.data(), vertexInputAttributes.size(), vertexInputAttributes.data() );

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

	printf("Update Quad-Renderer\n");
}

RendResult VulkanQuadRenderer::render_quads(u32 index) {
	PerFrameQuadRenderObj& per_frame = per_target_data[index];
	if(per_frame.commandbuffer){
		per_frame.commandbuffer.reset(vk::CommandBufferResetFlags());
	}else{
		per_frame.commandbuffer = v_instance->createCommandBuffer(commandpool, vk::CommandBufferLevel::ePrimary);
	}
	vk::CommandBufferBeginInfo begininfo = {
		vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr
	};
	vk::RenderPassBeginInfo renderPassBeginInfo = {
		renderpass, per_frame.framebuffer, vk::Rect2D(),
		0, nullptr
	};
	per_frame.commandbuffer.begin(begininfo);
	per_frame.commandbuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
	
	/*VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
	vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

	VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
	vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);*/
	
	//here be commands
	
	//1. simple textured quads
	//2. simple bordered quads
	//3. fonts
	
	per_frame.commandbuffer.endRenderPass();
	per_frame.commandbuffer.end();
	
	


}
