
#include "VHeader.h"
#include "VBuilders.h"
#include "VGlobal.h"

VkPipeline standardPipeline = VK_NULL_HANDLE;

VkPipelineLayout standardPipelineLayout = VK_NULL_HANDLE;

VkPipelineLayout createStandardPipelineLayout(std::vector<VkDescriptorSetLayout>* descriptorSetLayouts, std::vector<VkPushConstantRange>* pushConstRanges){
	
	destroyStandardPipelineLayout();
	
	VkPipelineLayoutCreateInfo createInfo = {};
	
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	
	if(descriptorSetLayouts){
		createInfo.setLayoutCount = descriptorSetLayouts->size();
		createInfo.pSetLayouts = descriptorSetLayouts->data();
	}else{
		createInfo.setLayoutCount = 0;
		createInfo.pSetLayouts = nullptr;
	}
	if(pushConstRanges){
		createInfo.pushConstantRangeCount = pushConstRanges->size();
		createInfo.pPushConstantRanges = pushConstRanges->data();
	}else{
		createInfo.pushConstantRangeCount = 0;
		createInfo.pPushConstantRanges = nullptr;
	}
	
	VCHECKCALL(vkCreatePipelineLayout(vGlobal.deviceWrapper.device, &createInfo, nullptr, &standardPipelineLayout), printf("Creation of standard PipelineLayout failed\n"));
	
	return standardPipelineLayout;
}

void destroyStandardPipelineLayout(){
	if(standardPipelineLayout != VK_NULL_HANDLE)
		destroyPipelineLayout(standardPipelineLayout);
	standardPipelineLayout = VK_NULL_HANDLE;
}

void destroyPipelineLayout(VkPipelineLayout pipelineLayout){
	vkDestroyPipelineLayout(vGlobal.deviceWrapper.device, pipelineLayout, nullptr);
}

VkPipeline createStandardPipeline(VkExtent2D viewportExtend, VkPipelineLayout pipelineLayout, VkRenderPass renderPass){
	
	destroyStandardPipeline();
	
	VkVertexInputBindingDescription vertexInputBindings[2] = {};
	vertexInputBindings[0].binding = 0;
	vertexInputBindings[0].stride = sizeof(Vertex);
	vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexInputBindings[1].binding = 1;
	vertexInputBindings[1].stride = sizeof(Instance);
	vertexInputBindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	
	VkVertexInputAttributeDescription vertexInputAttributes[8] = {};
	vertexInputAttributes[0].location = 0;
	vertexInputAttributes[0].binding = 0;
	vertexInputAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributes[0].offset = offsetof(Vertex, pos);
	vertexInputAttributes[1].location = 1;
	vertexInputAttributes[1].binding = 0;
	vertexInputAttributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributes[1].offset = offsetof(Vertex, color);
	vertexInputAttributes[2].location = 2;
	vertexInputAttributes[2].binding = 0;
	vertexInputAttributes[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributes[2].offset = offsetof(Vertex, uv);
	vertexInputAttributes[3].location = 3;
	vertexInputAttributes[3].binding = 0;
	vertexInputAttributes[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributes[3].offset = offsetof(Vertex, normal);
	
	vertexInputAttributes[4].location = 4;
	vertexInputAttributes[4].binding = 1;
	vertexInputAttributes[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributes[4].offset = offsetof(Instance, m2wMatrix);
	vertexInputAttributes[5].location = 5;
	vertexInputAttributes[5].binding = 1;
	vertexInputAttributes[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributes[5].offset = offsetof(Instance, m2wMatrix) + sizeof(glm::vec4);
	vertexInputAttributes[6].location = 6;
	vertexInputAttributes[6].binding = 1;
	vertexInputAttributes[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributes[6].offset = offsetof(Instance, m2wMatrix) + sizeof(glm::vec4)*2;
	vertexInputAttributes[7].location = 7;
	vertexInputAttributes[7].binding = 1;
	vertexInputAttributes[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributes[7].offset = offsetof(Instance, m2wMatrix) + sizeof(glm::vec4)*3;
	
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.pNext = nullptr;
	vertexInputInfo.flags = 0;
	vertexInputInfo.vertexBindingDescriptionCount = 2;
	vertexInputInfo.pVertexBindingDescriptions = vertexInputBindings;
	vertexInputInfo.vertexAttributeDescriptionCount = 8;
	vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes;
	
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.pNext = nullptr;
	inputAssembly.flags = 0;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	
	VkViewport viewports[1] = {};
	viewports[0].x = 0.0f;
	viewports[0].y = 0.0f;
	viewports[0].width = (float) viewportExtend.width;
	viewports[0].height = (float) viewportExtend.height;
	viewports[0].minDepth = -1.0f;
	viewports[0].maxDepth = 1.0f;
	
	VkRect2D scissors[1] = {};
	scissors[0].offset = {0, 0};
	scissors[0].extent = viewportExtend;
	
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;
	viewportState.flags = 0;
	viewportState.viewportCount = 1;
	viewportState.pViewports = viewports;
	viewportState.scissorCount = 1;
	viewportState.pScissors = scissors;
	
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.pNext = nullptr;
	rasterizer.flags = 0;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;//VK_POLYGON_MODE_POINT;//VK_POLYGON_MODE_LINE;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
	rasterizer.lineWidth = 1.0f;
	
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.pNext = nullptr;
	multisampling.flags = 0;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional
	
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.pNext = nullptr;
	depthStencil.flags = 0;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;
	
	VkPipelineColorBlendAttachmentState colorBlendAttachments[1];
	colorBlendAttachments[0].blendEnable = VK_FALSE;
	colorBlendAttachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachments[0].colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachments[0].alphaBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;
	colorBlending.flags = 0;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = colorBlendAttachments;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;
	
	VkPipelineShaderStageCreateInfo shaderStages[2] = {};
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].flags = 0;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = loadShaderFromFile ("../workingdir/shader/tri.vert");
	shaderStages[0].pName = "main";
	shaderStages[0].pSpecializationInfo = nullptr;
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].flags = 0;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = loadShaderFromFile ("../workingdir/shader/tri.frag");
	shaderStages[1].pName = "main";
	shaderStages[1].pSpecializationInfo = nullptr;
	
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.flags = 0;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional
	
	VCHECKCALL(vkCreateGraphicsPipelines(vGlobal.deviceWrapper.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &standardPipeline), printf("Creation of standard Pipeline failed\n"));
	return standardPipeline;
}

void destroyStandardPipeline(){
	if(standardPipeline != VK_NULL_HANDLE)
		destroyPipeline(standardPipeline);
	standardPipeline = VK_NULL_HANDLE;
}

void destroyPipeline(VkPipeline pipeline){
	vkDestroyPipeline(vGlobal.deviceWrapper.device, pipeline, nullptr);
}