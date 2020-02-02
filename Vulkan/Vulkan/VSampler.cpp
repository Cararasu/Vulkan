#include "VSampler.h"
#include "VTransformEnums.h"
#include "VInstance.h"

VSampler::VSampler ( VInstance* instance, FilterType magnification, FilterType minification, FilterType mipmapping, EdgeHandling u, EdgeHandling v, EdgeHandling w, float lodbias, Range<float> lodrange, float anismax, DepthComparison comp ) :
	Sampler ( magnification, minification, mipmapping, u, v, w, lodbias, lodrange, anismax, comp ), v_instance ( instance ) {
	vk::SamplerCreateInfo samplerInfo (
	    vk::SamplerCreateFlags(),
	    transform_filter ( magnification ), transform_filter ( minification ), transform_mipmapfilter ( mipmapping ),
	    transform_addressmode ( u ), transform_addressmode ( v ), transform_addressmode ( w ),
	    lodbias, //mipLodBias
	    anismax < 1.0f ? VK_FALSE : VK_TRUE, //anisotropyEnable
	    anismax, //maxAnisotropy
	    comp == DepthComparison::eNone ? VK_FALSE : VK_TRUE, //compareEnable
	    comp == DepthComparison::eNone ? vk::CompareOp::eNever : transform_depthcompare ( comp ),
	    lodrange.min, //minLod
	    lodrange.max, //maxLod
	    vk::BorderColor::eFloatTransparentBlack, //borderColor
	    VK_FALSE //unnormalizedCoordinates
	);
	v_instance->vk_device().createSampler ( &samplerInfo, nullptr, &sampler );
	created_frame_index = v_instance->frame_index;
}

VSampler::~VSampler() {
	v_instance->vk_device().destroySampler ( sampler, nullptr );
}
