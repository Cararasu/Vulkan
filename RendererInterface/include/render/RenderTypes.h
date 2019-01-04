#pragma once

#include <cassert>
#include <string>

#ifdef NDEBUG
	#define vassert(COND)  ((void)0)
	#define vassert_exec(COND, CODE) ((void)0)
#else
	#define vassert(COND) if(!(COND)) {fprintf(stderr, "Triggered assert vassert(" #COND ") in file " __FILE__ " on line %d\n", __LINE__);*((u8*)0) = 12;}
	#define vassert_exec(COND, CODE) if(!(COND)) {[](const char* condition, const char* file, int line) { CODE } (#COND, __FILE__, __LINE__);*((u8*)0) = 12;}
#endif

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#define RID_STATIC_IDS (0x80000000)

typedef u32 RId;

typedef RId ContextBaseId;
typedef RId ModelBaseId;
typedef RId InstanceBaseId;

typedef RId ContextId;
typedef RId ModelId;

typedef float f32;
typedef double f64;

enum class ValueType {
	eUndefined = 0,
	eU8, eU16, eU32, eU64,
	eS8, eS16, eS32, eS64,
	           eF32, eF64,

	eU8Vec2, eU16Vec2, eU32Vec2, eU64Vec2,
	eS8Vec2, eS16Vec2, eS32Vec2, eS64Vec2,
	                   eF32Vec2, eF64Vec2,

	eU8Vec3, eU16Vec3, eU32Vec3, eU64Vec3,
	eS8Vec3, eS16Vec3, eS32Vec3, eS64Vec3,
	                   eF32Vec3, eF64Vec3,

	eU8Vec4, eU16Vec4, eU32Vec4, eU64Vec4,
	eS8Vec4, eS16Vec4, eS32Vec4, eS64Vec4,
	                   eF32Vec4, eF64Vec4,

	eU8Mat2, eU16Mat2, eU32Mat2, eU64Mat2,
	eS8Mat2, eS16Mat2, eS32Mat2, eS64Mat2,
	                   eF32Mat2, eF64Mat2,

	eU8Mat3, eU16Mat3, eU32Mat3, eU64Mat3,
	eS8Mat3, eS16Mat3, eS32Mat3, eS64Mat3,
	                   eF32Mat3, eF64Mat3,

	eU8Mat4, eU16Mat4, eU32Mat4, eU64Mat4,
	eS8Mat4, eS16Mat4, eS32Mat4, eS64Mat4,
	                   eF32Mat4, eF64Mat4,
};
inline const char* to_cstring(ValueType type){
	switch(type) {
	case ValueType::eUndefined: return "undefined";
	
	case ValueType::eU8: return "u8";
	case ValueType::eU16: return "u16";
	case ValueType::eU32: return "u32";
	case ValueType::eU64: return "u64";
	case ValueType::eS8: return "s8";
	case ValueType::eS16: return "s16";
	case ValueType::eS32: return "s32";
	case ValueType::eS64: return "s64";
	case ValueType::eF32: return "f32";
	case ValueType::eF64: return "f64";
	
	case ValueType::eU8Vec2: return "u8Vec2";
	case ValueType::eU16Vec2: return "u16Vec2";
	case ValueType::eU32Vec2: return "u32Vec2";
	case ValueType::eU64Vec2: return "u64Vec2";
	case ValueType::eS8Vec2: return "s8Vec2";
	case ValueType::eS16Vec2: return "s16Vec2";
	case ValueType::eS32Vec2: return "s32Vec2";
	case ValueType::eS64Vec2: return "s64Vec2";
	case ValueType::eF32Vec2: return "f32Vec2";
	case ValueType::eF64Vec2: return "f64Vec2";
	
	case ValueType::eU8Vec3: return "u8Vec3";
	case ValueType::eU16Vec3: return "u16Vec3";
	case ValueType::eU32Vec3: return "u32Vec3";
	case ValueType::eU64Vec3: return "u64Vec3";
	case ValueType::eS8Vec3: return "s8Vec3";
	case ValueType::eS16Vec3: return "s16Vec3";
	case ValueType::eS32Vec3: return "s32Vec3";
	case ValueType::eS64Vec3: return "s64Vec3";
	case ValueType::eF32Vec3: return "f32Vec3";
	case ValueType::eF64Vec3: return "f64Vec3";
	
	case ValueType::eU8Vec4: return "u8Vec4";
	case ValueType::eU16Vec4: return "u16Vec4";
	case ValueType::eU32Vec4: return "u32Vec4";
	case ValueType::eU64Vec4: return "u64Vec4";
	case ValueType::eS8Vec4: return "s8Vec4";
	case ValueType::eS16Vec4: return "s16Vec4";
	case ValueType::eS32Vec4: return "s32Vec4";
	case ValueType::eS64Vec4: return "s64Vec4";
	case ValueType::eF32Vec4: return "f32Vec4";
	case ValueType::eF64Vec4: return "f64Vec4";

	case ValueType::eU8Mat2: return "u8Mat2";
	case ValueType::eU16Mat2: return "u16Mat2";
	case ValueType::eU32Mat2: return "u32Mat2";
	case ValueType::eU64Mat2: return "u64Mat2";
	case ValueType::eS8Mat2: return "s8Mat2";
	case ValueType::eS16Mat2: return "s16Mat2";
	case ValueType::eS32Mat2: return "s32Mat2";
	case ValueType::eS64Mat2: return "s64Mat2";
	case ValueType::eF32Mat2: return "f32Mat2";
	case ValueType::eF64Mat2: return "f64Mat2";
	
	case ValueType::eU8Mat3: return "u8Mat3";
	case ValueType::eU16Mat3: return "u16Mat3";
	case ValueType::eU32Mat3: return "u32Mat3";
	case ValueType::eU64Mat3: return "u64Mat3";
	case ValueType::eS8Mat3: return "s8Mat3";
	case ValueType::eS16Mat3: return "s16Mat3";
	case ValueType::eS32Mat3: return "s32Mat3";
	case ValueType::eS64Mat3: return "s64Mat3";
	case ValueType::eF32Mat3: return "f32Mat3";
	case ValueType::eF64Mat3: return "f64Mat3";
	
	case ValueType::eU8Mat4: return "u8Mat4";
	case ValueType::eU16Mat4: return "u16Mat4";
	case ValueType::eU32Mat4: return "u32Mat4";
	case ValueType::eU64Mat4: return "u64Mat4";
	case ValueType::eS8Mat4: return "s8Mat4";
	case ValueType::eS16Mat4: return "s16Mat4";
	case ValueType::eS32Mat4: return "s32Mat4";
	case ValueType::eS64Mat4: return "s64Mat4";
	case ValueType::eF32Mat4: return "f32Mat4";
	case ValueType::eF64Mat4: return "f64Mat4";
	}
	return "undefined Type";
}

enum class ImagePart {
	eColor,
	eDepth,
	eDepthStencil,
	eStencil,
};
//type U, S, Unorm, Snorm, Float
enum class ImageFormat {
	eUndefined,
	//color
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
	
	//depth
	eD16Unorm, eD32F, 
	//depthstencil
	eD24Unorm_St8U, eD32F_St8Uint,
	//stencil
	eSt8Uint
};
enum class ResourceType {
	eUndefined,
	eImage,
	eBuffer,
	
	eDataGroupDef,
	eContextBase,
	eModelBase,
	eModelInstanceBase,
	
	eModel,
	eModelInstance,
	eRenderPass,
};
