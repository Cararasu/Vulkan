#include "VContext.h"
#include "VBuffer.h"
#include "VInstance.h"
#include "VBufferStorage.h"


VContext::VContext ( VInstance* instance, ContextBaseId contextbase_id ) : id ( 0 ), contextbase_id ( contextbase_id ), v_instance ( instance ), v_buffer ( instance ), data ( nullptr ) {

	const ContextBase* contextbase = v_instance->contextbase ( contextbase_id );
	VContextBase& v_contextbase = v_instance->contextbase_map[contextbase_id];

	images.resize(contextbase->image_count, nullptr);

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

	if ( contextbase->datagroup.size ) {
		//update buffer
		v_buffer.init ( contextbase->datagroup.size, vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
		                vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );
		writecount++;
	}
	//setup descriptorset
	vk::DescriptorSetAllocateInfo allocInfo ( descriptor_pool, 1, &v_contextbase.descriptorset_layout );
	v_instance->vk_device().allocateDescriptorSets ( &allocInfo, &descriptor_set );

	writecount = 0;
	if ( contextbase->datagroup.size ) {
		//bind descriptorset to buffer
		vk::DescriptorBufferInfo bufferwrite = vk::DescriptorBufferInfo ( v_buffer.buffer, 0, VK_WHOLE_SIZE );
		vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet ( descriptor_set, writecount, 0, 1, vk::DescriptorType::eUniformBuffer, nullptr, &bufferwrite, nullptr );
		v_instance->vk_device().updateDescriptorSets ( 1, &writeDescriptorSet, 0, nullptr );
		writecount++;
	}
	if ( contextbase->image_count ) {
		writecount++;
	}
	if ( contextbase->sampler_count ) {
		vk::SamplerCreateInfo samplerInfo (
			vk::SamplerCreateFlags(),
			vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
			vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
			0.0f, //mipLodBias
			VK_FALSE, //anisotropyEnable
			0.0f, //maxAnisotropy
			VK_FALSE, //compareEnable
			vk::CompareOp::eNever,
			0.0f, //minLod
			0.0f, //maxLod
			vk::BorderColor::eFloatTransparentBlack, //borderColor
			VK_FALSE //unnormalizedCoordinates
		);
		if(!temp_sampler) temp_sampler = v_instance->vk_device().createSampler(samplerInfo);
		
		//bind descriptorset to image
		vk::DescriptorImageInfo imagewrite = vk::DescriptorImageInfo( temp_sampler, vk::ImageView(), vk::ImageLayout::eUndefined );
		vk::WriteDescriptorSet writeDescriptorSet = vk::WriteDescriptorSet(descriptor_set, writecount, 0, 1, vk::DescriptorType::eSampler, &imagewrite, nullptr, nullptr );
		v_instance->vk_device().updateDescriptorSets ( 1, &writeDescriptorSet, 0, nullptr );
		writecount++;
	}
}
VContext::~VContext() {
	v_instance->vk_device().destroyDescriptorPool ( descriptor_pool );
	v_instance->vk_device().destroySampler(temp_sampler);
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
