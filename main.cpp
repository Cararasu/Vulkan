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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#include <fstream>

ObjectStorage objectStorage;

void loadDataFile (std::string file, Object* object) {

	std::ifstream input (file, std::ios::binary);

	std::string str;
	std::getline (input, str, '\0');
	uint32_t vertexCount = 0;
	input.read (reinterpret_cast<char*> (&vertexCount), sizeof (uint32_t));

	uint32_t baseVertexIndex = objectStorage.vertices.size();
	objectStorage.vertices.resize (baseVertexIndex + vertexCount);
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

		v.color = {0.0f, 1.0f, 0.0f};

		objectStorage.vertices[baseVertexIndex + i] = v;
	}
	uint32_t indexCount;
	input.read (reinterpret_cast<char*> (&indexCount), sizeof (uint32_t));

	uint32_t baseIndexIndex = objectStorage.indices.size();
	objectStorage.indices.resize (baseIndexIndex + indexCount);

	input.read (reinterpret_cast<char*> (&objectStorage.indices[baseIndexIndex]), sizeof (uint32_t) *indexCount);

	object->parts.emplace_back();
	ObjectPart& part = object->parts.back();
	part.indexCount = indexCount;
	part.indexOffset = baseIndexIndex;
	part.vertexOffset = baseVertexIndex;
}


ImageWrapper* loadImage (std::string file, vk::CommandPool commandPool, vk::Queue queue) {
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load (file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error ("failed to load texture image!");
	}
	vk::Extent3D imageExtent = vk::Extent3D (texWidth, texHeight, 1);
	vk::Format imageFormat = findSupportedFormat ({vk::Format::eR8G8B8A8Unorm}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eSampledImage);
	ImageWrapper * imageWrapper = new ImageWrapper (imageExtent, imageFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlags (vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst),
	        vk::ImageAspectFlagBits::eColor, vk::MemoryPropertyFlags (vk::MemoryPropertyFlagBits::eDeviceLocal));

	imageWrapper->transitionImageLayout (vk::ImageLayout::eTransferDstOptimal, commandPool, queue);
	transferData (pixels, imageWrapper->image, vk::Offset3D (0, 0, 0), imageExtent, imageSize,
	              vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, commandPool, queue);

	stbi_image_free (pixels);
	return imageWrapper;
}

