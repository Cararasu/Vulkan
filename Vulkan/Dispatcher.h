#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "VGlobal.h"
#include "VHeader.h"
#include <vector>
#include "DataWrapper.h"

struct OpaqueInstance {
	uint32_t objIndex;
	Instance data;
};
struct OpaqueObject {
	uint32_t count;
	ObjectPartData data;
	uint32_t indexOffset;
	uint32_t indexCount;
	int32_t vertexOffset;
};

struct OpaqueObjectDispatcher {

	VInstance* instance;
	std::vector<OpaqueObject> parts;
	std::vector<OpaqueInstance> instances;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	BufferWrapper* instanceBuffer = nullptr;
	BufferWrapper* vertexBuffer = nullptr;
	BufferWrapper* indexBuffer = nullptr;

	ImageWrapper* image;

	vk::Sampler sampler;
	vk::ImageView imageView;
	vk::DescriptorSet desciptorSet;

	OpaqueObjectDispatcher ();
	~OpaqueObjectDispatcher();
	
	uint32_t add_object(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, ObjectPartData& part);
	void set_image_array(ImageWrapper* imageWrapper, vk::Sampler sampler);
	void set_descriptor_set(vk::DescriptorSet desciptorSet);
	
	void upload_data(vk::CommandPool commandPool, vk::Queue submitQueue);

	void push_instance (uint32_t objectId, Instance& inst);

	uint32_t setup (MappedBufferWrapper* stagingBuffer, uint32_t offset, vk::CommandBuffer commandBuffer);
	uint32_t setup (MappedBufferWrapper* stagingBuffer, uint32_t offset, vk::CommandPool commandPool, vk::Queue submitQueue);
	void dispatch (vk::CommandBuffer commandBuffer);

	void reset_instances();
	
	void init(VInstance* instance){
		this->instance = instance;
	}
	
	void finit(VInstance* instance){
		if (vertexBuffer) {
			delete vertexBuffer;
			vertexBuffer = nullptr;
		}
		if (indexBuffer) {
			delete indexBuffer;
			indexBuffer = nullptr;
		}
		if (instanceBuffer) {
			delete instanceBuffer;
			instanceBuffer = nullptr;
		}
		if (imageView)
			instance->device.destroyImageView (imageView);
	}
};



#endif // DISPATCHER_H
