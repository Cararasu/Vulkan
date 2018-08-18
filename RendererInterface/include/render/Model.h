#pragma once

#include "Header.h"

struct DataValueDef {
	ValueType type;
	u32 arraycount;
	u32 offset;
};
struct DataGroupDef {
	RId id;
	Array<DataValueDef> valuedefs;
	u32 size;
	u32 arraycount;
};

struct DataBlock {
	RId datadef_id;
	u8* dataptr;
};

struct ContextBase {
	RId id;
	RId datagroups;
	//TODO something something images and samplers
};
struct Context {
	RId context_id;
	DataBlock blocks;
};

struct ModelBase {
	RId id;
	RId datagroup;
};
struct Model {
	RId id;
	RId modelbase_id;
};

struct ModelInstanceBase{
	RId id;
};
struct ModelInstance {
	RId id;
	RId model_id;
	RId context_id;
};

#define GEN_SETTER(__VALUETYPE, __TYPE) inline void set_value ( u8* datablock, const DataGroupDef* groupdef, const __TYPE value, u32 group_index, u32 val_index, u32 array_index = 0 ) {\
		assert(group_index < groupdef->arraycount && val_index < groupdef->valuedefs.size() && array_index < groupdef->valuedefs[val_index].arraycount && groupdef->valuedefs[val_index].type == __VALUETYPE);\
		__TYPE* ptr = (__TYPE*)&datablock[groupdef->size * group_index + sizeof(__TYPE)*array_index + groupdef->valuedefs[val_index].offset];\
		memcpy(ptr, &value, sizeof(__TYPE));\
	}
GEN_SETTER ( ValueType::eU8, u8 )
GEN_SETTER ( ValueType::eU16, u16 )
GEN_SETTER ( ValueType::eU32, u32 )
GEN_SETTER ( ValueType::eU64, u64 )

GEN_SETTER ( ValueType::eS8, s8 )
GEN_SETTER ( ValueType::eS16, s16 )
GEN_SETTER ( ValueType::eS32, s32 )
GEN_SETTER ( ValueType::eS64, s64 )

GEN_SETTER ( ValueType::eF32, f32 )
GEN_SETTER ( ValueType::eF64, f64 )
#undef GEN_SETTER
#define GEN_SETTER(__VALUETYPE, __TYPE, __COUNT) inline void set_value_v##__COUNT ( u8* datablock, const DataGroupDef* groupdef, const __TYPE* valptr, u32 group_index, u32 val_index, u32 array_index = 0 ) {\
		assert(group_index < groupdef->arraycount && val_index < groupdef->valuedefs.size() && array_index < groupdef->valuedefs[val_index].arraycount && groupdef->valuedefs[val_index].type == __VALUETYPE);\
		__TYPE* ptr = (__TYPE*)&datablock[groupdef->size * group_index + __COUNT*sizeof(__TYPE)*array_index + groupdef->valuedefs[val_index].offset];\
		memcpy(ptr, valptr, __COUNT*sizeof(__TYPE));\
	}
GEN_SETTER ( ValueType::eU8, u8, 2 )
GEN_SETTER ( ValueType::eU8, u8, 3 )
GEN_SETTER ( ValueType::eU8, u8, 4 )
GEN_SETTER ( ValueType::eU16, u16, 2 )
GEN_SETTER ( ValueType::eU16, u16, 3 )
GEN_SETTER ( ValueType::eU16, u16, 4 )
GEN_SETTER ( ValueType::eU32, u32, 2 )
GEN_SETTER ( ValueType::eU32, u32, 3 )
GEN_SETTER ( ValueType::eU32, u32, 4 )
GEN_SETTER ( ValueType::eU64, u64, 2 )
GEN_SETTER ( ValueType::eU64, u64, 3 )
GEN_SETTER ( ValueType::eU64, u64, 4 )