int main (int argc, char **argv) {

	objectStorage.objects.emplace_back();
	{
		Object& obj = objectStorage.objects.back();

		loadDataFile ("../workingdir/assets/Tie_Fighter_Body.data", &obj);
		obj.parts.back().data.diffuseTexId = 0;
		loadDataFile ("../workingdir/assets/Tie_Fighter_Arm_L.data", &obj);
		obj.parts.back().data.diffuseTexId = 1;
		loadDataFile ("../workingdir/assets/Tie_Fighter_Arm_R.data", &obj);
		obj.parts.back().data.diffuseTexId = 1;
		loadDataFile ("../workingdir/assets/Tie_Fighter_Wing_L.data", &obj);
		obj.parts.back().data.diffuseTexId = 2;
		loadDataFile ("../workingdir/assets/Tie_Fighter_Wing_R.data", &obj);
		obj.parts.back().data.diffuseTexId = 2;
		//loadDataFile ("../workingdir/assets/Tie_Fighter_Windows.data", &obj);

		obj.instances.push_back ({glm::translate (glm::vec3 (2.0f, 0.0f, 0.0f)) * glm::rotate (1.0f, glm::vec3 (0.0f, 1.0f, 0.0f)) });
		obj.instances.push_back ({glm::translate (glm::vec3 (0.0f, 0.0f, 0.0f)) });
		obj.instances.push_back ({glm::translate (glm::vec3 (-1.0f, -1.0f, -2.0f)) });
		obj.instances.push_back ({glm::translate (glm::vec3 (-2.0f, 0.0f, 0.0f)) });
	}

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

	vWindow->initializeWindow();

	global.buildStandardPipeline (vWindow->presentSwapFormat.format, vWindow->swapChainExtend);

	vk::DescriptorPool descriptorSetPool = createStandardDescriptorSetPool();

	vk::Format depthFormat = findDepthFormat();
	vk::Extent3D extent (vWindow->swapChainExtend.width, vWindow->swapChainExtend.height, 1);
	ImageWrapper *depthImage = new ImageWrapper (extent, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlags (vk::ImageUsageFlagBits::eDepthStencilAttachment),
	        vk::ImageAspectFlagBits::eDepth, vk::MemoryPropertyFlags (vk::MemoryPropertyFlagBits::eDeviceLocal));
	vk::ImageView depthImageView = createImageView2D (depthImage->image, depthFormat, vk::ImageAspectFlags (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil));
	transitionImageLayout (depthImage->image, depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal,
	                       vk::ImageAspectFlags (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil), vWindow->getCurrentGraphicsCommandPool(), vWindow->pgcQueue->graphicsQueue);

	for (size_t i = 0; i < vWindow->perPresentImageDatas.size(); i++) {
		vk::ImageView attachments[2] = {
			vWindow->perPresentImageDatas[i].presentImageView,
			depthImageView
		};
		vk::FramebufferCreateInfo framebufferInfo (vk::FramebufferCreateFlags(), global.renderpass.standardRenderPass, 2, attachments, vWindow->swapChainExtend.width, vWindow->swapChainExtend.height, 1);
		vWindow->perPresentImageDatas[i].framebuffer = global.deviceWrapper.device.createFramebuffer (framebufferInfo, nullptr);
	}

	std::vector<vk::DescriptorSet> descriptorSets1 = createDescriptorSets (descriptorSetPool, &global.descriptorsetlayout.standardDescriptorSetLayouts);
	std::vector<vk::DescriptorSet> descriptorSets2 = createDescriptorSets (descriptorSetPool, &global.descriptorsetlayout.standardDescriptorSetLayouts);
	std::vector<vk::DescriptorSet> descriptorSets3 = createDescriptorSets (descriptorSetPool, &global.descriptorsetlayout.standardDescriptorSetLayouts);

	vk::CommandPool transferCommandPool = createTransferCommandPool (vk::CommandPoolCreateFlagBits::eTransient);

	BufferWrapper *vertexBuffer = new BufferWrapper (sizeof (objectStorage.vertices[0]) * objectStorage.vertices.size(),
	        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
	transferData (objectStorage.vertices.data(), vertexBuffer->buffer, 0, sizeof (objectStorage.vertices[0]) * objectStorage.vertices.size(),
	              vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, transferCommandPool, global.deviceWrapper.tqueue->transferQueue);
	//global.deviceWrapper.tqueue->waitForFinish();

	BufferWrapper *indexBuffer = new BufferWrapper (sizeof (objectStorage.indices[0]) * objectStorage.indices.size(),
	        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
	transferData (objectStorage.indices.data(), indexBuffer->buffer, 0, sizeof (objectStorage.indices[0]) * objectStorage.indices.size(),
	              vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, transferCommandPool, global.deviceWrapper.tqueue->transferQueue);

	ImageWrapper *fighterBodyImage = loadImage ("../workingdir/assets/Tie_Fighter_Body_Diffuse.png", transferCommandPool, global.deviceWrapper.tqueue->transferQueue);
	ImageWrapper *fighterArmImage = loadImage ("../workingdir/assets/Tie_Fighter_Arm_Diffuse.png", transferCommandPool, global.deviceWrapper.tqueue->transferQueue);
	ImageWrapper *fighterWingImage = loadImage ("../workingdir/assets/Tie_Fighter_Wing_Diffuse.png", transferCommandPool, global.deviceWrapper.tqueue->transferQueue);

	//TODO Memory Barrier for graphics queue
	global.deviceWrapper.tqueue->waitForFinish();
	vWindow->pgcQueue->waitForFinish();

	fighterBodyImage->transitionImageLayout (vk::ImageLayout::eShaderReadOnlyOptimal, vWindow->getCurrentGraphicsCommandPool(), vWindow->pgcQueue->graphicsQueue);
	fighterArmImage->transitionImageLayout (vk::ImageLayout::eShaderReadOnlyOptimal, vWindow->getCurrentGraphicsCommandPool(), vWindow->pgcQueue->graphicsQueue);
	fighterWingImage->transitionImageLayout (vk::ImageLayout::eShaderReadOnlyOptimal, vWindow->getCurrentGraphicsCommandPool(), vWindow->pgcQueue->graphicsQueue);

	vk::ImageView fighterBodyImageView = createImageView2D (fighterBodyImage->image, fighterBodyImage->format, fighterBodyImage->aspectFlags);
	vk::ImageView fighterArmImageView = createImageView2D (fighterArmImage->image, fighterArmImage->format, fighterArmImage->aspectFlags);
	vk::ImageView fighterWingImageView = createImageView2D (fighterWingImage->image, fighterWingImage->format, fighterWingImage->aspectFlags);

	vk::Sampler sampler;
	{
		vk::SamplerCreateInfo samplerInfo (
		    vk::SamplerCreateFlags(), vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
		    vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
		    0.0f,
		    VK_TRUE, 16, //anisotrophic
		    VK_FALSE, vk::CompareOp::eAlways, //compare
		    0.0f, 0.0f, //lod
		    vk::BorderColor::eFloatOpaqueBlack, VK_FALSE
		);

		global.deviceWrapper.device.createSampler (&samplerInfo, nullptr, &sampler);
	}


	uint32_t MAX_COMMAND_COUNT = 100;
	uint32_t MAX_INSTANCE_COUNT = 100;

	BufferWrapper *indirectCommandBuffer = new BufferWrapper (sizeof (vk::DrawIndexedIndirectCommand) * MAX_COMMAND_COUNT,
	        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndirectBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
	BufferWrapper *instanceBuffer = new BufferWrapper (sizeof (Instance) * MAX_INSTANCE_COUNT,
	        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
	BufferWrapper *partInstanceBuffer = new BufferWrapper (sizeof (ObjectPartData) * MAX_INSTANCE_COUNT,
	        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
	BufferWrapper *uniformBuffer = new BufferWrapper (sizeof (Camera),
	        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);


	vk::DescriptorBufferInfo bufferInfo (uniformBuffer->buffer, offsetof (Camera, w2sMatrix), sizeof (glm::mat4));
	{
		vk::DescriptorImageInfo sampleInfo = vk::DescriptorImageInfo (sampler);
		vk::DescriptorImageInfo imageSetInfo1 = vk::DescriptorImageInfo (vk::Sampler(), fighterBodyImageView, fighterArmImage->layout);
		vk::DescriptorImageInfo imageSetInfo2 = vk::DescriptorImageInfo (vk::Sampler(), fighterArmImageView, fighterArmImage->layout);
		vk::DescriptorImageInfo imageSetInfo3 = vk::DescriptorImageInfo (vk::Sampler(), fighterWingImageView, fighterArmImage->layout);
		global.deviceWrapper.device.updateDescriptorSets ({
			vk::WriteDescriptorSet (descriptorSets1[0],
			                        0, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eUniformBuffer, //descriptorType
			                        nullptr, &bufferInfo, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
			vk::WriteDescriptorSet (descriptorSets1[1],
			                        0, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eSampler, //descriptorType
			                        &sampleInfo, nullptr, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
			vk::WriteDescriptorSet (descriptorSets1[1],
			                        1, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eSampledImage, //descriptorType
			                        &imageSetInfo1, nullptr, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
			vk::WriteDescriptorSet (descriptorSets2[0],
			                        0, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eUniformBuffer, //descriptorType
			                        nullptr, &bufferInfo, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
			vk::WriteDescriptorSet (descriptorSets2[1],
			                        0, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eSampler, //descriptorType
			                        &sampleInfo, nullptr, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
			vk::WriteDescriptorSet (descriptorSets2[1],
			                        1, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eSampledImage, //descriptorType
			                        &imageSetInfo2, nullptr, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
			vk::WriteDescriptorSet (descriptorSets3[0],
			                        0, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eUniformBuffer, //descriptorType
			                        nullptr, &bufferInfo, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
			vk::WriteDescriptorSet (descriptorSets3[1],
			                        0, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eSampler, //descriptorType
			                        &sampleInfo, nullptr, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
			vk::WriteDescriptorSet (descriptorSets3[1],
			                        1, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eSampledImage, //descriptorType
			                        &imageSetInfo3, nullptr, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
		}, {});

	}

	vk::Semaphore imageAvailableSemaphore = createSemaphore();
	vk::Semaphore drawFinishedSemaphore = createSemaphore();

	while (vWindow->isOpen()) {
		printf ("--------------- FrameBoundary ---------------\n");
		printf ("PresetImageId: %d\n", vWindow->presentImageIndex);

		viewport.m_viewvector = glm::rotate (viewport.m_viewvector, 0.005f, viewport.m_upvector);

		glm::vec3 rotVec = glm::cross (viewport.m_viewvector, viewport.m_upvector);
		viewport.m_upvector = glm::rotate (viewport.m_upvector, 0.005f, rotVec);
		viewport.m_viewvector = glm::rotate (viewport.m_viewvector, 0.005f, rotVec);
		viewport.m_upvector = glm::rotate (viewport.m_upvector, 0.005f, viewport.m_viewvector);


		uint32_t stagingOffset = 0;

		uint32_t commandOffset = stagingOffset;
		uint32_t commandCount = 0;
		{
			uint32_t count = 0;
			for (Object& obj : objectStorage.objects) {
				for (ObjectPart& part : obj.parts) {
					( (vk::DrawIndexedIndirectCommand*) stagingBuffer->data) [commandCount++] = vk::DrawIndexedIndirectCommand (part.indexCount, obj.instances.size(), part.indexOffset, part.vertexOffset, count);
				}
				count += obj.instances.size();
			}
		}
		stagingOffset += sizeof (vk::DrawIndexedIndirectCommand) * commandCount;

		copyBuffer (stagingBuffer->buffer, indirectCommandBuffer->buffer, commandOffset, 0, sizeof (vk::DrawIndexedIndirectCommand) *commandCount,
		            vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite,
		            vWindow->getCurrentGraphicsCommandPool(), vWindow->pgcQueue->graphicsQueue);

		uint32_t instanceCount = 0;
		uint32_t instanceOffset = stagingOffset;
		for (Object& obj : objectStorage.objects) {
			Instance* instanceArray = (Instance*) (stagingBuffer->data + stagingOffset);
			memcpy (& (instanceArray[instanceCount]), obj.instances.data(), sizeof (Instance) *obj.instances.size());
			instanceCount += obj.instances.size();
		}
		stagingOffset += sizeof (Instance) * instanceCount;

		copyBuffer (stagingBuffer->buffer, instanceBuffer->buffer, instanceOffset, 0, sizeof (Instance) *instanceCount,
		            vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite,
		            vWindow->getCurrentGraphicsCommandPool(), vWindow->pgcQueue->graphicsQueue);

		uint32_t partInstanceCount = 0;
		uint32_t partInstanceOffset = stagingOffset;
		for (Object& obj : objectStorage.objects) {
			for (ObjectPart& part : obj.parts) {
				( (ObjectPartData*) (stagingBuffer->data + stagingOffset)) [partInstanceCount++] = part.data;
			}
		}
		stagingOffset += sizeof (ObjectPartData) * partInstanceCount;

		copyBuffer (stagingBuffer->buffer, partInstanceBuffer->buffer, partInstanceOffset, 0, sizeof (ObjectPartData) *partInstanceCount,
		            vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite,
		            vWindow->getCurrentGraphicsCommandPool(), vWindow->pgcQueue->graphicsQueue);

		uint32_t uniformOffset = stagingOffset;
		( (Camera*) (stagingBuffer->data + stagingOffset)) [0].w2sMatrix = viewport.createWorldToScreenSpaceMatrix();
		stagingOffset += sizeof (Camera);

		copyBuffer (stagingBuffer->buffer, uniformBuffer->buffer, uniformOffset, 0, sizeof (Camera),
		            vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite,
		            vWindow->getCurrentGraphicsCommandPool(), vWindow->pgcQueue->graphicsQueue);


		vk::CommandBuffer commandBuffer = createCommandBuffer (vWindow->getCurrentGraphicsCommandPool(), vk::CommandBufferLevel::ePrimary);
		vk::CommandBufferBeginInfo beginInfo (vk::CommandBufferUsageFlags (vk::CommandBufferUsageFlagBits::eOneTimeSubmit), nullptr);

		{
			commandBuffer.begin (&beginInfo);

			commandBuffer.pipelineBarrier (
			    vk::PipelineStageFlags (vk::PipelineStageFlagBits::eTransfer), vk::PipelineStageFlags (vk::PipelineStageFlagBits::eDrawIndirect),
			    vk::DependencyFlags(),
			{}/*memoryBarrier*/, {
				vk::BufferMemoryBarrier (
				    vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eIndirectCommandRead,
				    global.deviceWrapper.transfQId, global.deviceWrapper.graphQId,
				    indirectCommandBuffer->buffer, 0, indirectCommandBuffer->bufferSize
				)
			}/*bufferBarrier*/,
			{}/*imageBarrier*/
			);
			commandBuffer.pipelineBarrier (
			    vk::PipelineStageFlags (vk::PipelineStageFlagBits::eTransfer), vk::PipelineStageFlags (vk::PipelineStageFlagBits::eVertexInput),
			    vk::DependencyFlags(),
			{}/*memoryBarrier*/, {
				vk::BufferMemoryBarrier (
				    vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eVertexAttributeRead,
				    global.deviceWrapper.transfQId, global.deviceWrapper.graphQId,
				    instanceBuffer->buffer, 0, instanceBuffer->bufferSize
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
			        global.renderpass.standardRenderPass,
			        vWindow->perPresentImageDatas[vWindow->presentImageIndex].framebuffer,
			        vk::Rect2D (vk::Offset2D (0, 0), vWindow->swapChainExtend),
			        2, clearColors
			    ),
			    vk::SubpassContents::eInline
			);

			commandBuffer.bindPipeline (vk::PipelineBindPoint::eGraphics, global.pipeline.standardPipeline);

			commandBuffer.bindVertexBuffers (0, {vertexBuffer->buffer, instanceBuffer->buffer}, {0, 0});
			commandBuffer.bindIndexBuffer (indexBuffer->buffer, 0, vk::IndexType::eUint32);

			commandBuffer.bindDescriptorSets (vk::PipelineBindPoint::eGraphics, global.pipelinelayout.standardPipelineLayout, 0, descriptorSets1, {});

			commandBuffer.drawIndexedIndirect (indirectCommandBuffer->buffer, 0, 1, sizeof (vk::DrawIndexedIndirectCommand));

			commandBuffer.bindDescriptorSets (vk::PipelineBindPoint::eGraphics, global.pipelinelayout.standardPipelineLayout, 0, descriptorSets2, {});

			commandBuffer.drawIndexedIndirect (indirectCommandBuffer->buffer, sizeof (vk::DrawIndexedIndirectCommand), 2, sizeof (vk::DrawIndexedIndirectCommand));

			commandBuffer.bindDescriptorSets (vk::PipelineBindPoint::eGraphics, global.pipelinelayout.standardPipelineLayout, 0, descriptorSets3, {});

			commandBuffer.drawIndexedIndirect (indirectCommandBuffer->buffer, sizeof (vk::DrawIndexedIndirectCommand) * 3, 2, sizeof (vk::DrawIndexedIndirectCommand));

			commandBuffer.endRenderPass();

			V_CHECKCALL_MAYBE (commandBuffer.end(), printf ("Recording of CommandBuffer failed\n"));
		}


		vk::Semaphore waitSemaphores[] = {vWindow->imageAvailableGuardSem};
		vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
		vk::Semaphore signalSemaphores[] = {drawFinishedSemaphore};

		vk::SubmitInfo submitInfo (1, waitSemaphores, waitStages, 1, &commandBuffer, 1, signalSemaphores);

		vWindow->pgcQueue->submitGraphics (1, &submitInfo);

		vWindow->showNextImage (1, signalSemaphores);
		glfwPollEvents();

		std::this_thread::sleep_for (std::chrono::nanoseconds (10000000));
	}


	global.deviceWrapper.device.waitIdle();


	V_CHECKCALL_MAYBE (vWindow->pgcQueue->presentQueue.waitIdle(), printf ("Failed to wait for Present-Queue\n"));
	V_CHECKCALL_MAYBE (global.deviceWrapper.device.waitIdle(), printf ("Failed to wait for Device\n"));


	global.deviceWrapper.device.destroyCommandPool (transferCommandPool);

	destroySemaphore (imageAvailableSemaphore);
	destroySemaphore (drawFinishedSemaphore);

	delete vWindow;
	global.deviceWrapper.device.destroyImageView (depthImageView, nullptr);
	delete depthImage;
	if (stagingBuffer)
		delete stagingBuffer;

	global.deviceWrapper.device.destroyImageView (fighterArmImageView, nullptr);
	global.deviceWrapper.device.destroyImageView (fighterBodyImageView, nullptr);
	global.deviceWrapper.device.destroyImageView (fighterWingImageView, nullptr);

	delete fighterArmImage;
	delete fighterBodyImage;
	delete fighterWingImage;


	delete vertexBuffer;
	delete indexBuffer;

	delete indirectCommandBuffer;
	delete instanceBuffer;
	delete uniformBuffer;

	global.deviceWrapper.device.destroyDescriptorPool (descriptorSetPool, nullptr);

	glfwTerminate();
	global.terminate();

	return 0;
}
