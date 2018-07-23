
#include <inttypes.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint32_t RId;

typedef float f32;
typedef double f64;

/*
u32 valuetype_sizes[] = {
	1, 2, 4, 8,
	1, 2, 4, 8,
	   2, 4, 8,
	
	2, 4, 8, 16,
	2, 4, 8, 16,
	2, 4, 8, 16,
	
	3, 6, 12, 24,
	3, 6, 12, 24,
	3, 6, 12, 24,
	
	4, 8, 16, 32,
	4, 8, 16, 32,
	4, 8, 16, 32,
	
	4, 8, 16, 32,
	4, 8, 16, 32,
	4, 8, 16, 32,
	
	9, 18, 36, 72,
	9, 18, 36, 72,
	9, 18, 36, 72,
	
	16, 32, 48, 64,
	16, 32, 48, 64,
	16, 32, 48, 64,
};*/

enum class ValueType {
	eU8, eU16, eU32, eU64,
	eS8, eS16, eS32, eS64,
	     eF16, eF32, eF64,

	eU8Vec2, eU16Vec2, eU32Vec2, eU64Vec2,
	eS8Vec2, eS16Vec2, eS32Vec2, eS64Vec2,
	eF8Vec2, eF16Vec2, eF32Vec2, eF64Vec2,

	eU8Vec3, eU16Vec3, eU32Vec3, eU64Vec3,
	eS8Vec3, eS16Vec3, eS32Vec3, eS64Vec3,
	eF8Vec3, eF16Vec3, eF32Vec3, eF64Vec3,

	eU8Vec4, eU16Vec4, eU32Vec4, eU64Vec4,
	eS8Vec4, eS16Vec4, eS32Vec4, eS64Vec4,
	eF8Vec4, eF16Vec4, eF32Vec4, eF64Vec4,

	eU8Mat2, eU16Mat2, eU32Mat2, eU64Mat2,
	eS8Mat2, eS16Mat2, eS32Mat2, eS64Mat2,
	eF8Mat2, eF16Mat2, eF32Mat2, eF64Mat2,

	eU8Mat3, eU16Mat3, eU32Mat3, eU64Mat3,
	eS8Mat3, eS16Mat3, eS32Mat3, eS64Mat3,
	eF8Mat3, eF16Mat3, eF32Mat3, eF64Mat3,

	eU8Mat4, eU16Mat4, eU32Mat4, eU64Mat4,
	eS8Mat4, eS16Mat4, eS32Mat4, eS64Mat4,
	eF8Mat4, eF16Mat4, eF32Mat4, eF64Mat4,
};
//type U, S, Unorm, Snorm, Float
enum class ImageFormat {
	eUndefined,
	
	eU8, eU16, eU32, eU64,
	eS8, eS16, eS32, eS64,
	     eF16, eF32, eF64,
	eUnorm8, eUnorm16,
	eSnorm8, eSnorm16,
	
	e2U8, e2U16, e2U32, e2U64,
	e2S8, e2S16, e2S32, e2S64,
	      e2F16, e2F32, e2F64,
	e2Unorm8, e2Unorm16,
	e2Snorm8, e2Snorm16,
	
	e4U8, e4U16, e4U32, e4U64,
	e4S8, e4S16, e4S32, e4S64,
	      e4F16, e4F32, e4F64,
	e4Unorm8, e4Unorm16,
	e4Snorm8, e4Snorm16,
	
	//srgb???
	
	e3U10_U2,
	e3S10_S2,
	e3Unorm10_Unorm2,
	e3Snorm10_Snorm2,
	
	eD16Unorm, eD32F, 
	eD24Unorm_St8U, eD32F_St8Uint,
	eSt8Uint
};
enum class ResourceType {
	eUndefined,
	eImage,
	eBuffer,
	
	eModelBase,
	eContextBase,
	eInstanceBase,
	
	eModel,
	eModelInstance,
	eRenderPass,
};
struct ResourceHandle {
	ResourceType type = ResourceType::eUndefined;
	RId id = 0;
};

inline bool operator== ( ResourceHandle lh, ResourceHandle rh ) {
	return lh.type == rh.type && lh.id == rh.id;
}
