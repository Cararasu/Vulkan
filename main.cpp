#include <stdio.h>

#include <chrono>
#include <thread>
#include <algorithm>
#include <vector>
#include <fstream>
#include "VGlobal.h"
#include "VHeader.h"
#include "VWindow.h"
#include "VBuilders.h"
#include "ViewPort.h"
#include "DataWrapper.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


VkBool32 VKAPI_PTR debugLogger(
			VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, 
			uint64_t object, size_t location, int32_t messageCode,
			const char* pLayerPrefix, const char* pMessage, void* pUserData){
	printf("Layer: %s - Message: %s\n", pLayerPrefix, pMessage);
	return VK_TRUE;
}

PFN_vkCreateDebugReportCallbackEXT pfn_vkCreateDebugReportCallbackEXT;

#include <fstream>

ObjectStorage objectStorage;

void loadDataFile(std::string file, Object* object){
	
	std::ifstream input(file, std::ios::binary);
	
	std::string str;
	std::getline(input, str, '\0');
	uint32_t vertexCount = 0;
	input.read(reinterpret_cast<char*>(&vertexCount), sizeof(uint32_t));
	
	uint32_t baseVertexIndex = objectStorage.vertices.size();
	objectStorage.vertices.resize(baseVertexIndex + vertexCount);
	for(size_t i = 0; i < vertexCount; i++){
		Vertex v;
		input.read(reinterpret_cast<char*>(&v.pos[0]), sizeof(float));
		input.read(reinterpret_cast<char*>(&v.uv[0]), sizeof(float));
		input.read(reinterpret_cast<char*>(&v.normal[0]), sizeof(float));
		
		input.read(reinterpret_cast<char*>(&v.pos[1]), sizeof(float));
		input.read(reinterpret_cast<char*>(&v.uv[1]), sizeof(float));
		input.read(reinterpret_cast<char*>(&v.normal[1]), sizeof(float));
		
		input.read(reinterpret_cast<char*>(&v.pos[2]), sizeof(float));
		input.read(reinterpret_cast<char*>(&v.uv[2]), sizeof(float));
		input.read(reinterpret_cast<char*>(&v.normal[2]), sizeof(float));
		
		v.color = {0.0f, 1.0f, 0.0f};
		
		objectStorage.vertices[baseVertexIndex + i] = v;
	}
	uint32_t indexCount;
	input.read(reinterpret_cast<char*>(&indexCount), sizeof(uint32_t));
	
	uint32_t baseIndexIndex = objectStorage.indices.size();
	objectStorage.indices.resize(baseIndexIndex + indexCount);
	
	input.read(reinterpret_cast<char*>(&objectStorage.indices[baseIndexIndex]), sizeof(uint32_t)*indexCount);

	object->parts.emplace_back();
	ObjectPart& part = object->parts.back();
	part.indexCount = indexCount;
	part.indexOffset = baseIndexIndex;
	part.vertexOffset = baseVertexIndex;
}