GEN_SETTER ( ValueType::eS8, s8, 2 )
GEN_SETTER ( ValueType::eS8, s8, 3 )
GEN_SETTER ( ValueType::eS8, s8, 4 )
GEN_SETTER ( ValueType::eS16, s16, 2 )
GEN_SETTER ( ValueType::eS16, s16, 3 )
GEN_SETTER ( ValueType::eS16, s16, 4 )
GEN_SETTER ( ValueType::eS32, s32, 2 )
GEN_SETTER ( ValueType::eS32, s32, 3 )
GEN_SETTER ( ValueType::eS32, s32, 4 )
GEN_SETTER ( ValueType::eS64, s64, 2 )
GEN_SETTER ( ValueType::eS64, s64, 3 )
GEN_SETTER ( ValueType::eS64, s64, 4 )

GEN_SETTER ( ValueType::eF32, f32, 2 )
GEN_SETTER ( ValueType::eF32, f32, 3 )
GEN_SETTER ( ValueType::eF32, f32, 4 )
GEN_SETTER ( ValueType::eF64, f64, 2 )
GEN_SETTER ( ValueType::eF64, f64, 3 )
GEN_SETTER ( ValueType::eF64, f64, 4 )
#undef GEN_SETTER
#define GEN_SETTER(__VALUETYPE, __TYPE, __COUNT) inline void set_value_m##__COUNT ( u8* datablock, const DataGroupDef* groupdef, const __TYPE* valptr, u32 group_index, u32 val_index, u32 array_index = 0 ) {\
		assert(group_index < groupdef->arraycount && val_index < groupdef->valuedefs.size() && array_index < groupdef->valuedefs[val_index].arraycount && groupdef->valuedefs[val_index].type == __VALUETYPE);\
		__TYPE* ptr = (__TYPE*)&datablock[groupdef->size * group_index + __COUNT*__COUNT*sizeof(__TYPE)*array_index + groupdef->valuedefs[val_index].offset];\
		memcpy(ptr, valptr, __COUNT*__COUNT*sizeof(__TYPE));\
	}
GEN_SETTER ( ValueType::eU8, u8, 2 )
GEN_SETTER ( ValueType::eU8, u8, 3 )
GEN_SETTER ( ValueType::eU8, u8, 4 )
GEN_SETTER ( ValueType::eU16, u16, 2 )
GEN_SETTER ( ValueType::eU16, u16, 3 )
GEN_SETTER ( ValueType::eU16, u16, 4 )
GEN_SETTER ( ValueType::eU32, u32, 2 )
GEN_SETTER ( ValueType::eU32, u32, 3 )
GEN_SETTER ( ValueType::eU32, u32, 4 )
GEN_SETTER ( ValueType::eU64, u64, 2 )
GEN_SETTER ( ValueType::eU64, u64, 3 )
GEN_SETTER ( ValueType::eU64, u64, 4 )

GEN_SETTER ( ValueType::eS8, s8, 2 )
GEN_SETTER ( ValueType::eS8, s8, 3 )
GEN_SETTER ( ValueType::eS8, s8, 4 )
GEN_SETTER ( ValueType::eS16, s16, 2 )
GEN_SETTER ( ValueType::eS16, s16, 3 )
GEN_SETTER ( ValueType::eS16, s16, 4 )
GEN_SETTER ( ValueType::eS32, s32, 2 )
GEN_SETTER ( ValueType::eS32, s32, 3 )
GEN_SETTER ( ValueType::eS32, s32, 4 )
GEN_SETTER ( ValueType::eS64, s64, 2 )
GEN_SETTER ( ValueType::eS64, s64, 3 )
GEN_SETTER ( ValueType::eS64, s64, 4 )

GEN_SETTER ( ValueType::eF32, f32, 2 )
GEN_SETTER ( ValueType::eF32, f32, 3 )
GEN_SETTER ( ValueType::eF32, f32, 4 )
GEN_SETTER ( ValueType::eF64, f64, 2 )
GEN_SETTER ( ValueType::eF64, f64, 3 )
GEN_SETTER ( ValueType::eF64, f64, 4 )
#undef GEN_SETTER
