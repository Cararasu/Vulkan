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
	Undefined = 0,
	U8, U16, U32, U64,
	S8, S16, S32, S64,
	           F32, F64,

	U8Vec2, U16Vec2, U32Vec2, U64Vec2,
	S8Vec2, S16Vec2, S32Vec2, S64Vec2,
	                   F32Vec2, F64Vec2,

	U8Vec3, U16Vec3, U32Vec3, U64Vec3,
	S8Vec3, S16Vec3, S32Vec3, S64Vec3,
	                   F32Vec3, F64Vec3,

	U8Vec4, U16Vec4, U32Vec4, U64Vec4,
	S8Vec4, S16Vec4, S32Vec4, S64Vec4,
	                   F32Vec4, F64Vec4,

	U8Mat2, U16Mat2, U32Mat2, U64Mat2,
	S8Mat2, S16Mat2, S32Mat2, S64Mat2,
	                   F32Mat2, F64Mat2,

	U8Mat3, U16Mat3, U32Mat3, U64Mat3,
	S8Mat3, S16Mat3, S32Mat3, S64Mat3,
	                   F32Mat3, F64Mat3,

	U8Mat4, U16Mat4, U32Mat4, U64Mat4,
	S8Mat4, S16Mat4, S32Mat4, S64Mat4,
	                   F32Mat4, F64Mat4,
};
inline const char* to_cstring(ValueType type){
	switch(type) {
	case ValueType::Undefined: return "undefined";
	
	case ValueType::U8: return "u8";
	case ValueType::U16: return "u16";
	case ValueType::U32: return "u32";
	case ValueType::U64: return "u64";
	case ValueType::S8: return "s8";
	case ValueType::S16: return "s16";
	case ValueType::S32: return "s32";
	case ValueType::S64: return "s64";
	case ValueType::F32: return "f32";
	case ValueType::F64: return "f64";
	
	case ValueType::U8Vec2: return "u8Vec2";
	case ValueType::U16Vec2: return "u16Vec2";
	case ValueType::U32Vec2: return "u32Vec2";
	case ValueType::U64Vec2: return "u64Vec2";
	case ValueType::S8Vec2: return "s8Vec2";
	case ValueType::S16Vec2: return "s16Vec2";
	case ValueType::S32Vec2: return "s32Vec2";
	case ValueType::S64Vec2: return "s64Vec2";
	case ValueType::F32Vec2: return "f32Vec2";
	case ValueType::F64Vec2: return "f64Vec2";
	
	case ValueType::U8Vec3: return "u8Vec3";
	case ValueType::U16Vec3: return "u16Vec3";
	case ValueType::U32Vec3: return "u32Vec3";
	case ValueType::U64Vec3: return "u64Vec3";
	case ValueType::S8Vec3: return "s8Vec3";
	case ValueType::S16Vec3: return "s16Vec3";
	case ValueType::S32Vec3: return "s32Vec3";
	case ValueType::S64Vec3: return "s64Vec3";
	case ValueType::F32Vec3: return "f32Vec3";
	case ValueType::F64Vec3: return "f64Vec3";
	
	case ValueType::U8Vec4: return "u8Vec4";
	case ValueType::U16Vec4: return "u16Vec4";
	case ValueType::U32Vec4: return "u32Vec4";
	case ValueType::U64Vec4: return "u64Vec4";
	case ValueType::S8Vec4: return "s8Vec4";
	case ValueType::S16Vec4: return "s16Vec4";
	case ValueType::S32Vec4: return "s32Vec4";
	case ValueType::S64Vec4: return "s64Vec4";
	case ValueType::F32Vec4: return "f32Vec4";
	case ValueType::F64Vec4: return "f64Vec4";

	case ValueType::U8Mat2: return "u8Mat2";
	case ValueType::U16Mat2: return "u16Mat2";
	case ValueType::U32Mat2: return "u32Mat2";
	case ValueType::U64Mat2: return "u64Mat2";
	case ValueType::S8Mat2: return "s8Mat2";
	case ValueType::S16Mat2: return "s16Mat2";
	case ValueType::S32Mat2: return "s32Mat2";
	case ValueType::S64Mat2: return "s64Mat2";
	case ValueType::F32Mat2: return "f32Mat2";
	case ValueType::F64Mat2: return "f64Mat2";
	
	case ValueType::U8Mat3: return "u8Mat3";
	case ValueType::U16Mat3: return "u16Mat3";
	case ValueType::U32Mat3: return "u32Mat3";
	case ValueType::U64Mat3: return "u64Mat3";
	case ValueType::S8Mat3: return "s8Mat3";
	case ValueType::S16Mat3: return "s16Mat3";
	case ValueType::S32Mat3: return "s32Mat3";
	case ValueType::S64Mat3: return "s64Mat3";
	case ValueType::F32Mat3: return "f32Mat3";
	case ValueType::F64Mat3: return "f64Mat3";
	
	case ValueType::U8Mat4: return "u8Mat4";
	case ValueType::U16Mat4: return "u16Mat4";
	case ValueType::U32Mat4: return "u32Mat4";
	case ValueType::U64Mat4: return "u64Mat4";
	case ValueType::S8Mat4: return "s8Mat4";
	case ValueType::S16Mat4: return "s16Mat4";
	case ValueType::S32Mat4: return "s32Mat4";
	case ValueType::S64Mat4: return "s64Mat4";
	case ValueType::F32Mat4: return "f32Mat4";
	case ValueType::F64Mat4: return "f64Mat4";
	}
	return "undefined Type";
}

enum class ImagePart {
	Color,
	Depth,
	DepthStencil,
	Stencil,
};
//type U, S, Unorm, Snorm, Float
enum class ImageFormat {
	Undefined,
	//color
	U8, U16, U32, U64,
	S8, S16, S32, S64,
	     F16, F32, F64,
	Unorm8, Unorm16,
	Snorm8, Snorm16,
	
	U8x2, U16x2, U32x2, U64x2,
	S8x2, S16x2, S32x2, S64x2,
	      F16x2, F32x2, F64x2,
	Unorm8x2, Unorm16x2,
	Snorm8x2, Snorm16x2,
	
	U8x4, U16x4, U32x4, U64x4,
	S8x4, S16x4, S32x4, S64x4,
	      F16x4, F32x4, F64x4,
	Unorm8x4, Unorm16x4,
	Snorm8x4, Snorm16x4,
	
	//srgb???
	
	U10x3_U2,
	S10x3_S2,
	Unorm10x3_Unorm2,
	Snorm10x3_Snorm2,
	
	//depth
	D16Unorm, D32F, 
	//depthstencil
	D16F_St8U, D24Unorm_St8U, D32F_St8U,
	
	//stencil
	St8U
};
enum class ResourceType {
	Undefined,
	Image,
	Buffer,
	
	DataGroupDef,
	ContextBase,
	ModelBase,
	ModelInstancBase,
	
	Model,
	ModelInstance,
	RenderPass,
};