int main (int argc, char **argv) {
	
	objectStorage.objects.emplace_back();
	{
		Object& obj = objectStorage.objects.back();
		
		loadDataFile("../workingdir/assets/Tie_Fighter_Body.data", &obj);
		loadDataFile("../workingdir/assets/Tie_Fighter_Arm_L.data", &obj);
		loadDataFile("../workingdir/assets/Tie_Fighter_Arm_R.data", &obj);
		loadDataFile("../workingdir/assets/Tie_Fighter_Windows.data", &obj);
		loadDataFile("../workingdir/assets/Tie_Fighter_Wing_L.data", &obj);
		loadDataFile("../workingdir/assets/Tie_Fighter_Wing_R.data", &obj);
		
		obj.instances.push_back({glm::translate(glm::vec3(2.0f,0.0f,0.0f)) * glm::rotate(1.0f, glm::vec3(0.0f,1.0f,0.0f))});
		obj.instances.push_back({glm::translate(glm::vec3(0.0f,0.0f,0.0f))});
		obj.instances.push_back({glm::translate(glm::vec3(-1.0f,-1.0f,-2.0f))});
		obj.instances.push_back({glm::translate(glm::vec3(-2.0f,0.0f,0.0f))});
	}
	
	PerspectiveViewPort<float> viewport;
	
	viewport.m_viewvector = {-0.5f, -0.2f, 1.4f};
	viewport.m_focalpoint = {0.0f, 0.0f, 0.0f};
	viewport.m_upvector = {0.0f, 1.0f, 0.0f};
	viewport.m_distance = 2.0f;
	viewport.m_near = 0.01f;
	viewport.m_far = 100.0f;
	viewport.m_aspect = 1.0f;
	
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
		printf("Layer VK_LAYER_LUNARG_standard_validation not available\n");
	}
	if(!vGlobal.instExtLayers.activateLayer("VK_LAYER_LUNARG_swapchain")){
		printf("Layer VK_LAYER_LUNARG_swapchain not available\n");
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
	
	VkDebugReportCallbackEXT cb;
	
	pfn_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) glfwGetInstanceProcAddress(vGlobal.vkinstance, "vkCreateDebugReportCallbackEXT");
	
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo = {};
	debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugReportCallbackCreateInfo.pNext = nullptr;
	debugReportCallbackCreateInfo.flags = /*VK_DEBUG_REPORT_INFORMATION_BIT_EXT |*/ VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	debugReportCallbackCreateInfo.pfnCallback = &debugLogger;
	debugReportCallbackCreateInfo.pUserData = nullptr;
	
	pfn_vkCreateDebugReportCallbackEXT(vGlobal.vkinstance, &debugReportCallbackCreateInfo, nullptr, &cb);
	
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
	
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts = createStandardDescriptorSetLayouts();
	
	VkPipelineLayout pipelineLayout = createStandardPipelineLayout(&descriptorSetLayouts);
	
	VkDescriptorPool descriptorSetPool = createStandardDescriptorSetPool();
	
	std::vector<VkDescriptorSet> descriptorSets;
	createStandardDescriptorSet(descriptorSetPool, &descriptorSetLayouts, &descriptorSets);
	VkDescriptorSet descriptorSet = descriptorSets[0];
	
	VkFormat depthFormat = findDepthFormat();
	VkExtent3D extent = {vWindow.swapChainExtend.width, vWindow.swapChainExtend.height, 1};
	ImageWrapper depthImage(extent, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	
	VkImageView depthImageView;
	
	depthImageView = createImageView2D(depthImage.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
	transitionImageLayout(depthImage.image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

	VkRenderPass renderPass = createStandardRenderPass(vWindow.presentSwapFormat.format);
	
	VkPipeline graphicsPipeline = createStandardPipeline(vWindow.swapChainExtend, pipelineLayout, renderPass);
	
	VkFramebuffer framebuffers[vWindow.presentImages.size()];
	
	for(size_t i = 0; i < vWindow.presentImages.size(); i++){
		VkImageView attachments[2] = {
			vWindow.presentImages[i],
			depthImageView
		};
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = vWindow.swapChainExtend.width;
		framebufferInfo.height = vWindow.swapChainExtend.height;
		framebufferInfo.layers = 1;
		
		VCHECKCALL(vkCreateFramebuffer(vGlobal.deviceWrapper.device, &framebufferInfo, nullptr, &framebuffers[i]), printf("Creation of Framebuffer %d failed\n", i));
		
	}
	
	BufferWrapper vertexBuffer(sizeof(objectStorage.vertices[0]) * objectStorage.vertices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	transferData(objectStorage.vertices.data(), vertexBuffer.buffer, 0, sizeof(objectStorage.vertices[0]) * objectStorage.vertices.size(), VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
	//vGlobal.deviceWrapper.tqueue->waitForFinish();
	
	BufferWrapper indexBuffer(sizeof(objectStorage.indices[0]) * objectStorage.indices.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	transferData(objectStorage.indices.data(), indexBuffer.buffer, 0, sizeof(objectStorage.indices[0]) * objectStorage.indices.size(), VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
	//vGlobal.deviceWrapper.tqueue->waitForFinish();
	
	
	uint32_t MAX_COMMAND_COUNT = 100;
	uint32_t MAX_INSTANCE_COUNT = 100;
	
	BufferWrapper indirectCommandBuffer(sizeof(VkDrawIndexedIndirectCommand) * MAX_COMMAND_COUNT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	BufferWrapper instanceBuffer(sizeof(Instance) * MAX_INSTANCE_COUNT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	BufferWrapper uniformBuffer(sizeof(Camera), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = uniformBuffer.buffer;
	bufferInfo.offset = offsetof(Camera, w2sMatrix);
	bufferInfo.range = sizeof(glm::mat4);
	
	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr; // Optional
	descriptorWrite.pTexelBufferView = nullptr; // Optional
	
	vkUpdateDescriptorSets(vGlobal.deviceWrapper.device, 1, &descriptorWrite, 0, nullptr);
	
	VkCommandPool commandPool = createCommandPool(vWindow.pgcQueue->graphicsQId, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	
	VkSemaphore imageAvailableSemaphore = createSemaphore(vGlobal.deviceWrapper.device);
	VkSemaphore drawFinishedSemaphore = createSemaphore(vGlobal.deviceWrapper.device);
	
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	
	while(vWindow.isOpen()){
		
		uint32_t stagingOffset = 0;
		
		uint32_t commandOffset = stagingOffset;
		uint32_t commandCount = 0;
		{
			uint32_t count = 0;
			for(Object& obj : objectStorage.objects){
				for(ObjectPart& part : obj.parts){
					((VkDrawIndexedIndirectCommand*)stagingBuffer->data)[commandCount++] = {part.indexCount, obj.instances.size(), part.indexOffset, part.vertexOffset, count};
				}
				count += obj.instances.size();
			}
		}
		stagingOffset += sizeof(VkDrawIndexedIndirectCommand) * commandCount;
		
		uint32_t instanceCount = 0;
		uint32_t instanceOffset = stagingOffset;
		for(Object& obj : objectStorage.objects){
			Instance* instanceArray = (Instance*)(stagingBuffer->data + stagingOffset);
			memcpy(&(instanceArray[instanceCount]), obj.instances.data(), sizeof(Instance)*obj.instances.size());
			instanceCount += obj.instances.size();
		}
		stagingOffset += sizeof(Instance) * instanceCount;
		
		uint32_t uniformOffset = stagingOffset;
		viewport.m_viewvector = glm::rotate(viewport.m_viewvector, 0.005f, glm::vec3(0.0f,1.0f,0.0f));
		((Camera*)(stagingBuffer->data + stagingOffset))[0].w2sMatrix = viewport.createWorldToScreenSpaceMatrix();
		stagingOffset += sizeof(Camera);
		
		copyBuffer(stagingBuffer->buffer, indirectCommandBuffer.buffer, commandOffset, 0, sizeof(VkDrawIndexedIndirectCommand)*commandCount,
			VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
		
		copyBuffer(stagingBuffer->buffer, instanceBuffer.buffer, instanceOffset, 0, sizeof(Instance)*instanceCount,
			VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_INDEX_READ_BIT);
		
		copyBuffer(stagingBuffer->buffer, uniformBuffer.buffer, uniformOffset, 0, sizeof(Camera),
			VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_UNIFORM_READ_BIT);
		
		
		printf("Commands: %d - %d Instances: %d - %d Uniform: %d\n", commandCount, commandOffset, instanceCount, instanceOffset, uniformOffset);
		printf("Id: %d\n", vWindow.presentImageIndex);
		//vGlobal.deviceWrapper.tqueue->waitForFinish();
		
		if(commandBuffer != VK_NULL_HANDLE)
			deleteCommandBuffer(commandPool, commandBuffer);
		commandBuffer = createCommandBuffer(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr; // Optional

		
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = framebuffers[vWindow.presentImageIndex];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = vWindow.swapChainExtend;
		renderPassInfo.clearValueCount = 2;
		VkClearValue clearColors[2];
		clearColors[0].color = {0.0f, 0.0f, 0.5f, 1.0f};
		clearColors[0].depthStencil = {0.0f, 0};
		clearColors[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
		clearColors[1].depthStencil = {1.0f, 0};
		renderPassInfo.pClearValues = clearColors;
		{
			vkBeginCommandBuffer(commandBuffer, &beginInfo);
			
			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,  pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
			VkBuffer vertexBuffers[] = {vertexBuffer.buffer, instanceBuffer.buffer};
			VkDeviceSize offsets[] = {0, 0};
			vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			
			vkCmdDrawIndexedIndirect(commandBuffer, indirectCommandBuffer.buffer, 0, commandCount, sizeof(VkDrawIndexedIndirectCommand));
			
			vkCmdEndRenderPass(commandBuffer);
			
			VCHECKCALL(vkEndCommandBuffer(commandBuffer), printf("Recording of CommandBuffer failed\n"));
		}
		
		
		VkSemaphore waitSemaphores[] = {vWindow.imageAvailableGuardSem};
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		VkSemaphore signalSemaphores[] = {drawFinishedSemaphore};
		
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		
		vWindow.pgcQueue->submitGraphics(1, &submitInfo, VK_NULL_HANDLE);
		
		vWindow.showNextImage(1, signalSemaphores);
		glfwPollEvents();
		
		std::this_thread::sleep_for(std::chrono::nanoseconds(10000000));
	}
	glfwDestroyWindow(vWindow.window);
	vkQueueWaitIdle(vWindow.pgcQueue->presentQueue);
	
	vkDeviceWaitIdle(vGlobal.deviceWrapper.device);
	
	
	for (size_t i = 0; i < vWindow.presentImages.size(); i++) {
		vkDestroyFramebuffer(vGlobal.deviceWrapper.device, framebuffers[i], nullptr);
	}
	vkDestroyPipelineLayout(vGlobal.deviceWrapper.device, pipelineLayout, nullptr);
	
	destroySemaphore(vGlobal.deviceWrapper.device, imageAvailableSemaphore);
	destroySemaphore(vGlobal.deviceWrapper.device, drawFinishedSemaphore);
	
	destroyStandardRenderPass();
	destroyStandardPipeline();
	for (size_t i = 0; i < vWindow.presentImages.size(); i++) {
		vkDestroyImageView(vGlobal.deviceWrapper.device, vWindow.presentImages[i], nullptr);
	}
	vkDestroySwapchainKHR(vGlobal.deviceWrapper.device, vWindow.swapChain, nullptr);
	
	delete stagingBuffer;

	glfwTerminate();

	return 0;
}
