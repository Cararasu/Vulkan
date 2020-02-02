#include "VTransformEnums.h"

VFormatData valuetypemap[] = {
	{ vk::Format::eUndefined, 0, 0},

	{vk::Format::eR8Uint, 1, 1},
	{vk::Format::eR16Uint, 2, 1},
	{vk::Format::eR32Uint, 4, 1},
	{vk::Format::eR64Uint, 8, 1},
	{vk::Format::eR8Sint, 1, 1},
	{vk::Format::eR16Sint, 2, 1},
	{vk::Format::eR32Sint, 4, 1},
	{vk::Format::eR64Sint, 8, 1},
	{vk::Format::eR32Sfloat, 4, 1},
	{vk::Format::eR64Sfloat, 8, 1},

	{vk::Format::eR8G8Uint, 2, 1},
	{vk::Format::eR16G16Uint, 4, 1},
	{vk::Format::eR32G32Uint, 8, 1},
	{vk::Format::eR64G64Uint, 16, 1},
	{vk::Format::eR8G8Sint, 2, 1},
	{vk::Format::eR16G16Sint, 4, 1},
	{vk::Format::eR32G32Sint, 8, 1},
	{vk::Format::eR64G64Sint, 16, 1},
	{vk::Format::eR32G32Sfloat, 8, 1},
	{vk::Format::eR64G64Sfloat, 16, 1},

	{vk::Format::eR8G8B8Uint, 3, 1},
	{vk::Format::eR16G16B16Uint, 6, 1},
	{vk::Format::eR32G32B32Uint, 12, 1},
	{vk::Format::eR64G64B64Uint, 24, 1},
	{vk::Format::eR8G8B8Sint, 3, 1},
	{vk::Format::eR16G16B16Sint, 6, 1},
	{vk::Format::eR32G32B32Sint, 12, 1},
	{vk::Format::eR64G64B64Sint, 24, 1},
	{vk::Format::eR32G32B32Sfloat, 12, 1},
	{vk::Format::eR64G64B64Sfloat, 24, 1},

	{vk::Format::eR8G8B8A8Uint, 4, 1},
	{vk::Format::eR16G16B16A16Uint, 8, 1},
	{vk::Format::eR32G32B32A32Uint, 16, 1},
	{vk::Format::eR64G64B64A64Uint, 32, 1},
	{vk::Format::eR8G8B8A8Sint, 4, 1},
	{vk::Format::eR16G16B16A16Sint, 8, 1},
	{vk::Format::eR32G32B32A32Sint, 16, 1},
	{vk::Format::eR64G64B64A64Sint, 32, 1},
	{vk::Format::eR32G32B32A32Sfloat, 16, 1},
	{vk::Format::eR64G64B64A64Sfloat, 32, 1},

	{vk::Format::eR8G8Uint, 2, 2},
	{vk::Format::eR16G16Uint, 4, 2},
	{vk::Format::eR32G32Uint, 8, 2},
	{vk::Format::eR64G64Uint, 16, 2},
	{vk::Format::eR8G8Sint, 2, 2},
	{vk::Format::eR16G16Sint, 4, 2},
	{vk::Format::eR32G32Sint, 8, 2},
	{vk::Format::eR64G64Sint, 16, 2},
	{vk::Format::eR32G32Sfloat, 8, 2},
	{vk::Format::eR64G64Sfloat, 16, 2},

	{vk::Format::eR8G8B8Uint, 3, 3},
	{vk::Format::eR16G16B16Uint, 6, 3},
	{vk::Format::eR32G32B32Uint, 12, 3},
	{vk::Format::eR64G64B64Uint, 24, 3},
	{vk::Format::eR8G8B8Sint, 3, 3},
	{vk::Format::eR16G16B16Sint, 6, 3},
	{vk::Format::eR32G32B32Sint, 12, 3},
	{vk::Format::eR64G64B64Sint, 16, 3},
	{vk::Format::eR32G32B32Sfloat, 12, 3},
	{vk::Format::eR64G64B64Sfloat, 24, 3},

	{vk::Format::eR8G8B8A8Uint, 4, 4},
	{vk::Format::eR16G16B16A16Uint, 8, 4},
	{vk::Format::eR32G32B32A32Uint, 16, 4},
	{vk::Format::eR64G64B64A64Uint, 32, 4},
	{vk::Format::eR8G8B8A8Sint, 4, 4},
	{vk::Format::eR16G16B16A16Sint, 8, 4},
	{vk::Format::eR32G32B32A32Sint, 16, 4},
	{vk::Format::eR64G64B64A64Sint, 32, 4},
	{vk::Format::eR32G32B32A32Sfloat, 16, 4},
	{vk::Format::eR64G64B64A64Sfloat, 32, 4},
};
VFormatData transform_v_format ( ValueType type ) {
	return valuetypemap[ ( int ) type];
}

