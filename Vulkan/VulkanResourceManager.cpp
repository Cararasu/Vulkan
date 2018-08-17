#include "VulkanResourceManager.h"

#include "VulkanInstance.h"
#include <fstream>
#include "VulkanBuffer.h"


VulkanTransferControl::VulkanTransferControl ( VulkanInstance* instance ) : v_instance ( instance ), transfer_buffers(), commandpool(), synchron_commandbuffer() {

}
VulkanTransferControl::~VulkanTransferControl() {
	if(synchron_commandbuffer){
		vulkan_device ( v_instance ).freeCommandBuffers ( commandpool, 1, &synchron_commandbuffer );
	}
	if(commandpool){
		vulkan_device ( v_instance ).destroyCommandPool ( commandpool );
	}
}
void VulkanTransferControl::transfer_data ( u8 * data, u32 size, VulkanBuffer* dstbuffer, vk::DeviceSize offset ) {
	if(!commandpool){
		vk::CommandPoolCreateInfo createInfo ( vk::CommandPoolCreateFlagBits::eResetCommandBuffer, vulkan_pgc_queue_wrapper ( v_instance, 0 )->graphics_queue_id );
		vulkan_device ( v_instance ).createCommandPool ( &createInfo, nullptr, &commandpool );
	}
	if ( synchron_commandbuffer ) {
		synchron_commandbuffer.reset ( vk::CommandBufferResetFlags() );
	} else {
		vk::CommandBufferAllocateInfo allocateInfo ( commandpool, vk::CommandBufferLevel::ePrimary, 1 );
		vulkan_device ( v_instance ).allocateCommandBuffers ( &allocateInfo, &synchron_commandbuffer );
	}
	//should be transfered to here:
	//transfer_data(data, size, dstbuffer, offset, synchron_commandbuffer);
	VulkanBuffer stagingbuffer ( v_instance, size,
	                             vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferSrc,
	                             vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible );
	stagingbuffer.map_mem();
	memcpy ( stagingbuffer.mapped_ptr, data, size );

	synchron_commandbuffer.begin ( vk::CommandBufferBeginInfo ( vk::CommandBufferUsageFlagBits::eOneTimeSubmit ) );
	vk::BufferCopy copyRegion ( 0, offset, size );
	synchron_commandbuffer.copyBuffer ( stagingbuffer.buffer, dstbuffer->buffer, 1, &copyRegion );
	synchron_commandbuffer.end();
	vk::PipelineStageFlags waitflags = vk::PipelineStageFlagBits::eHost;
	vk::SubmitInfo submitInfo ( 0, nullptr, &waitflags,
	                            1, &synchron_commandbuffer,
	                            0, nullptr );
	//should be transfer queue
	v_instance->queues.pgc[0].graphics_queue.submit ( 1, &submitInfo, vk::Fence() );
	v_instance->queues.pgc[0].graphics_queue.waitIdle();
}
void VulkanTransferControl::transfer_data ( u8 * data, u32 size, VulkanBuffer* dstbuffer, vk::DeviceSize offset, vk::CommandBuffer commandbuffer ) {

}

