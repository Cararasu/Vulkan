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
#include "Dispatcher.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#include <fstream>

void loadDataFile (std::string file, OpaqueObjectDispatcher* dispatcher) {

	std::ifstream input (file, std::ios::binary);

	std::string str;
	std::getline (input, str, '\0');
	uint32_t vertexCount = 0;
	input.read (reinterpret_cast<char*> (&vertexCount), sizeof (uint32_t));

	std::vector<Vertex> vs;
	vs.resize (vertexCount);
	for (size_t i = 0; i < vertexCount; i++) {
		Vertex v;
		input.read (reinterpret_cast<char*> (&v.pos[0]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.uv[0]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.normal[0]), sizeof (float));

		input.read (reinterpret_cast<char*> (&v.pos[1]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.uv[1]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.normal[1]), sizeof (float));

		input.read (reinterpret_cast<char*> (&v.pos[2]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.uv[2]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.normal[2]), sizeof (float));

		vs[i] = v;
	}
	uint32_t indexCount;
	input.read (reinterpret_cast<char*> (&indexCount), sizeof (uint32_t));

	std::vector<uint32_t> is;
	is.resize (indexCount);

	input.read (reinterpret_cast<char*> (is.data()), sizeof (uint32_t) *indexCount);
	
	ObjectPartData data;
	data.diffuseTexId = 0;
	dispatcher->add_object(vs, is, data);
}


void loadImage (std::string file, ImageWrapper * imageWrapper, uint32_t index, vk::CommandPool commandPool, vk::Queue queue) {
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load (file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error ("failed to load texture image!");
	}
	vk::Extent3D imageExtent = vk::Extent3D (texWidth, texHeight, 1);
	transferData (pixels, imageWrapper->image, vk::Offset3D (0, 0, 0), imageExtent, index, imageSize,
	              vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, commandPool, queue);

	stbi_image_free (pixels);
}