vk::Format transform_image_format ( ImageFormat format ) {
	switch ( format ) {
	case ImageFormat::U8:
		return vk::Format::eR8Uint;
	case ImageFormat::U16:
		return vk::Format::eR16Uint;
	case ImageFormat::U32:
		return vk::Format::eR32Uint;
	case ImageFormat::U64:
		return vk::Format::eR64Uint;

	case ImageFormat::Unorm8:
		return vk::Format::eR8Unorm;
	case ImageFormat::Unorm16:
		return vk::Format::eR16Unorm;

	case ImageFormat::S8:
		return vk::Format::eR8Sint;
	case ImageFormat::S16:
		return vk::Format::eR16Sint;
	case ImageFormat::S32:
		return vk::Format::eR32Sint;
	case ImageFormat::S64:
		return vk::Format::eR64Sint;

	case ImageFormat::Snorm8:
		return vk::Format::eR8Snorm;
	case ImageFormat::Snorm16:
		return vk::Format::eR16Snorm;

	case ImageFormat::F16:
		return vk::Format::eR16Sfloat;
	case ImageFormat::F32:
		return vk::Format::eR32Sfloat;
	case ImageFormat::F64:
		return vk::Format::eR64Sfloat;


	case ImageFormat::U8x2:
		return vk::Format::eR8G8Uint;
	case ImageFormat::U16x2:
		return vk::Format::eR16G16Uint;
	case ImageFormat::U32x2:
		return vk::Format::eR32G32Uint;
	case ImageFormat::U64x2:
		return vk::Format::eR64G64Uint;

	case ImageFormat::Unorm8x2:
		return vk::Format::eR8G8Unorm;
	case ImageFormat::Unorm16x2:
		return vk::Format::eR16G16Unorm;

	case ImageFormat::S8x2:
		return vk::Format::eR8G8Sint;
	case ImageFormat::S16x2:
		return vk::Format::eR16G16Sint;
	case ImageFormat::S32x2:
		return vk::Format::eR32G32Sint;
	case ImageFormat::S64x2:
		return vk::Format::eR64G64Sint;

	case ImageFormat::Snorm8x2:
		return vk::Format::eR8G8Snorm;
	case ImageFormat::Snorm16x2:
		return vk::Format::eR16G16Snorm;

	case ImageFormat::F16x2:
		return vk::Format::eR16G16Sfloat;
	case ImageFormat::F32x2:
		return vk::Format::eR32G32Sfloat;
	case ImageFormat::F64x2:
		return vk::Format::eR64G64Sfloat;


	case ImageFormat::U8x4:
		return vk::Format::eR8G8B8A8Uint;
	case ImageFormat::U16x4:
		return vk::Format::eR16G16B16A16Uint;
	case ImageFormat::U32x4:
		return vk::Format::eR32G32B32A32Uint;
	case ImageFormat::U64x4:
		return vk::Format::eR64G64B64A64Uint;

	case ImageFormat::Unorm8x4:
		return vk::Format::eR8G8B8A8Unorm;
	case ImageFormat::Unorm16x4:
		return vk::Format::eR16G16B16A16Unorm;

	case ImageFormat::S8x4:
		return vk::Format::eR8G8B8A8Sint;
	case ImageFormat::S16x4:
		return vk::Format::eR16G16B16A16Sint;
	case ImageFormat::S32x4:
		return vk::Format::eR32G32B32A32Sint;
	case ImageFormat::S64x4:
		return vk::Format::eR64G64B64A64Sint;

	case ImageFormat::Snorm8x4:
		return vk::Format::eR8G8B8A8Snorm;
	case ImageFormat::Snorm16x4:
		return vk::Format::eR16G16B16A16Snorm;

	case ImageFormat::F16x4:
		return vk::Format::eR16G16B16A16Sfloat;
	case ImageFormat::F32x4:
		return vk::Format::eR32G32B32A32Sfloat;
	case ImageFormat::F64x4:
		return vk::Format::eR64G64B64A64Sfloat;


	case ImageFormat::U10x3_U2:
		return vk::Format::eA2B10G10R10UintPack32;
	case ImageFormat::S10x3_S2:
		return vk::Format::eA2B10G10R10SintPack32;
	case ImageFormat::Unorm10x3_Unorm2:
		return vk::Format::eA2B10G10R10UnormPack32;
	case ImageFormat::Snorm10x3_Snorm2:
		return vk::Format::eA2B10G10R10SnormPack32;

	case ImageFormat::D16Unorm:
		return vk::Format::eD16Unorm;
	case ImageFormat::D32F:
		return vk::Format::eD32Sfloat;
	case ImageFormat::D24Unorm_St8U:
		return vk::Format::eD24UnormS8Uint;
	case ImageFormat::D32F_St8U:
		return vk::Format::eD32SfloatS8Uint;
	case ImageFormat::St8U:
		return vk::Format::eS8Uint;

	default:
		return vk::Format::eUndefined;
	}
}
ImageFormat transform_image_format ( vk::Format format ) {
	switch ( format ) {
	case vk::Format::eR8Uint:
		return ImageFormat::U8;
	case vk::Format::eR16Uint:
		return ImageFormat::U16;
	case vk::Format::eR32Uint:
		return ImageFormat::U32;
	case vk::Format::eR64Uint:
		return ImageFormat::U64;

	case vk::Format::eR8Unorm:
		return ImageFormat::Unorm8;
	case vk::Format::eR16Unorm:
		return ImageFormat::Unorm16;

	case vk::Format::eR8Sint:
		return ImageFormat::S8;
	case vk::Format::eR16Sint:
		return ImageFormat::S16;
	case vk::Format::eR32Sint:
		return ImageFormat::S32;
	case vk::Format::eR64Sint:
		return ImageFormat::S64;

	case vk::Format::eR8Snorm:
		return ImageFormat::Snorm8;
	case vk::Format::eR16Snorm:
		return ImageFormat::Snorm16;

	case vk::Format::eR16Sfloat:
		return ImageFormat::F16;
	case vk::Format::eR32Sfloat:
		return ImageFormat::F32;
	case vk::Format::eR64Sfloat:
		return ImageFormat::F64;


	case vk::Format::eR8G8Uint:
		return ImageFormat::U8x2;
	case vk::Format::eR16G16Uint:
		return ImageFormat::U16x2;
	case vk::Format::eR32G32Uint:
		return ImageFormat::U32x2;
	case vk::Format::eR64G64Uint:
		return ImageFormat::U64x2;

	case vk::Format::eR8G8Unorm:
		return ImageFormat::Unorm8x2;
	case vk::Format::eR16G16Unorm:
		return ImageFormat::Unorm16x2;

	case vk::Format::eR8G8Sint:
		return ImageFormat::S8x2;
	case vk::Format::eR16G16Sint:
		return ImageFormat::S16x2;
	case vk::Format::eR32G32Sint:
		return ImageFormat::S32x2;
	case vk::Format::eR64G64Sint:
		return ImageFormat::S64x2;

	case vk::Format::eR8G8Snorm:
		return ImageFormat::Snorm8x2;
	case vk::Format::eR16G16Snorm:
		return ImageFormat::Snorm16x2;

	case vk::Format::eR16G16Sfloat:
		return ImageFormat::F16x2;
	case vk::Format::eR32G32Sfloat:
		return ImageFormat::F32x2;
	case vk::Format::eR64G64Sfloat:
		return ImageFormat::F64x2;


	case vk::Format::eR8G8B8A8Uint:
		return ImageFormat::U8x4;
	case vk::Format::eR16G16B16A16Uint:
		return ImageFormat::U16x4;
	case vk::Format::eR32G32B32A32Uint:
		return ImageFormat::U32x4;
	case vk::Format::eR64G64B64A64Uint:
		return ImageFormat::U64x4;

	case vk::Format::eR8G8B8A8Unorm:
		return ImageFormat::Unorm8x4;
	case vk::Format::eR16G16B16A16Unorm:
		return ImageFormat::Unorm16x4;

	case vk::Format::eR8G8B8A8Sint:
		return ImageFormat::S8x4;
	case vk::Format::eR16G16B16A16Sint:
		return ImageFormat::S16x4;
	case vk::Format::eR32G32B32A32Sint:
		return ImageFormat::S32x4;
	case vk::Format::eR64G64B64A64Sint:
		return ImageFormat::S64x4;

	case vk::Format::eR8G8B8A8Snorm:
		return ImageFormat::Snorm8x4;
	case vk::Format::eR16G16B16A16Snorm:
		return ImageFormat::Snorm16x4;

	case vk::Format::eR16G16B16A16Sfloat:
		return ImageFormat::F16x4;
	case vk::Format::eR32G32B32A32Sfloat:
		return ImageFormat::F32x4;
	case vk::Format::eR64G64B64A64Sfloat:
		return ImageFormat::F64x4;


	case vk::Format::eA2B10G10R10UintPack32:
		return ImageFormat::U10x3_U2;
	case vk::Format::eA2B10G10R10SintPack32:
		return ImageFormat::S10x3_S2;
	case vk::Format::eA2B10G10R10UnormPack32:
		return ImageFormat::Unorm10x3_Unorm2;
	case vk::Format::eA2B10G10R10SnormPack32:
		return ImageFormat::Snorm10x3_Snorm2;

	case vk::Format::eD16Unorm:
		return ImageFormat::D16Unorm;
	case vk::Format::eD32Sfloat:
		return ImageFormat::D32F;
	case vk::Format::eD24UnormS8Uint:
		return ImageFormat::D24Unorm_St8U;
	case vk::Format::eD32SfloatS8Uint:
		return ImageFormat::D32F_St8U;
	case vk::Format::eS8Uint:
		return ImageFormat::St8U;

	default:
		return ImageFormat::Undefined;
	}
}
vk::Filter transform_filter(FilterType filter) {
	switch(filter) {
	case FilterType::eNearest:
		return vk::Filter::eNearest;
	case FilterType::eLinear:
		return vk::Filter::eLinear;
	}
	assert(false);
}
FilterType transform_filter(vk::Filter filter) {
	switch(filter) {
	case vk::Filter::eNearest:
		return FilterType::eNearest;
	case vk::Filter::eLinear:
		return FilterType::eLinear;
	}
	assert(false);
}

