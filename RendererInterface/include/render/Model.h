#pragma once

#include "Header.h"
#include <atomic>

struct DataValueDef {
	ValueType type;
	u32 arraycount;
	u32 offset;
};
struct DataGroupDef : public IdHandle {
	Array<DataValueDef> valuedefs;
	u32 size;
	u32 arraycount;
	mutable std::atomic<u32> uses;

	DataGroupDef ( Array<DataValueDef> valuedefs, u32 size, u32 arraycount ) :
		IdHandle(), valuedefs ( valuedefs ), size ( size ), arraycount ( arraycount ), uses ( 0 ) {

	}
};

struct ContextBase : public IdHandle {
	const DataGroupDef* datagroup;
	mutable std::atomic<u32> uses;
	//TODO something something images and samplers

	ContextBase ( const DataGroupDef* datagroup ) : IdHandle(), datagroup ( datagroup ), uses ( 0 ) { }
	ContextBase ( IdHandle idhandle, const DataGroupDef* datagroup ) : IdHandle ( idhandle ), datagroup ( datagroup ), uses ( 0 ) { }
};
struct Context : public IdHandle {
	RId id;
	const ContextBase* contextbase;

	Context ( Context&& context ) : IdHandle ( context ), contextbase ( context.contextbase ) { }
	Context ( const Context& context ) : IdHandle ( context ), contextbase ( context.contextbase ) { }
	Context ( const ContextBase* contextbase ) : IdHandle(), contextbase ( contextbase ) { }
	Context ( IdHandle idhandle, const ContextBase* contextbase ) : IdHandle ( idhandle ), contextbase ( contextbase ) { }
};

struct ModelBase : public IdHandle {
	const DataGroupDef* datagroup;
	mutable std::atomic<u32> uses;

	ModelBase ( const DataGroupDef* datagroup ) : IdHandle(), datagroup ( datagroup ) { }
	ModelBase ( IdHandle idhandle, const DataGroupDef* datagroup ) : IdHandle ( idhandle ), datagroup ( datagroup ) { }
};
struct Model : public IdHandle {
	const ModelBase* modelbase;

	Model ( Model&& model ) : IdHandle ( model ), modelbase ( model.modelbase ) { }
	Model ( const Model& model ) : IdHandle ( model ), modelbase ( model.modelbase ) { }
	Model ( const ModelBase* modelbase ) : IdHandle(), modelbase ( modelbase ) { }
	Model ( IdHandle idhandle, const ModelBase* modelbase ) : IdHandle ( idhandle ), modelbase ( modelbase ) { }
};

struct ModelInstanceBase : public IdHandle {
	const DataGroupDef* datagroup;
	const Model model;
	mutable std::atomic<u32> uses;

	ModelInstanceBase ( const DataGroupDef* datagroup, const Model model ) : IdHandle(), datagroup ( datagroup ), model ( model ) { }
	ModelInstanceBase ( IdHandle idhandle, const DataGroupDef* datagroup, const Model model ) : IdHandle ( idhandle ), datagroup ( datagroup ), model ( model ) { }
};

struct InstanceGroup {
	virtual void register_instances ( ModelInstanceBase* base, u32 count ) = 0;
	virtual void* get_data_ptr ( ModelInstanceBase* base ) = 0;
	virtual void clear() = 0;
};
struct ContextGroup {
	virtual void set_context ( Context* context ) = 0;
	virtual void remove_context ( ContextBase* base ) = 0;
	virtual void clear() = 0;
};


#define GEN_SETTER(__VALUETYPE, __TYPE) inline void set_value ( void* datablock, const DataGroupDef* groupdef, const __TYPE value, u32 group_index, u32 val_index, u32 array_index = 0 ) {\
		assert(group_index < groupdef->arraycount && val_index < groupdef->valuedefs.size() && array_index < groupdef->valuedefs[val_index].arraycount && groupdef->valuedefs[val_index].type == __VALUETYPE);\
		__TYPE* ptr = (__TYPE*)(((u8*)datablock) + (groupdef->size * group_index + sizeof(__TYPE)*array_index + groupdef->valuedefs[val_index].offset));\
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
#define GEN_SETTER(__VALUETYPE, __TYPE, __COUNT) inline void set_value_v##__COUNT ( void* datablock, const DataGroupDef* groupdef, const __TYPE* valptr, u32 group_index, u32 val_index, u32 array_index = 0 ) {\
		assert(group_index < groupdef->arraycount && val_index < groupdef->valuedefs.size() && array_index < groupdef->valuedefs[val_index].arraycount && groupdef->valuedefs[val_index].type == __VALUETYPE);\
		__TYPE* ptr = (__TYPE*)(((u8*)datablock) + (groupdef->size * group_index + __COUNT*sizeof(__TYPE)*array_index + groupdef->valuedefs[val_index].offset));\
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
#define GEN_SETTER(__VALUETYPE, __TYPE, __COUNT) inline void set_value_m##__COUNT ( void* datablock, const DataGroupDef* groupdef, const __TYPE* valptr, u32 group_index, u32 val_index, u32 array_index = 0 ) {\
		assert(group_index < groupdef->arraycount && val_index < groupdef->valuedefs.size() && array_index < groupdef->valuedefs[val_index].arraycount && groupdef->valuedefs[val_index].type == __VALUETYPE);\
		__TYPE* ptr = (__TYPE*)(((u8*)datablock) + (groupdef->size * group_index + __COUNT*__COUNT*sizeof(__TYPE)*array_index + groupdef->valuedefs[val_index].offset));\
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
