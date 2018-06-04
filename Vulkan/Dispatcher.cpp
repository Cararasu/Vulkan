#include "Dispatcher.h"
#include "VGlobal.h"


OpaqueObjectDispatcher::OpaqueObjectDispatcher () {
}
OpaqueObjectDispatcher::~OpaqueObjectDispatcher() {
}
uint32_t OpaqueObjectDispatcher::add_object (std::vector<Vertex>& vertices_, std::vector<uint32_t>& indices_, ObjectPartData& part_) {
	uint32_t id = this->parts.size();

	this->parts.push_back ({0, part_, this->indices.size(), indices_.size(), this->vertices.size() });
	this->vertices.insert (this->vertices.end(), vertices_.begin(), vertices_.end());
	this->indices.insert (this->indices.end(), indices_.begin(), indices_.end());
	return id;
}
void OpaqueObjectDispatcher::set_image_array (ImageWrapper* imageWrapper, vk::Sampler sampler) {
	this->sampler = sampler;
	this->image = imageWrapper;
	this->imageView = instance->createImageView2DArray (imageWrapper->image, 0, imageWrapper->mipMapLevels, 0, imageWrapper->arraySize, imageWrapper->format, imageWrapper->aspectFlags);
	if (desciptorSet) {
		vk::DescriptorImageInfo sampleInfo = vk::DescriptorImageInfo (sampler);
		vk::DescriptorImageInfo imageSetInfo = vk::DescriptorImageInfo (vk::Sampler(), imageView, imageWrapper->layout);
		instance->device.updateDescriptorSets ({
			vk::WriteDescriptorSet (desciptorSet,
			                        0, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eSampler, //descriptorType
			                        &sampleInfo, nullptr, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
			vk::WriteDescriptorSet (desciptorSet,
			                        1, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eSampledImage, //descriptorType
			                        &imageSetInfo, nullptr, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
		}, {});
	}
}
void OpaqueObjectDispatcher::set_descriptor_set (vk::DescriptorSet desciptorSet) {
	this->desciptorSet = desciptorSet;
	if (sampler && imageView) {
		vk::DescriptorImageInfo sampleInfo = vk::DescriptorImageInfo (sampler);
		vk::DescriptorImageInfo imageSetInfo = vk::DescriptorImageInfo (vk::Sampler(), imageView, image->layout);
		instance->device.updateDescriptorSets ({
			vk::WriteDescriptorSet (desciptorSet,
			                        0, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eSampler, //descriptorType
			                        &sampleInfo, nullptr, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
			vk::WriteDescriptorSet (desciptorSet,
			                        1, 0, //dstBinding, dstArrayElement
			                        1, //descriptorCount
			                        vk::DescriptorType::eSampledImage, //descriptorType
			                        &imageSetInfo, nullptr, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
		}, {});
	}
}
void OpaqueObjectDispatcher::upload_data (vk::CommandPool commandPool, vk::Queue submitQueue) {

	if (vertexBuffer) {
		delete vertexBuffer;
		vertexBuffer = nullptr;
	}
	if (indexBuffer) {
		delete indexBuffer;
		indexBuffer = nullptr;
	}
	vertexBuffer = new BufferWrapper (instance, sizeof (vertices[0]) * vertices.size(), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
	transferData (vertices.data(), vertexBuffer, 0, sizeof (vertices[0]) * vertices.size(), vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, commandPool, submitQueue);

	indexBuffer = new BufferWrapper (instance, sizeof (indices[0]) * indices.size(), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
	transferData (indices.data(), indexBuffer, 0, sizeof (indices[0]) * indices.size(), vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, commandPool, submitQueue);
	
	vertices.clear();
	indices.clear();
}
void OpaqueObjectDispatcher::push_instance (uint32_t objectId, Instance& inst) {
	instances.push_back ({objectId, inst});
}
uint32_t OpaqueObjectDispatcher::setup (MappedBufferWrapper* stagingBuffer, uint32_t offset, vk::CommandPool commandPool, vk::Queue submitQueue) {
	vk::CommandBuffer commandBuffer = instance->createCommandBuffer (commandPool, vk::CommandBufferLevel::ePrimary);

	commandBuffer.begin (vk::CommandBufferBeginInfo (vk::CommandBufferUsageFlags (vk::CommandBufferUsageFlagBits::eOneTimeSubmit)));

	setup (stagingBuffer, offset, commandBuffer);

	commandBuffer.end();

	vk::SubmitInfo submitInfo (
	    0, nullptr,//waitsemaphores
	    nullptr,//pWaitDstStageMask
	    1, &commandBuffer,
	    0, nullptr//signalsemaphores
	);
	submitQueue.submit ({submitInfo}, vk::Fence());
}
uint32_t OpaqueObjectDispatcher::setup (MappedBufferWrapper* stagingBuffer, uint32_t offset, vk::CommandBuffer commandBuffer) {
	if (!desciptorSet) {
		assert (false);
	}
	uint32_t neededSize = sizeof (Instance) * instances.size();
	if (instanceBuffer && instanceBuffer->memory.size < neededSize) {
		printf ("Increase InstanceBuffer %d - %d\n", instanceBuffer->memory.size, neededSize);
		delete instanceBuffer;
		instanceBuffer = nullptr;
	}
	if (!instanceBuffer) {
		instanceBuffer = new BufferWrapper (instance, sizeof (instances[0]) * (instances.size() + (instances.size() / 4)),
		                                    vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);
	}

	if (stagingBuffer->memory.size < neededSize) {
		assert (false);
	}
	if (stagingBuffer->memory.size - offset < neededSize) {
		offset = 0;
	}

	for (OpaqueObject& opObj : parts) {
		opObj.count = 0;
	}

	std::sort (instances.begin(), instances.end(), [] (OpaqueInstance & lhs, OpaqueInstance & rhs) {
		return lhs.objIndex < rhs.objIndex;
	});
	Instance* instanceArray = (Instance*) (stagingBuffer->data + offset);

	uint32_t instanceCount = 0;
	for (OpaqueInstance& opInst : instances) {
		instanceArray[instanceCount++] = opInst.data;
		parts[opInst.objIndex].count++;
	}

	copyBuffer (stagingBuffer, instanceBuffer, offset, 0, sizeof (Instance) *instanceCount,
	            vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite,
	            commandBuffer);

	return offset + neededSize;
}
void OpaqueObjectDispatcher::dispatch (vk::CommandBuffer commandBuffer) {
	if (!desciptorSet) {
		assert (false);
	}
	
	commandBuffer.bindDescriptorSets (vk::PipelineBindPoint::eGraphics, instance->pipeline_module_layouts.standard.pipelineLayout, 1, desciptorSet, {});

	commandBuffer.bindVertexBuffers (0, {vertexBuffer->buffer, instanceBuffer->buffer}, {0, 0});
	
	commandBuffer.bindIndexBuffer (indexBuffer->buffer, 0, vk::IndexType::eUint32);

	uint32_t count = 0;
	for (OpaqueObject& opObj : parts) {
		commandBuffer.pushConstants (instance->pipeline_module_layouts.standard.pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof (ObjectPartData), &opObj.data);
		commandBuffer.drawIndexed (opObj.indexCount, opObj.count, opObj.indexOffset, opObj.vertexOffset, count);
		count += opObj.count;
	}
}
void OpaqueObjectDispatcher::reset_instances() {
	instances.shrink_to_fit();
	instances.clear();
}