vk::SamplerMipmapMode transform_mipmapfilter(FilterType filter) {
	switch(filter) {
	case FilterType::eNearest:
		return vk::SamplerMipmapMode::eNearest;
	case FilterType::eLinear:
		return vk::SamplerMipmapMode::eLinear;
	}
	assert(false);
}
FilterType transform_mipmapfilter(vk::SamplerMipmapMode filter) {
	switch(filter) {
	case vk::SamplerMipmapMode::eNearest:
		return FilterType::eNearest;
	case vk::SamplerMipmapMode::eLinear:
		return FilterType::eLinear;
	}
	assert(false);
}

vk::SamplerAddressMode transform_addressmode(EdgeHandling addressmode) {
	switch(addressmode) {
	case EdgeHandling::eRepeat:
		return vk::SamplerAddressMode::eRepeat;
	case EdgeHandling::eMirror:
		return vk::SamplerAddressMode::eMirroredRepeat;
	case EdgeHandling::eClamp:
		return vk::SamplerAddressMode::eClampToEdge;
	}
	assert(false);
}
EdgeHandling transform_addressmode(vk::SamplerAddressMode addressmode) {
	switch(addressmode) {
	case vk::SamplerAddressMode::eRepeat:
		return EdgeHandling::eRepeat;
	case vk::SamplerAddressMode::eMirroredRepeat:
		return EdgeHandling::eMirror;
	case vk::SamplerAddressMode::eClampToEdge:
		return EdgeHandling::eClamp;
	}
	assert(false);
}

