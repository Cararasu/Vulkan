#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "VHeader.h"
#include <vector>

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
	void push_instance (std::vector<uint32_t>& objectIds, Instance& inst);

	uint32_t setup (MappedBufferWrapper* stagingBuffer, uint32_t offset, vk::CommandBuffer commandBuffer);
	uint32_t setup (MappedBufferWrapper* stagingBuffer, uint32_t offset, vk::CommandPool commandPool, vk::Queue submitQueue);
	void dispatch (vk::CommandBuffer commandBuffer);

	void reset_instances();
	void hard_reset_instances();
};



#endif // DISPATCHER_H
