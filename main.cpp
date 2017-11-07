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


vk::Bool32 VKAPI_PTR debugLogger(
			vk::DebugReportFlagsEXT flags, vk::DebugReportObjectTypeEXT objectType, 
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
	for(vk::ExtensionProperties& extProp : vGlobal.instExtLayers.availableExtensions){
		printf("\t%s\n", extProp.extensionName);
	}
	printf("Instance Layers:\n");
	for(vk::LayerProperties& layerProp : vGlobal.instExtLayers.availableLayers){
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
	for(vk::ExtensionProperties& prop : vGlobal.instExtLayers.availableExtensions){
		printf("\t%s\n", prop.extensionName);
	}
	printf("Instance Layers available:\n");
	for(vk::LayerProperties& prop : vGlobal.instExtLayers.availableLayers){
		printf("\t%s\n", prop.layerName);
	}
	
	vGlobal.initializeInstance("Blabla", "wuwu Engine");
	
	vk::DebugReportCallbackEXT cb;
	
	pfn_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) glfwGetInstanceProcAddress(vGlobal.vkinstance, "vkCreateDebugReportCallbackEXT");
	
	vk::DebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo(
		vk::DebugReportFlagsEXT(
			//vk::DebugReportFlagBitsEXT::eInformation | 
			vk::DebugReportFlagBitsEXT::eWarning | 
			vk::DebugReportFlagBitsEXT::ePerformanceWarning | 
			vk::DebugReportFlagBitsEXT::eError | 
			vk::DebugReportFlagBitsEXT::eDebug),
		&debugLogger,
		nullptr
	);
	vGlobal.vkinstance.createDebugReportCallbackEXT(debugReportCallbackCreateInfo,&cb);
	
	vGlobal.choseBestDevice();

	

	printf("Device Extensions available:\n");
	for(vk::ExtensionProperties& prop : vGlobal.devExtLayers.availableExtensions){
		printf("\t%s\n", prop.extensionName);
	}
	printf("Device Layers available:\n");
	for(vk::LayerProperties& prop : vGlobal.devExtLayers.availableLayers){
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
	
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = createStandardDescriptorSetLayouts();
	
	vk::PipelineLayout pipelineLayout = createStandardPipelineLayout(&descriptorSetLayouts);
	
	vk::DescriptorPool descriptorSetPool = createStandardDescriptorSetPool();
	
	std::vector<vk::DescriptorSet> descriptorSets;
	createStandardDescriptorSet(descriptorSetPool, &descriptorSetLayouts, &descriptorSets);
	vk::DescriptorSet descriptorSet = descriptorSets[0];
	
	vk::Format depthFormat = findDepthFormat();
	vk::Extent3D extent(vWindow.swapChainExtend.width, vWindow.swapChainExtend.height, 1);
	ImageWrapper depthImage(extent, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment), vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eDeviceLocal));
	
	vk::ImageView depthImageView;
	
	depthImageView = createImageView2D(depthImage.image, (vk::Format)depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
	transitionImageLayout(depthImage.image, (vk::Format)depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

	vk::RenderPass renderPass = createStandardRenderPass(vWindow.presentSwapFormat.format);
	
	vk::Pipeline graphicsPipeline = createStandardPipeline(vWindow.swapChainExtend, pipelineLayout, renderPass);
	
	vk::Framebuffer framebuffers[vWindow.presentImages.size()];
	
	for(size_t i = 0; i < vWindow.presentImages.size(); i++){
		vk::ImageView attachments[2] = {
			vWindow.presentImages[i],
			depthImageView
		};
		vk::FramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = vWindow.swapChainExtend.width;
		framebufferInfo.height = vWindow.swapChainExtend.height;
		framebufferInfo.layers = 1;
		
		VCHECKCALL(vkCreateFramebuffer(vGlobal.deviceWrapper.device, &framebufferInfo, nullptr, &framebuffers[i]), printf("Creation of Framebuffer %d failed\n", i));
		
	}
	
	BufferWrapper vertexBuffer(
		sizeof(objectStorage.vertices[0]) * objectStorage.vertices.size(), 
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, 
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	transferData(objectStorage.vertices.data(), vertexBuffer.buffer, 0, sizeof(objectStorage.vertices[0]) * objectStorage.vertices.size(), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
	//vGlobal.deviceWrapper.tqueue->waitForFinish();
	
	BufferWrapper indexBuffer(
		sizeof(objectStorage.indices[0]) * objectStorage.indices.size(), 
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, 
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	transferData(objectStorage.indices.data(), indexBuffer.buffer, 0, sizeof(objectStorage.indices[0]) * objectStorage.indices.size(), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
	//vGlobal.deviceWrapper.tqueue->waitForFinish();
	
	
	uint32_t MAX_COMMAND_COUNT = 100;
	uint32_t MAX_INSTANCE_COUNT = 100;
	
	BufferWrapper indirectCommandBuffer(sizeof(vk::DrawIndexedIndirectCommand) * MAX_COMMAND_COUNT, 
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndirectBuffer, 
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	BufferWrapper instanceBuffer(sizeof(Instance) * MAX_INSTANCE_COUNT, 
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	BufferWrapper uniformBuffer(sizeof(Camera), 
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	
	vk::DescriptorBufferInfo bufferInfo(uniformBuffer.buffer, offsetof(Camera, w2sMatrix), sizeof(glm::mat4));
	vk::WriteDescriptorSet descriptorWrite(vk::DescriptorSet(descriptorSet), 
		0, 0, //dstBinding, dstArrayElement
		1, //descriptorCount
		vk::DescriptorType::eUniformBuffer, //descriptorType
		nullptr, &bufferInfo, nullptr);//pImageInfo, pBufferInfo, pTexelBufferView
	
	vGlobal.deviceWrapper.device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
	
	vk::CommandPool commandPool = createCommandPool(vWindow.pgcQueue->graphicsQId, vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient) );
	
	vk::Semaphore imageAvailableSemaphore = createSemaphore();
	vk::Semaphore drawFinishedSemaphore = createSemaphore();
	
	vk::CommandBuffer commandBuffer = VK_NULL_HANDLE;
	
	while(vWindow.isOpen()){
		
		uint32_t stagingOffset = 0;
		
		uint32_t commandOffset = stagingOffset;
		uint32_t commandCount = 0;
		{
			uint32_t count = 0;
			for(Object& obj : objectStorage.objects){
				for(ObjectPart& part : obj.parts){
					((vk::DrawIndexedIndirectCommand*)stagingBuffer->data)[commandCount++] = {part.indexCount, obj.instances.size(), part.indexOffset, part.vertexOffset, count};
				}
				count += obj.instances.size();
			}
		}
		stagingOffset += sizeof(vk::DrawIndexedIndirectCommand) * commandCount;
		
		copyBuffer(stagingBuffer->buffer, indirectCommandBuffer.buffer, commandOffset, 0, sizeof(vk::DrawIndexedIndirectCommand)*commandCount,
			VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_HOST_WRITE_BIT, VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_HOST_WRITE_BIT);
		
		uint32_t instanceCount = 0;
		uint32_t instanceOffset = stagingOffset;
		for(Object& obj : objectStorage.objects){
			Instance* instanceArray = (Instance*)(stagingBuffer->data + stagingOffset);
			memcpy(&(instanceArray[instanceCount]), obj.instances.data(), sizeof(Instance)*obj.instances.size());
			instanceCount += obj.instances.size();
		}
		stagingOffset += sizeof(Instance) * instanceCount;
		
		copyBuffer(stagingBuffer->buffer, instanceBuffer.buffer, instanceOffset, 0, sizeof(Instance)*instanceCount,
			VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_HOST_WRITE_BIT, VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_HOST_WRITE_BIT);
		
		uint32_t uniformOffset = stagingOffset;
		viewport.m_viewvector = glm::rotate(viewport.m_viewvector, 0.005f, glm::vec3(0.0f,1.0f,0.0f));
		((Camera*)(stagingBuffer->data + stagingOffset))[0].w2sMatrix = viewport.createWorldToScreenSpaceMatrix();
		stagingOffset += sizeof(Camera);
		
		copyBuffer(stagingBuffer->buffer, uniformBuffer.buffer, uniformOffset, 0, sizeof(Camera),
			VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_HOST_WRITE_BIT, VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_HOST_WRITE_BIT);
		
		
		printf("Commands: %d - %d Instances: %d - %d Uniform: %d\n", commandCount, commandOffset, instanceCount, instanceOffset, uniformOffset);
		printf("Id: %d\n", vWindow.presentImageIndex);
		//vGlobal.deviceWrapper.tqueue->waitForFinish();
		
		if(commandBuffer)
			vGlobal.deviceWrapper.device.freeCommandBuffers(commandPool, 1, &commandBuffer);
		commandBuffer = createCommandBuffer(commandPool, vk::CommandBufferLevel::ePrimary);
		vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit), nullptr);
		
		
		vk::ClearValue clearColors[2] = {
			vk::ClearValue(vk::ClearColorValue(std::array<float,4>({0.0f, 0.0f, 0.5f, 1.0f}))),
			vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0)),
		};
		vk::RenderPassBeginInfo renderPassInfo(
			renderPass, 
			framebuffers[vWindow.presentImageIndex],
			vk::Rect2D(0,0),
			2,clearColors
			);
		{
			commandBuffer.begin(&beginInfo);
			
			vk::BufferMemoryBarrier bufferMemoryBarriers[3] = {
				vk::BufferMemoryBarrier(
					vk::AccessFlags(vk::AccessFlagBits::eTransferWrite), vk::AccessFlags(vk::AccessFlagBits::eIndirectCommandRead),
					vGlobal.deviceWrapper.transfQId, vGlobal.deviceWrapper.graphQId,
					indirectCommandBuffer.buffer,
					0, indirectCommandBuffer.bufferSize),
				vk::BufferMemoryBarrier(
					vk::AccessFlags(vk::AccessFlagBits::eTransferWrite), vk::AccessFlags(vk::AccessFlagBits::eVertexAttributeRead),
					vGlobal.deviceWrapper.transfQId, vGlobal.deviceWrapper.graphQId,
					instanceBuffer.buffer,
					0, instanceBuffer.bufferSize),
				vk::BufferMemoryBarrier(
					vk::AccessFlags(vk::AccessFlagBits::eTransferWrite), vk::AccessFlags(vk::AccessFlagBits::eUniformRead),
					vGlobal.deviceWrapper.transfQId, vGlobal.deviceWrapper.graphQId,
					uniformBuffer.buffer,
					0, uniformBuffer.bufferSize)
			};
			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer),vk::PipelineStageFlags(vk::PipelineStageFlagBits::eDrawIndirect),
				vk::DependencyFlags(),
				0, nullptr,//memoryBarrier
				1, &bufferMemoryBarriers[0],//bufferBarrier
				0, nullptr//imageBarrier
				);
			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer),vk::PipelineStageFlags(vk::PipelineStageFlagBits::eVertexInput),
				vk::DependencyFlags(),
				0, nullptr,//memoryBarrier
				1, &bufferMemoryBarriers[1],//bufferBarrier
				0, nullptr//imageBarrier
				);
			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlags(vk::PipelineStageFlagBits::eTransfer),vk::PipelineStageFlags(vk::PipelineStageFlagBits::eVertexShader),
				vk::DependencyFlags(),
				0, nullptr,//memoryBarrier
				1, &bufferMemoryBarriers[2],//bufferBarrier
				0, nullptr//imageBarrier
				);
			
			commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
			
			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
			//commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr)
			vk::Buffer vertexBuffers[] = {vertexBuffer.buffer, instanceBuffer.buffer};
			vk::DeviceSize offsets[] = {0, 0};
			commandBuffer.bindVertexBuffers(0, 2, vertexBuffers, offsets);
			commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0, vk::IndexType::eUint32);
			
			commandBuffer.drawIndexedIndirect(indirectCommandBuffer.buffer, 0, commandCount, sizeof(vk::DrawIndexedIndirectCommand));
			
			commandBuffer.endRenderPass();
			
			commandBuffer.end();
			//V_CHECKCALL(commandBuffer.end(), printf("Recording of CommandBuffer failed\n"));
		}
		
		
		vk::Semaphore waitSemaphores[] = {vWindow.imageAvailableGuardSem};
		vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
		vk::Semaphore signalSemaphores[] = {drawFinishedSemaphore};
		
		vk::SubmitInfo submitInfo(1, waitSemaphores, waitStages, 1, &commandBuffer, 1, signalSemaphores);
		
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
	
	destroySemaphore(imageAvailableSemaphore);
	destroySemaphore(drawFinishedSemaphore);
	
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
