#pragma once

#include "Header.h"
#include <atomic>
#include "Resources.h"

class Image;
struct DataValueDef {
	ValueType type;
	u32 arraycount;
	u32 offset;
};
struct DataGroupDef {
	Array<DataValueDef> valuedefs;
	u32 size;
	u32 arraycount = 1;
	bool needs_write = false;
};
enum class TextureResourceType : u8 {
	Undefined,
	Image,
	Sampler,
	ImageSampled,
	BufferSampled
};
struct TextureResource {
	TextureResourceType type;
	u32 arraycount = 1;
	bool needs_write = false;
};

struct ContextBase {
	ContextBaseId id;
	DataGroupDef datagroup;
	Array<TextureResource> texture_resources;
};
struct Context {
	ContextId id;
	ContextBaseId contextbase_id;
};

struct ModelBase {
	ModelBaseId id;
	DataGroupDef datagroup;
	Array<ContextBaseId> contextbase_ids;
};
struct Model {
	ModelId id;
	ModelBaseId modelbase_id;
	Array<Context> contexts;
};

struct InstanceBase {
	InstanceBaseId id;
	DataGroupDef instance_datagroup;
};

struct InstanceGroup {
	InstanceGroup() {};
	virtual ~InstanceGroup() {};
	//returns offset
	virtual void register_instances ( InstanceBaseId instancebase_id, Model& model, void* data, u32 count ) = 0;
	virtual void register_instances ( InstanceBaseId instancebase_id, Model* model, u32 model_count, void* data, u32 count ) = 0;
	virtual void register_instances ( InstanceBaseId instancebase_id, Array<Model>& models, void* data, u32 count ) = 0;
	virtual void clear() = 0;
};
struct ContextGroup {
	ContextGroup() {};
	virtual ~ContextGroup() {};
	virtual void set_context ( Context& context ) = 0;
	virtual void remove_context ( ContextBaseId base_id ) = 0;
	virtual void clear() = 0;
};


#define GEN_SETTER(__VALUETYPE, __TYPE) inline void set_value ( void* datablock, const DataGroupDef* groupdef, const __TYPE value, u32 group_index, u32 val_index, u32 array_index = 0 ) {\
		assert(group_index < groupdef->arraycount && val_index < groupdef->valuedefs.size && array_index < groupdef->valuedefs[val_index].arraycount && groupdef->valuedefs[val_index].type == __VALUETYPE);\
		__TYPE* ptr = (__TYPE*)(((u8*)datablock) + (groupdef->size * group_index + sizeof(__TYPE)*array_index + groupdef->valuedefs[val_index].offset));\
		memcpy(ptr, &value, sizeof(__TYPE));\
	}
GEN_SETTER ( ValueType::U8, u8 )
GEN_SETTER ( ValueType::U16, u16 )
GEN_SETTER ( ValueType::U32, u32 )
GEN_SETTER ( ValueType::U64, u64 )

GEN_SETTER ( ValueType::S8, s8 )
GEN_SETTER ( ValueType::S16, s16 )
GEN_SETTER ( ValueType::S32, s32 )
GEN_SETTER ( ValueType::S64, s64 )

GEN_SETTER ( ValueType::F32, f32 )
GEN_SETTER ( ValueType::F64, f64 )
#undef GEN_SETTER
#define GEN_SETTER(__VALUETYPE, __TYPE, __COUNT) inline void set_value_v##__COUNT ( void* datablock, const DataGroupDef* groupdef, const __TYPE* valptr, u32 group_index, u32 val_index, u32 array_index = 0 ) {\
		assert(group_index < groupdef->arraycount && val_index < groupdef->valuedefs.size && array_index < groupdef->valuedefs[val_index].arraycount && groupdef->valuedefs[val_index].type == __VALUETYPE);\
		__TYPE* ptr = (__TYPE*)(((u8*)datablock) + (groupdef->size * group_index + __COUNT*sizeof(__TYPE)*array_index + groupdef->valuedefs[val_index].offset));\
		memcpy(ptr, valptr, __COUNT*sizeof(__TYPE));\
	}
