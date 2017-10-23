#include <stdio.h>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <algorithm>
#include <vector>
#include <fstream>
#include "VGlobal.h"
#include "VHeader.h"
#include "VWindow.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

static std::vector<char> readFile(const char* filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		printf("Couldn't open File %s\n", filename);
		return std::vector<char>();
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

//MainClass

//Window




int main (int argc, char **argv) {
	printf ("hello world\n");
	glfwInit();

	vGlobal.preInitialize();
	
	printf("Instance Extensions:\n");
	for(VkExtensionProperties& extProp : vGlobal.instExtLayers.availableExtensions){
		printf("\t%s\n", extProp.extensionName);
	}
	printf("Instance Layers:\n");
	for(VkLayerProperties& layerProp : vGlobal.instExtLayers.availableLayers){
		printf("\t%s\n", layerProp.layerName);
		printf("\t\tDesc: %s\n", layerProp.description);
	}
	
	if(!vGlobal.instExtLayers.activateLayer("VK_LAYER_LUNARG_standard_validation")){
		printf("Extension VK_LAYER_LUNARG_standard_validation not available\n");
	}
	if(!vGlobal.instExtLayers.activateLayer("VK_LAYER_LUNARG_swapchain")){
		printf("Extension VK_LAYER_LUNARG_swapchain not available\n");
	}
	/*if(!vGlobal.instExtLayers.activateLayer("VK_LAYER_RENDERDOC_Capture")){
		printf("Extension VK_LAYER_RENDERDOC_Capture not available\n");
	}
	if(!vGlobal.activateLayer("VK_LAYER_LUNARG_api_dump")){
		printf("Extension VK_LAYER_LUNARG_api_dump not available\n");
	}*/
	uint32_t instanceExtCount;
	const char** glfwReqInstanceExt = glfwGetRequiredInstanceExtensions (&instanceExtCount);
	for (size_t i = 0; i < instanceExtCount; i++) {
		if(!vGlobal.instExtLayers.activateExtension(glfwReqInstanceExt[i])){
			printf("Extension %s not available\n", glfwReqInstanceExt[i]);
		}
	}
	if(!vGlobal.instExtLayers.activateExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)){
		printf("Extension %s not available\n", VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}
	
	printf("Instance Extensions available:\n");
	for(VkExtensionProperties& prop : vGlobal.instExtLayers.availableExtensions){
		printf("\t%s\n", prop.extensionName);
	}
	printf("Instance Layers available:\n");
	for(VkLayerProperties& prop : vGlobal.instExtLayers.availableLayers){
		printf("\t%s\n", prop.layerName);
	}
	
	vGlobal.initializeInstance("Blabla", "wuwu Engine");
	
	vGlobal.choseBestDevice();

	printf("Device Extensions available:\n");
	for(VkExtensionProperties& prop : vGlobal.devExtLayers.availableExtensions){
		printf("\t%s\n", prop.extensionName);
	}
	printf("Device Layers available:\n");
	for(VkLayerProperties& prop : vGlobal.devExtLayers.availableLayers){
		printf("\t%s\n", prop.layerName);
	}
	
	if(!vGlobal.devExtLayers.activateExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)){
		printf("Extension %s not available\n", VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}
	if(!vGlobal.devExtLayers.activateExtension(VK_NV_GLSL_SHADER_EXTENSION_NAME)){
		printf("Extension %s not available\n", VK_NV_GLSL_SHADER_EXTENSION_NAME);
	}
	//vGlobal.devExtLayers.activateExtension(VK_NV_GLSL_SHADER_EXTENSION_NAME);
	
	vGlobal.initializeDevice();
	
	VWindow vWindow;
	
	vWindow.initializeWindow();
	
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	VkPipelineLayout pipelineLayout;
	{
		std::vector<char> vertShader = readFile("../workingdir/shader/tri.vert");
		std::vector<char> fragShader = readFile("../workingdir/shader/tri.frag");
		
		VkShaderModuleCreateInfo shaderCreateInfo;
		shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderCreateInfo.pNext = nullptr;
		shaderCreateInfo.flags = 0;
		shaderCreateInfo.codeSize = vertShader.size();
		shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertShader.data());
		
		if (VkResult res = vkCreateShaderModule(vGlobal.vDevice.device, &shaderCreateInfo, nullptr, &vertShaderModule)) {
			printf("Creation of ShaderModule failed %d\n", res);
			return -1;
		}
		
		shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderCreateInfo.pNext = nullptr;
		shaderCreateInfo.flags = 0;
		shaderCreateInfo.codeSize = fragShader.size();
		shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragShader.data());
		
		if (VkResult res = vkCreateShaderModule(vGlobal.vDevice.device, &shaderCreateInfo, nullptr, &fragShaderModule)) {
			printf("Creation of ShaderModule failed %d\n", res);
			return -1;
		}
		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;
		
		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = 0; // Optional

		if (VkResult res = vkCreatePipelineLayout(vGlobal.vDevice.device, &pipelineLayoutInfo, nullptr, &pipelineLayout)) {
			printf("Creation of Pipeline Layout failed %d\n", res);
			return -1;
		}
	}
	VkRenderPass renderPass;
	{
		VkAttachmentDescription colorAttachment = {};//attachment reference for target image
		colorAttachment.format = vWindow.presentSwapFormat.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		if (VkResult res = vkCreateRenderPass(vGlobal.vDevice.device, &renderPassInfo, nullptr, &renderPass)) {
			printf("Creation of RenderPass failed %d\n", res);
			return -1;
		}
	}
	VkPipeline graphicsPipeline;
	{
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.pNext = nullptr;
		vertexInputInfo.flags = 0;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional
		
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.pNext = nullptr;
		inputAssembly.flags = 0;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float) vWindow.swapChainExtend.width;
		viewport.height = (float) vWindow.swapChainExtend.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		
		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = vWindow.swapChainExtend;
		
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;
		viewportState.flags = 0;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.pNext = nullptr;
		rasterizer.flags = 0;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
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
		
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		/*
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		 */
		
		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.pNext = nullptr;
		colorBlending.flags = 0;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional
		
		
		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.pNext = nullptr;
		vertShaderStageInfo.flags = 0;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";
		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.pNext = nullptr;
		fragShaderStageInfo.flags = 0;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";
		
		VkPipelineShaderStageCreateInfo shaderStages[2] = {vertShaderStageInfo, fragShaderStageInfo};
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
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional
		if (VkResult res = vkCreateGraphicsPipelines(vGlobal.vDevice.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline)) {
			printf("Creation of Graphic Pipeline failed %d\n", res);
			return -1;
		}
	}
	
	VkFramebuffer framebuffers[vWindow.presentImages.size()];
	
	for(int i = 0; i < vWindow.presentImages.size(); i++){
		
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &vWindow.presentImages[i];
		framebufferInfo.width = vWindow.swapChainExtend.width;
		framebufferInfo.height = vWindow.swapChainExtend.height;
		framebufferInfo.layers = 1;
		
		if (VkResult res = vkCreateFramebuffer(vGlobal.vDevice.device, &framebufferInfo, nullptr, &framebuffers[i])) {
			printf("Creation of Framebuffer %d failed %d\n", i, res);
			return -1;
		}
		
	}
	
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer[vWindow.presentImages.size()];
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = vWindow.pgcQueue->graphicsQId;
		poolInfo.flags = 0;
		if (VkResult res = vkCreateCommandPool(vGlobal.vDevice.device, &poolInfo, nullptr, &commandPool)) {
			printf("Creation of CommandPool failed %d\n", res);
			return -1;
		}
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = vWindow.presentImages.size();

		if (VkResult res = vkAllocateCommandBuffers(vGlobal.vDevice.device, &allocInfo, commandBuffer)) {
			printf("Creation of CommandBuffers failed %d\n", res);
			return -1;
		}
		
		
		for (size_t i = 0; i < vWindow.presentImages.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr; // Optional

			vkBeginCommandBuffer(commandBuffer[i], &beginInfo);
			
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.pNext = nullptr;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = framebuffers[i];
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = vWindow.swapChainExtend;
			renderPassInfo.clearValueCount = 1;
			VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
			renderPassInfo.pClearValues = &clearColor;
			{
				vkCmdBeginRenderPass(commandBuffer[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
				
				vkCmdBindPipeline(commandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
				vkCmdDraw(commandBuffer[i], 3, 1, 0, 0);
				
				vkCmdEndRenderPass(commandBuffer[i]);
			}
			if (VkResult res = vkEndCommandBuffer(commandBuffer[i])) {
				printf("Recording of CommandBuffer failed %d\n", res);
				return -1;
			}
		}
	}
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore drawFinishedSemaphore;
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
		for (size_t i = 0; i < vWindow.presentImages.size(); i++) {
			if (VkResult res = vkCreateSemaphore(vGlobal.vDevice.device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore)) {
				printf("Creation of Semaphore failed %d\n", res);
				return -1;
			}
		}
		if (VkResult res = vkCreateSemaphore(vGlobal.vDevice.device, &semaphoreCreateInfo, nullptr, &drawFinishedSemaphore)) {
			printf("Creation of Semaphore failed %d\n", res);
			return -1;
		}
	}
	while(true){
		printf("SwapImage Index: %d\n", vWindow.presentImageIndex);
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		VkSemaphore waitSemaphores[] = {vWindow.imageAvailableGuardSem};
		submitInfo.pWaitSemaphores = waitSemaphores;
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer[vWindow.presentImageIndex];
		submitInfo.signalSemaphoreCount = 1;
		VkSemaphore signalSemaphores[] = {drawFinishedSemaphore};
		submitInfo.pSignalSemaphores = signalSemaphores;
		
		vWindow.pgcQueue->submitGraphics(1, &submitInfo, VK_NULL_HANDLE);
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		
		vWindow.showNextImage(1, signalSemaphores);
		
	}
	vkQueueWaitIdle(vWindow.pgcQueue->presentQueue);
	
	vkDeviceWaitIdle(vGlobal.vDevice.device);
	
	for (size_t i = 0; i < vWindow.presentImages.size(); i++) {
		vkDestroyFramebuffer(vGlobal.vDevice.device, framebuffers[i], nullptr);
	}
	vkDestroyPipeline(vGlobal.vDevice.device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(vGlobal.vDevice.device, pipelineLayout, nullptr);
	vkDestroyRenderPass(vGlobal.vDevice.device, renderPass, nullptr);
	for (size_t i = 0; i < vWindow.presentImages.size(); i++) {
		vkDestroyImageView(vGlobal.vDevice.device, vWindow.presentImages[i], nullptr);
	}
    vkDestroyShaderModule(vGlobal.vDevice.device, fragShaderModule, nullptr);
    vkDestroyShaderModule(vGlobal.vDevice.device, vertShaderModule, nullptr);
	vkDestroySwapchainKHR(vGlobal.vDevice.device, vWindow.swapChain, nullptr);
	

	glfwTerminate();

	return 0;
}
