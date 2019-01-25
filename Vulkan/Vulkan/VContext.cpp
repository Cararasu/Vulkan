#include "VContext.h"
#include "VBuffer.h"
#include "VInstance.h"
#include "VBufferStorage.h"
#include "VImage.h"
#include "VSampler.h"


VContext::VContext ( VInstance* instance, ContextBaseId contextbase_id ) : id ( 0 ), contextbase_id ( contextbase_id ), v_instance ( instance ), buffer_chunk ( {0, 0, 0} ), data ( nullptr ) {

	const ContextBase* contextbase = v_instance->contextbase ( contextbase_id );
	VContextBase& v_contextbase = v_instance->contextbase_map[contextbase_id];

	texture_resources.resize ( contextbase->texture_resources.size );

	u32 offset = contextbase->datagroup.size ? 1 : 0;

	Array<vk::DescriptorPoolSize> poolsizes ( offset + contextbase->texture_resources.size );
	if ( contextbase->datagroup.size ) {
		vk::DescriptorType type;
		if(contextbase->datagroup.needs_write)
			type = vk::DescriptorType::eStorageBuffer;
		else
			type = vk::DescriptorType::eUniformBuffer;
		poolsizes[0] = vk::DescriptorPoolSize ( type, 1 );
		//maybe eUniformBufferDynamic and eStorageBufferDynamic
	}
	u32 dscount = 0;
	for ( TextureResource& texres : contextbase->texture_resources ) {
		vk::DescriptorType type;
		switch(texres.type) {
		case TextureResourceType::eImage:
			if(texres.needs_write)
				type = vk::DescriptorType::eStorageImage;
			else
				type = vk::DescriptorType::eSampledImage;
			break;
		case TextureResourceType::eSampler:
			type = vk::DescriptorType::eSampler;
			break;
		case TextureResourceType::eImageSampled:
			type = vk::DescriptorType::eCombinedImageSampler;
			break;
		case TextureResourceType::eBufferSampled:
			if(texres.needs_write)
				type = vk::DescriptorType::eStorageTexelBuffer;
			else
				type = vk::DescriptorType::eUniformTexelBuffer;
			break;
		default:
			assert(false);
			break;
		}
		texture_resources[dscount].type = type;
		poolsizes[dscount + offset] = vk::DescriptorPoolSize ( type, texres.arraycount );
		dscount++;
	}
	vk::DescriptorPoolCreateInfo poolInfo ( vk::DescriptorPoolCreateFlags(), dscount + offset, poolsizes.size, poolsizes.data );
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
}
VContext::~VContext() {
	v_instance->vk_device().destroyDescriptorPool ( descriptor_pool );
}
void VContext::update_if_needed() {
	bool updated = false;
	for( VBoundTextureResource& texref : texture_resources) {
		if(texref.imageuse.is_updated()) {
			texref.imageuse.set_updated();
			updated = true;
		}
	}
	if(updated) {
		const ContextBase* contextbase = v_instance->contextbase ( contextbase_id );
		u32 offset = contextbase->datagroup.size ? 1 : 0;
		std::vector<vk::DescriptorImageInfo> dsimageinfo;
		std::vector<vk::WriteDescriptorSet> writesets;
		dsimageinfo.resize(texture_resources.size - offset);
		writesets.resize(texture_resources.size - offset);

		for( int i = 0; i < texture_resources.size - offset; i++ ) {
			vk::Sampler sampler = texture_resources[i].sampler ? texture_resources[i].sampler->sampler : vk::Sampler();
			dsimageinfo[i] = vk::DescriptorImageInfo ( sampler, texture_resources[i].imageuse.imageview(), vk::ImageLayout::eShaderReadOnlyOptimal );
			writesets[i] = vk::WriteDescriptorSet (
					descriptor_set,
					i + offset, 0, 1,
					texture_resources[i + offset].type,
					&dsimageinfo[i],
					nullptr, nullptr
				);
		}
		//TODO vk::DescriptorType::eSampledImage is wrong
		v_instance->vk_device().updateDescriptorSets ( writesets, {} );
	}
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