GEN_SETTER ( ValueType::U8, u8, 2 )
GEN_SETTER ( ValueType::U8, u8, 3 )
GEN_SETTER ( ValueType::U8, u8, 4 )
GEN_SETTER ( ValueType::U16, u16, 2 )
GEN_SETTER ( ValueType::U16, u16, 3 )
GEN_SETTER ( ValueType::U16, u16, 4 )
GEN_SETTER ( ValueType::U32, u32, 2 )
GEN_SETTER ( ValueType::U32, u32, 3 )
GEN_SETTER ( ValueType::U32, u32, 4 )
GEN_SETTER ( ValueType::U64, u64, 2 )
GEN_SETTER ( ValueType::U64, u64, 3 )
GEN_SETTER ( ValueType::U64, u64, 4 )

GEN_SETTER ( ValueType::S8, s8, 2 )
GEN_SETTER ( ValueType::S8, s8, 3 )
GEN_SETTER ( ValueType::S8, s8, 4 )
GEN_SETTER ( ValueType::S16, s16, 2 )
GEN_SETTER ( ValueType::S16, s16, 3 )
GEN_SETTER ( ValueType::S16, s16, 4 )
GEN_SETTER ( ValueType::S32, s32, 2 )
GEN_SETTER ( ValueType::S32, s32, 3 )
GEN_SETTER ( ValueType::S32, s32, 4 )
GEN_SETTER ( ValueType::S64, s64, 2 )
GEN_SETTER ( ValueType::S64, s64, 3 )
GEN_SETTER ( ValueType::S64, s64, 4 )

GEN_SETTER ( ValueType::F32, f32, 2 )
GEN_SETTER ( ValueType::F32, f32, 3 )
GEN_SETTER ( ValueType::F32, f32, 4 )
GEN_SETTER ( ValueType::F64, f64, 2 )
GEN_SETTER ( ValueType::F64, f64, 3 )
GEN_SETTER ( ValueType::F64, f64, 4 )
#undef GEN_SETTER
#define GEN_SETTER(__VALUETYPE, __TYPE, __COUNT) inline void set_value_m##__COUNT ( void* datablock, const DataGroupDef* groupdef, const __TYPE* valptr, u32 group_index, u32 val_index, u32 array_index = 0 ) {\
		assert(group_index < groupdef->arraycount && val_index < groupdef->valuedefs.size && array_index < groupdef->valuedefs[val_index].arraycount && groupdef->valuedefs[val_index].type == __VALUETYPE);\
		__TYPE* ptr = (__TYPE*)(((u8*)datablock) + (groupdef->size * group_index + __COUNT*__COUNT*sizeof(__TYPE)*array_index + groupdef->valuedefs[val_index].offset));\
		memcpy(ptr, valptr, __COUNT*__COUNT*sizeof(__TYPE));\
	}
GEN_SETTER ( ValueType::U8, u8, 2 )
GEN_SETTER ( ValueType::U8, u8, 3 )
GEN_SETTER ( ValueType::U8, u8, 4 )
GEN_SETTER ( ValueType::U16, u16, 2 )
GEN_SETTER ( ValueType::U16, u16, 3 )
GEN_SETTER ( ValueType::U16, u16, 4 )
GEN_SETTER ( ValueType::U32, u32, 2 )
GEN_SETTER ( ValueType::U32, u32, 3 )
GEN_SETTER ( ValueType::U32, u32, 4 )
GEN_SETTER ( ValueType::U64, u64, 2 )
GEN_SETTER ( ValueType::U64, u64, 3 )
GEN_SETTER ( ValueType::U64, u64, 4 )

GEN_SETTER ( ValueType::S8, s8, 2 )
GEN_SETTER ( ValueType::S8, s8, 3 )
GEN_SETTER ( ValueType::S8, s8, 4 )
GEN_SETTER ( ValueType::S16, s16, 2 )
GEN_SETTER ( ValueType::S16, s16, 3 )
GEN_SETTER ( ValueType::S16, s16, 4 )
GEN_SETTER ( ValueType::S32, s32, 2 )
GEN_SETTER ( ValueType::S32, s32, 3 )
GEN_SETTER ( ValueType::S32, s32, 4 )
GEN_SETTER ( ValueType::S64, s64, 2 )
GEN_SETTER ( ValueType::S64, s64, 3 )
GEN_SETTER ( ValueType::S64, s64, 4 )

GEN_SETTER ( ValueType::F32, f32, 2 )
GEN_SETTER ( ValueType::F32, f32, 3 )
GEN_SETTER ( ValueType::F32, f32, 4 )
GEN_SETTER ( ValueType::F64, f64, 2 )
GEN_SETTER ( ValueType::F64, f64, 3 )
GEN_SETTER ( ValueType::F64, f64, 4 )
#undef GEN_SETTER