int main (int argc, char **argv) {
	
	
	OpaqueObjectDispatcher* dispatcher = new OpaqueObjectDispatcher();
	
	std::vector<uint32_t> TiePartIds = {0,1,2,3,4,5};
	loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Body.data", dispatcher);
	dispatcher->parts.back().data.diffuseTexId = 0;
	loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Arm_L.data", dispatcher);
	dispatcher->parts.back().data.diffuseTexId = 1;
	loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Arm_R.data", dispatcher);
	dispatcher->parts.back().data.diffuseTexId = 1;
	loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Wing_L.data", dispatcher);
	dispatcher->parts.back().data.diffuseTexId = 2;
	loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Wing_R.data", dispatcher);
	dispatcher->parts.back().data.diffuseTexId = 2;
	loadDataFile ("../workingdir/assets/Tie_Fighter_Windows.data", dispatcher);
	dispatcher->parts.back().data.diffuseTexId = 2;
	
	std::vector<uint32_t> XPartIds = {6,7,8,9,10,11};
	loadDataFile ("../workingdir/assets/X/XWing_Body.data", dispatcher);
	dispatcher->parts.back().data.diffuseTexId = 3;
	loadDataFile ("../workingdir/assets/X/XWing_Windows.data", dispatcher);
	dispatcher->parts.back().data.diffuseTexId = 3;
	loadDataFile ("../workingdir/assets/X/XWing_Wing_LB.data", dispatcher);
	dispatcher->parts.back().data.diffuseTexId = 3;
	loadDataFile ("../workingdir/assets/X/XWing_Wing_LT.data", dispatcher);
	dispatcher->parts.back().data.diffuseTexId = 3;
	loadDataFile ("../workingdir/assets/X/XWing_Wing_RB.data", dispatcher);
	dispatcher->parts.back().data.diffuseTexId = 3;
	loadDataFile ("../workingdir/assets/X/XWing_Wing_RT.data", dispatcher);
	dispatcher->parts.back().data.diffuseTexId = 3;

	PerspectiveViewPort<float> viewport;

	viewport.m_viewvector = {0.0f, 0.2f, -1.4f};
	viewport.m_focalpoint = {0.0f, 0.0f, 0.0f};
	viewport.m_upvector = {0.0f, 1.0f, 0.0f};
	viewport.m_distance = 2.0f;
	viewport.m_near = 0.01f;
	viewport.m_far = 100.0f;
	viewport.m_aspect = 1.0f;

	printf ("hello world\n");

	glfwInit();

	global.preInitialize();

	printf ("Instance Extensions:\n");
	for (vk::ExtensionProperties& extProp : global.instExtLayers.availableExtensions) {
		printf ("\t%s\n", extProp.extensionName);
	}
	printf ("Instance Layers:\n");
	for (vk::LayerProperties& layerProp : global.instExtLayers.availableLayers) {
		printf ("\t%s\n", layerProp.layerName);
		printf ("\t\tDesc: %s\n", layerProp.description);
	}

	if (!global.instExtLayers.activateLayer ("VK_LAYER_LUNARG_standard_validation")) {
		printf ("Layer VK_LAYER_LUNARG_standard_validation not available\n");
	}
	if (!global.instExtLayers.activateLayer ("VK_LAYER_LUNARG_swapchain")) {
		printf ("Layer VK_LAYER_LUNARG_swapchain not available\n");
	}

	/*if(!global.instExtLayers.activateLayer("VK_LAYER_RENDERDOC_Capture")){
		printf("Extension VK_LAYER_RENDERDOC_Capture not available\n");
	}
	if(!global.instExtLayers.activateLayer("VK_LAYER_LUNARG_api_dump")){
		printf("Extension VK_LAYER_LUNARG_api_dump not available\n");
	}*/
	uint32_t instanceExtCount;
	const char** glfwReqInstanceExt = glfwGetRequiredInstanceExtensions (&instanceExtCount);
	for (size_t i = 0; i < instanceExtCount; i++) {
		if (!global.instExtLayers.activateExtension (glfwReqInstanceExt[i])) {
			printf ("Extension %s not available\n", glfwReqInstanceExt[i]);
		}
	}
	if (!global.instExtLayers.activateExtension (VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
		printf ("Extension %s not available\n", VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	printf ("Instance Extensions available:\n");
	for (vk::ExtensionProperties& prop : global.instExtLayers.availableExtensions) {
		printf ("\t%s\n", prop.extensionName);
	}
	printf ("Instance Layers available:\n");
	for (vk::LayerProperties& prop : global.instExtLayers.availableLayers) {
		printf ("\t%s\n", prop.layerName);
	}

	global.initializeInstance ("Blabla", "wuwu Engine");

	global.choseBestDevice();

	printf ("Device Extensions available:\n");
	for (vk::ExtensionProperties& prop : global.devExtLayers.availableExtensions) {
		printf ("\t%s\n", prop.extensionName);
	}
	printf ("Device Layers available:\n");
	for (vk::LayerProperties& prop : global.devExtLayers.availableLayers) {
		printf ("\t%s\n", prop.layerName);
	}

	if (!global.devExtLayers.activateExtension (VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
		printf ("Extension %s not available\n", VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}
	if (!global.devExtLayers.activateExtension (VK_NV_GLSL_SHADER_EXTENSION_NAME)) {
		printf ("Extension %s not available\n", VK_NV_GLSL_SHADER_EXTENSION_NAME);
	}
	//global.devExtLayers.activateExtension(VK_NV_GLSL_SHADER_EXTENSION_NAME);

	global.initializeDevice();

	VWindow* vWindow = new VWindow();

	global.pipeline_module_layouts.standard = createPipelineModuleLayout(&global.pipeline_module_builders.standard);

	vWindow->initializeWindow();


	vk::DescriptorPool descriptorSetPool = createStandardDescriptorSetPool();

	std::vector<vk::DescriptorSet> descriptorSets = createDescriptorSets (descriptorSetPool, &global.pipeline_module_layouts.standard.descriptorSetLayouts);

	dispatcher->set_descriptor_set(descriptorSets[1]);

	vk::CommandPool transferCommandPool = createTransferCommandPool (vk::CommandPoolCreateFlagBits::eTransient);

	dispatcher->upload_data(transferCommandPool, global.deviceWrapper.tqueue->transferQueue);

	VkExtent3D imageExtent = {4096, 4096, 1};
	vk::Format imageFormat = findSupportedFormat ({vk::Format::eR8G8B8A8Unorm}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eSampledImage);
	ImageWrapper * imageWrapper = new ImageWrapper (imageExtent, 12, 4, imageFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlags (vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc),
	        vk::ImageAspectFlagBits::eColor, vk::MemoryPropertyFlags (vk::MemoryPropertyFlagBits::eDeviceLocal));
	{
		imageWrapper->transitionImageLayout (vk::ImageLayout::eTransferDstOptimal, vWindow->getCurrentGraphicsCommandPool(), vWindow->pgcQueue->graphicsQueue);

		loadImage ("../workingdir/assets/Tie/Tie_Fighter_Body_Diffuse.png", imageWrapper, 0, transferCommandPool, global.deviceWrapper.tqueue->transferQueue);
		loadImage ("../workingdir/assets/Tie/Tie_Fighter_Arm_Diffuse.png", imageWrapper, 1, transferCommandPool, global.deviceWrapper.tqueue->transferQueue);
		loadImage ("../workingdir/assets/Tie/Tie_Fighter_Wing_Diffuse.png", imageWrapper, 2, transferCommandPool, global.deviceWrapper.tqueue->transferQueue);
		loadImage ("../workingdir/assets/X/XWing_Diffuse.png", imageWrapper, 3, transferCommandPool, global.deviceWrapper.tqueue->transferQueue);
		
		imageWrapper->generateMipmaps(0, vk::ImageLayout::eShaderReadOnlyOptimal, vWindow->getCurrentGraphicsCommandPool(), vWindow->pgcQueue->graphicsQueue);
		
		//imageWrapper->transitionImageLayout (vk::ImageLayout::eShaderReadOnlyOptimal, vWindow->getCurrentGraphicsCommandPool(), vWindow->pgcQueue->graphicsQueue);
	}

	//TODO Memory Barrier for graphics queue
	//global.deviceWrapper.tqueue->waitForFinish();
	//vWindow->pgcQueue->waitForFinish();

	vk::Sampler sampler;
	{
		vk::SamplerCreateInfo samplerInfo (
		    vk::SamplerCreateFlags(), vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
		    vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
		    0.0f,
		    VK_TRUE, 16, //anisotrophic
		    VK_FALSE, vk::CompareOp::eAlways, //compare
		    0.0f, 12.0f, //lod
		    vk::BorderColor::eFloatOpaqueBlack, VK_FALSE
		);

		global.deviceWrapper.device.createSampler (&samplerInfo, nullptr, &sampler);
	}

	dispatcher->set_image_array(imageWrapper, sampler);

	uint32_t MAX_COMMAND_COUNT = 100;

	BufferWrapper *uniformBuffer = new BufferWrapper (sizeof (Camera),
	        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);


	vk::DescriptorBufferInfo bufferInfo (uniformBuffer->buffer, offsetof (Camera, w2sMatrix), sizeof (glm::mat4));
	{
		global.deviceWrapper.device.updateDescriptorSets ({
			vk::WriteDescriptorSet (descriptorSets[0],
			                        0, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eUniformBuffer, //descriptorType
			                        nullptr, &bufferInfo, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
		}, {});

	}

	vk::Semaphore imageAvailableSemaphore = createSemaphore();
	vk::Semaphore drawFinishedSemaphore = createSemaphore();

	while (vWindow->isOpen()) {
		
		vWindow->setupFrame();
		printf ("--------------- FrameBoundary ---------------\n");
		printf ("PresetImageId: %d\n", vWindow->presentImageIndex);

		viewport.m_viewvector = glm::rotate (viewport.m_viewvector, 0.005f, viewport.m_upvector);
		
		/*
		glm::vec3 rotVec = glm::cross (viewport.m_viewvector, viewport.m_upvector);
		viewport.m_upvector = glm::rotate (viewport.m_upvector, 0.005f, rotVec);
		viewport.m_viewvector = glm::rotate (viewport.m_viewvector, 0.005f, rotVec);
		viewport.m_upvector = glm::rotate (viewport.m_upvector, 0.005f, viewport.m_viewvector);*/

		Instance inst1 = {glm::translate(glm::vec3(1.0f, 1.0f, 1.0f))};
		Instance inst2 = {glm::translate(glm::vec3(0.0f, 0.0f, 0.0f))};
		Instance inst3 = {glm::translate(glm::vec3(-3.0f, 0.0f, 0.0f))};
		dispatcher->reset_instances();
		dispatcher->push_instance(TiePartIds, inst1);
		dispatcher->push_instance(TiePartIds, inst3);
		
		dispatcher->push_instance(XPartIds, inst2);
		
		vk::CommandBuffer commandBuffer = createCommandBuffer (vWindow->getCurrentGraphicsCommandPool(), vk::CommandBufferLevel::ePrimary);
		vk::CommandBufferBeginInfo beginInfo (vk::CommandBufferUsageFlags (vk::CommandBufferUsageFlagBits::eOneTimeSubmit), nullptr);
		
		{
			commandBuffer.begin (&beginInfo);
			
			uint32_t stagingOffset = 0;

			uint32_t uniformOffset = stagingOffset;
			( (Camera*) (stagingBuffer->data + stagingOffset)) [0].w2sMatrix = viewport.createWorldToScreenSpaceMatrix();
			stagingOffset += sizeof (Camera);

			copyBuffer (stagingBuffer->buffer, uniformBuffer->buffer, uniformOffset, 0, sizeof (Camera),
						vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite,
						commandBuffer);
			
			stagingOffset = dispatcher->setup(stagingBuffer, stagingOffset, commandBuffer);
			
			commandBuffer.pipelineBarrier (
				vk::PipelineStageFlags (vk::PipelineStageFlagBits::eTransfer), vk::PipelineStageFlags (vk::PipelineStageFlagBits::eVertexInput),
				vk::DependencyFlags(),
			{}/*memoryBarrier*/, {
				vk::BufferMemoryBarrier (
					vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eVertexAttributeRead,
					global.deviceWrapper.transfQId, global.deviceWrapper.graphQId,
					dispatcher->instanceBuffer->buffer, 0, dispatcher->instanceBuffer->bufferSize
				)
			}/*bufferBarrier*/,
			{}/*imageBarrier*/
			);
			commandBuffer.pipelineBarrier (
			    vk::PipelineStageFlags (vk::PipelineStageFlagBits::eTransfer), vk::PipelineStageFlags (vk::PipelineStageFlagBits::eVertexShader),
			    vk::DependencyFlags(),
			{}/*memoryBarrier*/, {
				vk::BufferMemoryBarrier (
				    vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eUniformRead,
				    global.deviceWrapper.transfQId, global.deviceWrapper.graphQId,
				    uniformBuffer->buffer, 0, uniformBuffer->bufferSize
				)
			}/*bufferBarrier*/,
			{}/*imageBarrier*/
			);
			vk::ClearValue clearColors[2] = {
				vk::ClearValue (vk::ClearColorValue (std::array<float, 4> ({0.0f, 0.0f, 0.5f, 1.0f}))),
				vk::ClearValue (vk::ClearDepthStencilValue (1.0f, 0)),
			};
			commandBuffer.beginRenderPass (
			    vk::RenderPassBeginInfo (
					vWindow->standardmodule.renderPass,
			        vWindow->perPresentImageDatas[vWindow->presentImageIndex].framebuffer,
			        vk::Rect2D (vk::Offset2D (0, 0), vWindow->swapChainExtend),
			        2, clearColors
			    ),
			    vk::SubpassContents::eInline
			);
			
			commandBuffer.bindPipeline (vk::PipelineBindPoint::eGraphics, vWindow->standardmodule.pipeline);
			commandBuffer.bindDescriptorSets (vk::PipelineBindPoint::eGraphics, global.pipeline_module_layouts.standard.pipelineLayout, 0, descriptorSets[0], {});
			
			dispatcher->dispatch(commandBuffer);

			commandBuffer.endRenderPass();

			V_CHECKCALL_MAYBE (commandBuffer.end(), printf ("Recording of CommandBuffer failed\n"));
		}

		vk::Semaphore waitSemaphores[] = {vWindow->imageAvailableGuardSem};
		vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
		vk::Semaphore signalSemaphores[] = {drawFinishedSemaphore};

		vk::SubmitInfo submitInfo (1, waitSemaphores, waitStages, 1, &commandBuffer, 1, signalSemaphores);

		vWindow->pgcQueue->submitGraphics (1, &submitInfo);

		vWindow->showNextFrame (1, signalSemaphores);
		printf ("---------------   EndFrame    ---------------\n");
		glfwPollEvents();

		std::this_thread::sleep_for (std::chrono::nanoseconds (10000000));
	}

	global.deviceWrapper.device.waitIdle();

	V_CHECKCALL_MAYBE (vWindow->pgcQueue->presentQueue.waitIdle(), printf ("Failed to wait for Present-Queue\n"));
	V_CHECKCALL_MAYBE (global.deviceWrapper.device.waitIdle(), printf ("Failed to wait for Device\n"));

	global.deviceWrapper.device.destroyCommandPool (transferCommandPool);

	destroySemaphore (imageAvailableSemaphore);
	destroySemaphore (drawFinishedSemaphore);

	global.deviceWrapper.device.destroySampler(sampler);

	delete vWindow;
	if (stagingBuffer)
		delete stagingBuffer;

	delete imageWrapper;
	
	delete dispatcher;
	
	delete uniformBuffer;

	global.deviceWrapper.device.destroyDescriptorPool (descriptorSetPool, nullptr);

	glfwTerminate();
	global.terminate();

	return 0;
}