VulkanTransferBuffer::VulkanTransferBuffer ( VulkanInstance* instance, vk::DeviceSize buffersize ) :
	buffer ( new VulkanBuffer ( instance, buffersize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible ) ),
	remaining_bytes ( buffersize ), uses ( 0 ) {
	buffer->map_mem();
}
VulkanTransferBuffer::~VulkanTransferBuffer ( ) {
	delete buffer;
}
static std::vector<char> readFile ( const char* filename ) {
	std::ifstream file ( filename, std::ios::ate | std::ios::binary );

	if ( !file.is_open() ) {
		printf ( "Couldn't open File %s\n", filename );
		return std::vector<char>();
	}

	size_t fileSize = ( size_t ) file.tellg();
	std::vector<char> buffer ( fileSize );

	file.seekg ( 0 );
	file.read ( buffer.data(), fileSize );
	file.close();

	return buffer;
}
VulkanResourceManager::~VulkanResourceManager(){
	for(auto& entry : modelbase_map){
		for(VulkanModel& model : entry.second.models){
			delete model.vertexbuffer;
			delete model.indexbuffer;
		}
	}
}
vk::ShaderModule VulkanResourceManager::load_shader_from_file ( const char* filename ) {

	std::vector<char> shaderCode = readFile ( filename );

	vk::ShaderModuleCreateInfo createInfo ( vk::ShaderModuleCreateFlags(), shaderCode.size(), ( const u32* ) shaderCode.data() );

	vk::ShaderModule shadermodule;
	V_CHECKCALL ( vulkan_device ( v_instance ).createShaderModule ( &createInfo, nullptr, &shadermodule ), printf ( "Creation of Shadermodule failed\n" ) );
	return shadermodule;
}
const Model VulkanResourceManager::load_generic_model ( RId modelbase_id, u8* vertices, u32 vertexcount, u16* indices, u32 indexcount ) {
	const ModelBase& modelbase = modelbase_store[modelbase_id];
	const DataGroupDef& datagroupdef = datagroup_store[modelbase.datagroup];

	auto it = modelbase_map.find ( modelbase.id );
	if ( it == modelbase_map.end() ) {
		modelbase_map.insert ( std::make_pair ( modelbase.id, VulkanModelBase ( modelbase.id ) ) );
		it = modelbase_map.find ( modelbase.id );
	}

	VulkanModel newmodel;
	newmodel.index_is_2byte = true;
	newmodel.vertexoffset = 0;
	newmodel.vertexcount = vertexcount;
	newmodel.indexoffset = 0;
	newmodel.indexcount = indexcount;
	newmodel.vertexbuffer = new VulkanBuffer ( v_instance, vertexcount * datagroupdef.size,
	        vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
	        vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );
	newmodel.indexbuffer = new VulkanBuffer ( v_instance, indexcount * sizeof ( u16 ),
	        vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
	        vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );
	
	this->transfer_control.transfer_data(vertices, vertexcount * datagroupdef.size, newmodel.vertexbuffer, 0);
	this->transfer_control.transfer_data((u8*)indices, indexcount * sizeof ( u16 ), newmodel.indexbuffer, 0);
	return {it->second.models.insert ( newmodel ), modelbase_id};
}
const Model VulkanResourceManager::load_generic_model ( RId modelbase_id, u8* vertices, u32 vertexcount, u32* indices, u32 indexcount ) {
	const ModelBase& modelbase = modelbase_store[modelbase_id];
	const DataGroupDef& datagroupdef = datagroup_store[modelbase.datagroup];
	bool fitsin2bytes = vertexcount < 0x10000;

	auto it = modelbase_map.find ( modelbase.id );
	if ( it == modelbase_map.end() ) {
		modelbase_map.insert ( std::make_pair ( modelbase.id, VulkanModelBase ( modelbase.id ) ) );
		it = modelbase_map.find ( modelbase.id );
	}

	VulkanModel newmodel;
	newmodel.index_is_2byte = false;
	newmodel.vertexoffset = 0;
	newmodel.vertexcount = vertexcount;
	newmodel.indexoffset = 0;
	newmodel.indexcount = indexcount;
	newmodel.vertexbuffer = new VulkanBuffer ( v_instance, vertexcount * datagroupdef.size,
	        vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
	        vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );
	newmodel.indexbuffer = new VulkanBuffer ( v_instance, indexcount * sizeof ( u32 ),
	        vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
	        vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );


	return {it->second.models.insert ( newmodel ), modelbase_id};
}
const ModelInstance VulkanResourceManager::create_instance( RId model_id, RId context_id ) {
	
}
const ModelInstance VulkanResourceManager::create_instance( Model model, RId context_id ) {
}
