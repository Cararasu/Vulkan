#include "VContext.h"
#include "VBuffer.h"
#include "VInstance.h"
#include "VBufferStorage.h"
#include "VImage.h"
#include "VSampler.h"

const u32 CONTEXT_DESCRIPTOR_SET_COUNT = 2;

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
		poolsizes[0] = vk::DescriptorPoolSize ( type, CONTEXT_DESCRIPTOR_SET_COUNT );
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
		poolsizes[dscount + offset] = vk::DescriptorPoolSize ( type, texres.arraycount * CONTEXT_DESCRIPTOR_SET_COUNT );
		
		dscount++;
	}
	vk::DescriptorPoolCreateInfo poolInfo ( vk::DescriptorPoolCreateFlags(), CONTEXT_DESCRIPTOR_SET_COUNT, poolsizes.size, poolsizes.data );
	descriptor_pool = v_instance->vk_device().createDescriptorPool ( poolInfo );

	u32 writecount = 0;

	if ( contextbase->datagroup.size && contextbase->datagroup.arraycount ) {
		buffer_chunk = v_instance->context_bufferstorage->allocate_chunk(contextbase->datagroup.size * contextbase->datagroup.arraycount);
		writecount++;
	}
	vk::DescriptorSetLayout dsls[CONTEXT_DESCRIPTOR_SET_COUNT];
	for(u32 i = 0; i < CONTEXT_DESCRIPTOR_SET_COUNT; i++) {
		dsls[i] = v_contextbase.descriptorset_layout;
	}
	//setup descriptorset
	vk::DescriptorSetAllocateInfo allocInfo ( descriptor_pool, CONTEXT_DESCRIPTOR_SET_COUNT, dsls );
	
	vk::DescriptorSet dss[CONTEXT_DESCRIPTOR_SET_COUNT];
	v_instance->vk_device().allocateDescriptorSets ( &allocInfo, dss );
	for(u32 i = 0; i < CONTEXT_DESCRIPTOR_SET_COUNT; i++) {
		descriptor_sets[i].last_updated_frame_index = v_instance->frame_index;
		descriptor_sets[i].descriptor_set = dss[i];
	}
	current_ds = 0;
	
	writecount = 0;
	if ( contextbase->datagroup.size ) {
		//bind descriptorset to buffer
		vk::DescriptorBufferInfo bufferwrite = vk::DescriptorBufferInfo ( v_instance->context_bufferstorage->get_buffer(buffer_chunk.index)->buffer, buffer_chunk.offset, buffer_chunk.size );
		vk::WriteDescriptorSet write_descriptor_sets[CONTEXT_DESCRIPTOR_SET_COUNT];
		for(u32 i = 0; i < CONTEXT_DESCRIPTOR_SET_COUNT; i++) {
			write_descriptor_sets[i] = vk::WriteDescriptorSet ( descriptor_sets[i].descriptor_set, writecount, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufferwrite, nullptr );
		}
		v_instance->vk_device().updateDescriptorSets ( CONTEXT_DESCRIPTOR_SET_COUNT, write_descriptor_sets, 0, nullptr );
		writecount++;
	}
}
VContext::~VContext() {
	v_instance->vk_device().destroyDescriptorPool ( descriptor_pool );
}
void VContext::prepare_for_use() {
	if(texture_resources.size) {
		bool updated = false;
		for( VBoundTextureResource& texref : texture_resources) {
			if(texref.imageuse.is_updated()) {
				texref.imageuse.set_updated();
				updated = true;
			}
		}
		if(updated || needs_update) {
			cycle_descriptor_set();
			v_instance->wait_for_frame(descriptor_sets[current_ds].last_updated_frame_index);
			
			const ContextBase* contextbase = v_instance->contextbase ( contextbase_id );
			u32 offset = contextbase->datagroup.size ? 1 : 0;
			
			std::vector<vk::DescriptorImageInfo> dsimageinfo;
			std::vector<vk::WriteDescriptorSet> writesets;
			dsimageinfo.resize(texture_resources.size);
			writesets.resize(texture_resources.size);

			for( u64 i = 0; i < texture_resources.size; i++ ) {
				vk::Sampler sampler = texture_resources[i].sampler ? texture_resources[i].sampler->sampler : vk::Sampler();
				dsimageinfo[i] = vk::DescriptorImageInfo ( sampler, texture_resources[i].imageuse.imageview(), vk::ImageLayout::eShaderReadOnlyOptimal );
				writesets[i] = vk::WriteDescriptorSet (
						descriptor_set(),
						i + offset, 0, 1,
						texture_resources[i].type,
						&dsimageinfo[i],
						nullptr, nullptr
					);
			}
			v_instance->vk_device().updateDescriptorSets ( writesets, {} );
			needs_update = false;
		}
	}
	descriptor_sets[current_ds].last_updated_frame_index = v_instance->frame_index;
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