vk::CompareOp transform_depthcompare(DepthComparison comp) {
	switch(comp) {
	case DepthComparison::eTrue:
		return vk::CompareOp::eAlways;
	case DepthComparison::eFalse:
		return vk::CompareOp::eNever;
	case DepthComparison::eLower:
		return vk::CompareOp::eLess;
	case DepthComparison::eEquals:
		return vk::CompareOp::eEqual;
	case DepthComparison::eLEquals:
		return vk::CompareOp::eLessOrEqual;
	case DepthComparison::eGreater:
		return vk::CompareOp::eGreater;
	case DepthComparison::eNEquals:
		return vk::CompareOp::eNotEqual;
	case DepthComparison::eGEquals:
		return vk::CompareOp::eGreaterOrEqual;
	}
	assert(false);
}
DepthComparison transform_depthcompare(vk::CompareOp comp) {
	switch(comp) {
	case vk::CompareOp::eAlways:
		return DepthComparison::eTrue;
	case vk::CompareOp::eNever:
		return DepthComparison::eFalse;
	case vk::CompareOp::eLess:
		return DepthComparison::eLower;
	case vk::CompareOp::eEqual:
		return DepthComparison::eEquals;
	case vk::CompareOp::eLessOrEqual:
		return DepthComparison::eLEquals;
	case vk::CompareOp::eGreater:
		return DepthComparison::eGreater;
	case vk::CompareOp::eNotEqual:
		return DepthComparison::eNEquals;
	case vk::CompareOp::eGreaterOrEqual:
		return DepthComparison::eGEquals;
	}
	assert(false);
}