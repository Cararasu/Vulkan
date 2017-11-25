
#include "VHeader.h"
#include "VBuilders.h"
#include "VGlobal.h"

vk::PipelineLayout createStandardPipelineLayout (std::vector<vk::DescriptorSetLayout>* descriptorSetLayouts, std::vector<vk::PushConstantRange>* pushConstRanges) {

	vk::PipelineLayoutCreateInfo createInfo(
			vk::PipelineLayoutCreateFlags(),
			0, nullptr,/*setLayouts*/
			0, nullptr/*pushConstRanges*/
		);

	if (descriptorSetLayouts) {
		createInfo.setLayoutCount = descriptorSetLayouts->size();
		createInfo.pSetLayouts = descriptorSetLayouts->data();
	}
	if (pushConstRanges) {
		createInfo.pushConstantRangeCount = pushConstRanges->size();
		createInfo.pPushConstantRanges = pushConstRanges->data();
	}

	return global.deviceWrapper.device.createPipelineLayout (createInfo, nullptr);
}

void destroyPipelineLayout (vk::PipelineLayout pipelineLayout) {
	vkDestroyPipelineLayout (global.deviceWrapper.device, pipelineLayout, nullptr);
}


vk::Pipeline createStandardPipeline (vk::Extent2D viewportExtend, vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass) {
	std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
		vk::VertexInputBindingDescription (0, sizeof (Vertex), vk::VertexInputRate::eVertex),
		vk::VertexInputBindingDescription (1, sizeof (Instance), vk::VertexInputRate::eInstance)
	};

	std::array<vk::VertexInputAttributeDescription, 11> vertexInputAttributes = {
		vk::VertexInputAttributeDescription (0, 0, vk::Format::eR32G32B32Sfloat, offsetof (Vertex, pos)),
		vk::VertexInputAttributeDescription (1, 0, vk::Format::eR32G32B32Sfloat, offsetof (Vertex, uv)),
		vk::VertexInputAttributeDescription (2, 0, vk::Format::eR32G32B32Sfloat, offsetof (Vertex, normal)),
		
		vk::VertexInputAttributeDescription (4, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (Instance, m2wMatrix)),
		vk::VertexInputAttributeDescription (5, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (Instance, m2wMatrix) + sizeof (glm::vec4)),
		vk::VertexInputAttributeDescription (6, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (Instance, m2wMatrix) + sizeof (glm::vec4) * 2),
		vk::VertexInputAttributeDescription (7, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (Instance, m2wMatrix) + sizeof (glm::vec4) * 3),
		vk::VertexInputAttributeDescription (8, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (Instance, m2wMatrix) + sizeof (glm::vec4) * 3),
		vk::VertexInputAttributeDescription (9, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (Instance, m2wMatrix) + sizeof (glm::vec4) * 3),
		vk::VertexInputAttributeDescription (10, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (Instance, m2wMatrix) + sizeof (glm::vec4) * 3),
		vk::VertexInputAttributeDescription (11, 1, vk::Format::eR32G32B32A32Sfloat, offsetof (Instance, m2wMatrix) + sizeof (glm::vec4) * 3),
	};

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo (vk::PipelineVertexInputStateCreateFlags(), vertexInputBindings.size(), vertexInputBindings.data(), vertexInputAttributes.size(), vertexInputAttributes.data());

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly(vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE);

	vk::Viewport viewports[] = {
		vk::Viewport(0.0f, 0.0f, (float) viewportExtend.width, (float) viewportExtend.height, -1.0f, 1.0f)
	};
	
	vk::Rect2D scissors[] = {
		vk::Rect2D(vk::Offset2D(0,0), viewportExtend),
	};

	vk::PipelineViewportStateCreateInfo viewportState(vk::PipelineViewportStateCreateFlags(), 1, viewports, 1, scissors);

	vk::PipelineRasterizationStateCreateInfo rasterizer(vk::PipelineRasterizationStateCreateFlags(), 
		VK_FALSE, VK_FALSE, //depthClampEnable, rasterizerDiscardEnable
		vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise,
		VK_FALSE, //depthBiasEnable
		0.0f, //depthBiasConstantFactor
		0.0f, //depthBiasClamp
		0.0f, //depthBiasSlopeFactor
		1.0f);//lineWidth

	vk::PipelineMultisampleStateCreateInfo multisampling(
		vk::PipelineMultisampleStateCreateFlags(),
		vk::SampleCountFlagBits::e1,
		VK_FALSE,//sampleShadingEnable
		1.0f, nullptr, //minSampleShading, pSampleMask
		VK_FALSE, VK_FALSE //alphaToCoverageEnable, alphaToOneEnable
	);

	vk::PipelineDepthStencilStateCreateInfo depthStencil(
		vk::PipelineDepthStencilStateCreateFlags(),
		VK_TRUE, VK_TRUE, //depthTestEnable, depthWriteEnable
		vk::CompareOp::eLess, //depthCompareOp
		VK_FALSE, VK_FALSE, //depthBoundsTestEnable, stencilTestEnable
		{}, {}, //fron, back
		0.0f, 1.0f //minDepthBounds, maxDepthBounds
	);

	vk::PipelineColorBlendAttachmentState colorBlendAttachments[1] = {
		vk::PipelineColorBlendAttachmentState(
			VK_FALSE, //blendEnable
			vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcColorBlendFactor, dstColorBlendFactor
			vk::BlendOp::eAdd,//colorBlendOp
			vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
			vk::BlendOp::eAdd,//alphaBlendOp
			vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
		)
	};

	vk::PipelineColorBlendStateCreateInfo colorBlending(
		vk::PipelineColorBlendStateCreateFlags(),
		VK_FALSE, vk::LogicOp::eCopy,//logicOpEnable, logicOp
		1, colorBlendAttachments, // attachments
		{0.0f,0.0f,0.0f,0.0f} //blendConstants
	);

	vk::PipelineShaderStageCreateInfo shaderStages[2] = {
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eVertex, global.shadermodule.standardShaderVert,
			"main", nullptr//name, specialization
		),
		vk::PipelineShaderStageCreateInfo(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eFragment, global.shadermodule.standardShaderFrag,
			"main", nullptr//name, specialization
		),
	};

	vk::GraphicsPipelineCreateInfo pipelineInfo(
		vk::PipelineCreateFlags(),
		2, shaderStages,
		&vertexInputInfo, &inputAssembly, nullptr, &viewportState, &rasterizer, &multisampling, &depthStencil, &colorBlending,
		nullptr,
		pipelineLayout,
		renderPass,
		0,
		vk::Pipeline(),
		-1
	);
	
	return global.deviceWrapper.device.createGraphicsPipelines(vk::PipelineCache(), {pipelineInfo}, nullptr)[0];
}

void destroyPipeline (vk::Pipeline pipeline) {
	vkDestroyPipeline (global.deviceWrapper.device, pipeline, nullptr);
}
