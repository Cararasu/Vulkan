#pragma once

#include "VHeader.h"
#include <render/Resources.h>

struct VFormatData {
	vk::Format format;
	u32 bytesize;
	u32 count;
};

VFormatData transform_v_format ( ValueType type );

VFormatData transform_v_format ( ValueType type );

vk::Format transform_image_format ( ImageFormat format );
ImageFormat transform_image_format ( vk::Format format );

vk::Filter transform_filter(FilterType filter);
FilterType transform_filter(vk::Filter filter);

vk::SamplerMipmapMode transform_mipmapfilter(FilterType filter);
FilterType transform_mipmapfilter(vk::SamplerMipmapMode filter);

vk::SamplerAddressMode transform_addressmode(EdgeHandling addressmode);
EdgeHandling transform_addressmode(vk::SamplerAddressMode addressmode);

vk::CompareOp transform_depthcompare(DepthComparison addressmode);
DepthComparison transform_depthcompare(vk::CompareOp addressmode);
