#ifndef VULKANRESOURCEMANAGER_H
#define VULKANRESOURCEMANAGER_H

#include <VulkanHeader.h>
#include "render/Model.h"
#include "render/ResourceManager.h"

struct VulkanInstance;
struct VulkanBuffer;
struct VulkanResourceManager;

struct VulkanModelInstances {
	SparseStore<void> store;
};
struct VulkanModel {
	RId id;
	bool index_is_2byte;
	u32 vertexoffset, vertexcount;
	u32 indexoffset, indexcount;
	VulkanBuffer* vertexbuffer;
	VulkanBuffer* indexbuffer;
	
	
};
struct VulkanModelBase {
	RId id;
	VulkanBuffer* vertexbuffer;
	VulkanBuffer* indexbuffer2;//16-bit indices
	VulkanBuffer* indexbuffer4;//32-bit indices

	IdArray<VulkanModel> models;

	VulkanModelBase ( RId id ) : id ( id ), vertexbuffer ( nullptr ), indexbuffer2 ( nullptr ), indexbuffer4 ( nullptr ), models() {}
};

struct VulkanContext {
	vk::DescriptorPool descriptorpool;
	vk::DescriptorSetLayout descriptorsetlayout;
};

struct VulkanTransferBuffer {
	VulkanBuffer* buffer;
	vk::DeviceSize remaining_bytes;
	std::atomic<u32> uses;

	VulkanTransferBuffer ( VulkanInstance* instance, vk::DeviceSize buffersize );
	~VulkanTransferBuffer ( );
};
/*struct VulkanTransferJob {
	VulkanTransferBuffer* used_transfer_buffer;
	VulkanBuffer* dst_buffer;
	u32 transfer_offset, final_offset, byte_size;
	bool is_running;
	vk::Fence finished_fence;
};*/
struct VulkanTransferControl {
	VulkanInstance* v_instance;
	Array<VulkanTransferBuffer*> transfer_buffers;
	//Array<VulkanTransferJob> transfer_jobs;
	vk::CommandPool commandpool;
	vk::CommandBuffer synchron_commandbuffer;

	VulkanTransferControl ( VulkanInstance* v_instance );
	~VulkanTransferControl();

	void transfer_data ( u8 * data, u32 size, VulkanBuffer* dstbuffer, vk::DeviceSize offset );
	void transfer_data ( u8 * data, u32 size, VulkanBuffer* dstbuffer, vk::DeviceSize offset, vk::CommandBuffer commandbuffer );
};

struct VulkanInstance;

struct VulkanResourceManager : public ResourceManager {
	VulkanInstance * v_instance;

	VulkanTransferControl transfer_control;
	
	IdGenerator modelbase_idgen;
	Map<RId, VulkanModelBase> modelbase_map;

	VulkanResourceManager ( VulkanInstance * instance ) : v_instance ( instance ), transfer_control(instance) {}
	~VulkanResourceManager();

	vk::ShaderModule load_shader_from_file ( const char* filename );

	virtual const Model load_generic_model ( RId modelbase_id, u8* vertices, u32 vertexcount, u16* indices, u32 indexcount );
	virtual const Model load_generic_model ( RId modelbase_id, u8* vertices, u32 vertexcount, u32* indices, u32 indexcount );
	
	virtual const ModelInstance create_instance( RId model_id, RId context_id = 0 );
	virtual const ModelInstance create_instance( Model model, RId context_id = 0 );
};

#endif // VULKANRESOURCEMANAGER_H
