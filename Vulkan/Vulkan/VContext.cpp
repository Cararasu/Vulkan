#include "VContext.h"
#include "VBuffer.h"
#include "VInstance.h"
#include "VBufferStorage.h"


VContext::VContext ( VInstance* instance, ContextBaseId contextbase_id ) : id ( 0 ), contextbase_id ( contextbase_id ), v_instance ( instance ), v_buffer ( instance ) {
	
	vk::DescriptorPoolSize poolsizes[] = {
		vk::DescriptorPoolSize ( vk::DescriptorType::eUniformBuffer, 1 )
	};
	constexpr u32 ds_count = 1;
	vk::DescriptorPoolCreateInfo poolInfo ( vk::DescriptorPoolCreateFlags(), ds_count, 1, poolsizes );
	descriptor_pool = v_instance->vk_device().createDescriptorPool ( poolInfo );

	const ContextBase* contextbase = v_instance->contextbase ( contextbase_id );
	VContextBase& v_contextbase = v_instance->contextbase_map[contextbase_id];
	
	//update buffer
	v_buffer.init ( contextbase->datagroup.size, vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
	                vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );
	
	//setup descriptorset
	vk::DescriptorSetAllocateInfo allocInfo ( descriptor_pool, 1, &v_contextbase.descriptorset_layout );
	v_instance->vk_device().allocateDescriptorSets ( &allocInfo, &uniform_descriptor_set );
	
	//bind descriptorset to buffer
	vk::DescriptorBufferInfo bufferwrite ( v_buffer.buffer, 0, contextbase->datagroup.size );
	vk::WriteDescriptorSet writeDescriptorSets[] = {
		vk::WriteDescriptorSet ( uniform_descriptor_set, 0, 0, 1,
		                         vk::DescriptorType::eUniformBuffer, nullptr, &bufferwrite, nullptr )
	};
	v_instance->vk_device().updateDescriptorSets ( 1, writeDescriptorSets, 0, nullptr );
}
VContext::~VContext() {
	v_instance->vk_device().destroyDescriptorPool ( descriptor_pool );
}

void VContextGroup::set_context ( Context& context ) {
	context_map.insert ( std::make_pair ( context.contextbase_id, v_instance->v_context_map[context.contextbase_id][context.id] ) );
}
void VContextGroup::remove_context ( ContextBaseId base_id ) {
	context_map.erase ( base_id );
}
void VContextGroup::clear() {
	context_map.clear();
}
