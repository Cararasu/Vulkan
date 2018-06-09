
#include "PipelineModule.h"
#include "VGlobal.h"
#include "VInstance.h"

std::vector<vk::DescriptorSetLayout> StandardPipelineModuleBuilder::createDescriptorSetLayouts() {
	vk::DescriptorSetLayoutBinding bindings1[] = {
		vk::DescriptorSetLayoutBinding (0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
	};
	vk::DescriptorSetLayoutBinding bindings2[] = {
		vk::DescriptorSetLayoutBinding (0, vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr),
		vk::DescriptorSetLayoutBinding (1, vk::DescriptorType::eSampledImage, 3, vk::ShaderStageFlagBits::eFragment, nullptr)
	};

	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
		instance->device.createDescriptorSetLayout (vk::DescriptorSetLayoutCreateInfo (vk::DescriptorSetLayoutCreateFlags(), 1, bindings1), nullptr),
		instance->device.createDescriptorSetLayout (vk::DescriptorSetLayoutCreateInfo (vk::DescriptorSetLayoutCreateFlags(), 2, bindings2), nullptr)
	};
	return descriptorSetLayouts;
}
std::vector<vk::PushConstantRange> StandardPipelineModuleBuilder::createPushConstantRanges() {
	return {vk::PushConstantRange (vk::ShaderStageFlagBits::eFragment, 0, sizeof (ObjectPartData)) };
}
vk::PipelineLayout StandardPipelineModuleBuilder::createPipelineLayout (std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts, std::vector<vk::PushConstantRange>& pushConstRanges) {
	vk::PipelineLayoutCreateInfo createInfo (
	    vk::PipelineLayoutCreateFlags(),
	    0, nullptr,/*setLayouts*/
	    0, nullptr/*pushConstRanges*/
	);
	if (descriptorSetLayouts.size() != 0) {
		createInfo.setLayoutCount = descriptorSetLayouts.size();
		createInfo.pSetLayouts = descriptorSetLayouts.data();
	}
	if (pushConstRanges.size() != 0) {
		createInfo.pushConstantRangeCount = pushConstRanges.size();
		createInfo.pPushConstantRanges = pushConstRanges.data();
	}
	return instance->device.createPipelineLayout (createInfo, nullptr);
}
vk::RenderPass StandardPipelineModuleBuilder::createRenderPass (vk::Format format) {
	
	global.shadermodule.standardShaderVert = instance->loadShaderFromFile (this->shader_files[0]);
	global.shadermodule.standardShaderFrag = instance->loadShaderFromFile (this->shader_files[1]);
	vk::AttachmentDescription attachments[2] = {
		vk::AttachmentDescription (vk::AttachmentDescriptionFlags(),
		                           format, vk::SampleCountFlagBits::e1,//format, samples
		                           vk::AttachmentLoadOp::eClear,//loadOp
		                           vk::AttachmentStoreOp::eStore,//storeOp
		                           vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
		                           vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
		                           vk::ImageLayout::eUndefined,//initialLaylout
		                           vk::ImageLayout::ePresentSrcKHR//finalLayout
		                          ),
		vk::AttachmentDescription (vk::AttachmentDescriptionFlags(),
		                           instance->findDepthFormat(), vk::SampleCountFlagBits::e1,//format, samples
		                           vk::AttachmentLoadOp::eClear,//loadOp
		                           vk::AttachmentStoreOp::eDontCare,//storeOp
		                           vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
		                           vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
		                           vk::ImageLayout::eUndefined,//initialLaylout
		                           vk::ImageLayout::eDepthStencilAttachmentOptimal//finalLayout
		                          )
	};

	vk::AttachmentReference colorAttachmentRef (0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference depthAttachmentRef (1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass (
	    vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics,
	    0, nullptr/*inputAttackments*/,
	    1, &colorAttachmentRef/*colorAttachments*/,
	    nullptr,/*resolveAttachments*/
	    &depthAttachmentRef,/*depthAttackment*/
	    0, nullptr/*preserveAttachments*/
	);

	vk::RenderPassCreateInfo renderPassInfo (vk::RenderPassCreateFlags(), 2, attachments, 1, &subpass, 0, nullptr/*dependencies*/);

	return instance->device.createRenderPass (renderPassInfo, nullptr);
}
vk::Pipeline StandardPipelineModuleBuilder::createPipeline (vk::Extent2D extent, vk::RenderPass renderPass, vk::PipelineLayout layout) {

	std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
		vk::VertexInputBindingDescription (0, sizeof (Vertex), vk::VertexInputRate::eVertex),
		vk::VertexInputBindingDescription (1, sizeof (InstanceObj), vk::VertexInputRate::eInstance)
	};

	std::array<vk::VertexInputAttributeDescription, 7> vertexInputAttributes = {
		vk::VertexInputAttributeDescription (0, 0, vk::Format::eR32G32B32Sfloat, offsetof (Vertex, pos)),
		vk::VertexInputAttributeDescription (1, 0, vk::Format::eR32G32B32Sfloat, offsetof (Vertex, uv)),
		vk::VertexInputAttributeDescription (2, 0, vk::Format::eR32G32B32Sfloat, offsetof (Vertex, normal)),

		vk::VertexInputAttributeDescription (4, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (InstanceObj, m2wMatrix)),
		vk::VertexInputAttributeDescription (5, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (InstanceObj, m2wMatrix) + sizeof (glm::vec4)),
		vk::VertexInputAttributeDescription (6, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (InstanceObj, m2wMatrix) + sizeof (glm::vec4) * 2),
		vk::VertexInputAttributeDescription (7, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (InstanceObj, m2wMatrix) + sizeof (glm::vec4) * 3),
		//vk::VertexInputAttributeDescription (8, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (InstanceObj, m2wMatrix) + sizeof (glm::vec4) * 3),
		//vk::VertexInputAttributeDescription (9, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (InstanceObj, m2wMatrix) + sizeof (glm::vec4) * 3),
		//vk::VertexInputAttributeDescription (10, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (InstanceObj, m2wMatrix) + sizeof (glm::vec4) * 3),
		//vk::VertexInputAttributeDescription (11, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (InstanceObj, m2wMatrix) + sizeof (glm::vec4) * 3),
	};

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo (vk::PipelineVertexInputStateCreateFlags(), vertexInputBindings.size(), vertexInputBindings.data(), vertexInputAttributes.size(), vertexInputAttributes.data());

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly (vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE);

	vk::Viewport viewports[] = {
		vk::Viewport (0.0f, 0.0f, (float) extent.width, (float) extent.height, -1.0f, 1.0f)
	};

	vk::Rect2D scissors[] = {
		vk::Rect2D (vk::Offset2D (0, 0), extent),
	};

	vk::PipelineViewportStateCreateInfo viewportState (vk::PipelineViewportStateCreateFlags(), 1, viewports, 1, scissors);

	vk::PipelineRasterizationStateCreateInfo rasterizer (vk::PipelineRasterizationStateCreateFlags(),
	        VK_FALSE, VK_FALSE, //depthClampEnable, rasterizerDiscardEnable
	        vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise,
	        VK_FALSE, //depthBiasEnable
	        0.0f, //depthBiasConstantFactor
	        0.0f, //depthBiasClamp
	        0.0f, //depthBiasSlopeFactor
	        1.0f);//lineWidth

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
		    vk::ShaderStageFlagBits::eVertex, global.shadermodule.standardShaderVert,
		    "main", nullptr//name, specialization
		),
		vk::PipelineShaderStageCreateInfo (
		    vk::PipelineShaderStageCreateFlags(),
		    vk::ShaderStageFlagBits::eFragment, global.shadermodule.standardShaderFrag,
		    "main", nullptr//name, specialization
		),
	};
	vk::GraphicsPipelineCreateInfo pipelineInfo (
	    vk::PipelineCreateFlags(),
	    2, shaderStages,
	    &vertexInputInfo, &inputAssembly, nullptr, &viewportState, &rasterizer, &multisampling, &depthStencil, &colorBlending,
	    nullptr,
	    layout,
	    renderPass,
	    0,
	    vk::Pipeline(),
	    -1
	);

	return instance->device.createGraphicsPipelines (vk::PipelineCache(), {pipelineInfo}, nullptr) [0];
}
void StandardPipelineModuleBuilder::destroyDescriptorSetLayouts (std::vector<vk::DescriptorSetLayout> descriptorSetLayouts) {
	for (vk::DescriptorSetLayout dsl : descriptorSetLayouts){
		instance->device.destroyDescriptorSetLayout(dsl);
	}
}
void StandardPipelineModuleBuilder::destroyPipelineLayout (vk::PipelineLayout pipelineLayout) {
	instance->device.destroyPipelineLayout(pipelineLayout);
}
void StandardPipelineModuleBuilder::destroyRenderPass (vk::RenderPass renderPass) {
	instance->device.destroyRenderPass(renderPass);
}
void StandardPipelineModuleBuilder::destroyPipeline (vk::Pipeline pipeline) {
	instance->device.destroyPipeline(pipeline);
}
