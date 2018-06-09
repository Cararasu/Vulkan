#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "VGlobal.h"
#include "VHeader.h"
#include <vector>
#include "DataWrapper.h"

struct OpaqueInstance {
	u32 objIndex;
	InstanceObj data;
};
struct OpaqueObject {
	u32 count;
	ObjectPartData data;
	u32 indexOffset;
	u32 indexCount;
	int32_t vertexOffset;
};

struct OpaqueObjectDispatcher {

	VInstance* instance;
	std::vector<OpaqueObject> parts;
	std::vector<OpaqueInstance> instances;
	std::vector<Vertex> vertices;
	std::vector<u32> indices;

	BufferWrapper* instanceBuffer = nullptr;
	BufferWrapper* vertexBuffer = nullptr;
	BufferWrapper* indexBuffer = nullptr;

	ImageWrapper* image;

	vk::Sampler sampler;
	vk::ImageView imageView;
	vk::DescriptorSet desciptorSet;

	OpaqueObjectDispatcher ();
	~OpaqueObjectDispatcher();
	
	u32 add_object(std::vector<Vertex>& vertices, std::vector<u32>& indices, ObjectPartData& part);
	void set_image_array(ImageWrapper* imageWrapper, vk::Sampler sampler);
	void set_descriptor_set(vk::DescriptorSet desciptorSet);
	
	void upload_data(vk::CommandPool commandPool, vk::Queue submitQueue);

	void push_instance (u32 objectId, InstanceObj& inst);

	u32 setup (MappedBufferWrapper* stagingBuffer, u32 offset, vk::CommandBuffer commandBuffer);
	u32 setup (MappedBufferWrapper* stagingBuffer, u32 offset, vk::CommandPool commandPool, vk::Queue submitQueue);
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
