#include "VContext.h"
#include "VBuffer.h"
#include "VInstance.h"
#include "VBufferStorage.h"
#include "VImage.h"


VContext::VContext ( VInstance* instance, ContextBaseId contextbase_id ) : id ( 0 ), contextbase_id ( contextbase_id ), v_instance ( instance ), buffer_chunk ( {0, 0, 0} ), data ( nullptr ) {

	const ContextBase* contextbase = v_instance->contextbase ( contextbase_id );
	VContextBase& v_contextbase = v_instance->contextbase_map[contextbase_id];

	images.resize ( contextbase->image_count, VImageUseRef() );
	samplers.resize ( contextbase->sampler_count, nullptr );

	u32 dscount = 0;
	if ( contextbase->datagroup.size ) dscount++;
	if ( contextbase->image_count ) dscount++;
	if ( contextbase->sampler_count ) dscount++;

	Array<vk::DescriptorPoolSize> poolsizes ( dscount );
	dscount = 0;
	if ( contextbase->datagroup.size ) {
		poolsizes[dscount] = vk::DescriptorPoolSize ( vk::DescriptorType::eUniformBuffer, 1 );
		dscount++;
	}
	if ( contextbase->image_count ) {
		poolsizes[dscount] = vk::DescriptorPoolSize ( vk::DescriptorType::eSampledImage, 1 );
		dscount++;
	}
	if ( contextbase->sampler_count ) {
		poolsizes[dscount] = vk::DescriptorPoolSize ( vk::DescriptorType::eSampler, 1 );
		dscount++;
	}
	vk::DescriptorPoolCreateInfo poolInfo ( vk::DescriptorPoolCreateFlags(), dscount, poolsizes.size, poolsizes.data );
	descriptor_pool = v_instance->vk_device().createDescriptorPool ( poolInfo );

	u32 writecount = 0;

	if ( contextbase->datagroup.size && contextbase->datagroup.arraycount ) {
		
		buffer_chunk = v_instance->context_bufferstorage->allocate_chunk(contextbase->datagroup.size * contextbase->datagroup.arraycount);
		writecount++;
	}
	//setup descriptorset
	vk::DescriptorSetAllocateInfo allocInfo ( descriptor_pool, 1, &v_contextbase.descriptorset_layout );
	v_instance->vk_device().allocateDescriptorSets ( &allocInfo, &descriptor_set );

	writecount = 0;
	if ( contextbase->datagroup.size ) {
		//bind descriptorset to buffer
		vk::DescriptorBufferInfo bufferwrite = vk::DescriptorBufferInfo ( v_instance->context_bufferstorage->get_buffer(buffer_chunk.index)->buffer, buffer_chunk.offset, buffer_chunk.size );
		vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet ( descriptor_set, writecount, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufferwrite, nullptr );
		v_instance->vk_device().updateDescriptorSets ( 1, &writeDescriptorSet, 0, nullptr );
		writecount++;
	}
	if ( contextbase->image_count ) {
		writecount++;
	}
	if ( contextbase->sampler_count ) {
		writecount++;
	}
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
