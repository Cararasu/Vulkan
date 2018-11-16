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
struct DataGroupDef : public IdHandle {
	Array<DataValueDef> valuedefs;
	u32 size;
	u32 arraycount;

	DataGroupDef ( Array<DataValueDef> valuedefs, u32 size, u32 arraycount ) :
		IdHandle(), valuedefs ( valuedefs ), size ( size ), arraycount ( arraycount ) {

	}
};

struct ContextBase : public IdHandle {
	DataGroupDefId datagroup_id;
	//TODO something something images and samplers

	ContextBase ( const ContextBase&& contextbase ) : IdHandle ( contextbase ), datagroup_id ( contextbase.datagroup_id ) { }
	ContextBase ( const ContextBase& contextbase ) : IdHandle ( contextbase ), datagroup_id ( contextbase.datagroup_id ) { }
	ContextBase& operator= ( const ContextBase&& contextbase ) {
		this->hash = contextbase.hash;
		this->datagroup_id = contextbase.datagroup_id;
		return *this;
	}
	ContextBase& operator= ( const ContextBase& contextbase ) {
		this->hash = contextbase.hash;
		this->datagroup_id = contextbase.datagroup_id;
		return *this;
	}
	ContextBase ( const DataGroupDefId datagroup_id ) : IdHandle(), datagroup_id ( datagroup_id ) { }
	ContextBase ( IdHandle idhandle, const DataGroupDefId datagroup_id ) : IdHandle ( idhandle ), datagroup_id ( datagroup_id ) { }
};
struct Context : public IdHandle {
	ContextBaseId contextbase_id;

	Context ( Context&& context ) : IdHandle ( context ), contextbase_id ( context.contextbase_id ) { }
	Context ( const Context& context ) : IdHandle ( context ), contextbase_id ( context.contextbase_id ) { }
	Context ( const ContextBaseId contextbase_id ) : IdHandle(), contextbase_id ( contextbase_id ) { }
	Context ( IdHandle idhandle, const ContextBaseId contextbase_id ) : IdHandle ( idhandle ), contextbase_id ( contextbase_id ) { }
};

struct ModelBase : public IdHandle {
	DataGroupDefId datagroup_id;

	ModelBase ( const DataGroupDefId datagroup_id ) : IdHandle(), datagroup_id ( datagroup_id ) { }
	ModelBase ( IdHandle idhandle, const DataGroupDefId datagroup_id ) : IdHandle ( idhandle ), datagroup_id ( datagroup_id ) { }
};
struct Model : public IdHandle {
	ModelBaseId modelbase_id;

	Model ( Model&& model ) : IdHandle ( model ), modelbase_id ( model.modelbase_id ) { }
	Model ( const Model& model ) : IdHandle ( model ), modelbase_id ( model.modelbase_id ) { }
	Model ( const ModelBaseId modelbase_id ) : IdHandle(), modelbase_id ( modelbase_id ) { }
	Model ( IdHandle idhandle, const ModelBaseId modelbase_id ) : IdHandle ( idhandle ), modelbase_id ( modelbase_id ) { }
};

struct ModelInstanceBase : public IdHandle {
	DataGroupDefId instance_datagroup_id;
	ModelBaseId modelbase_id;

	ModelInstanceBase ( const DataGroupDefId instance_datagroup_id, const ModelBaseId modelbase_id ) :
		IdHandle(), instance_datagroup_id ( instance_datagroup_id ), modelbase_id ( modelbase_id ) { }
	ModelInstanceBase ( IdHandle idhandle, const DataGroupDefId instance_datagroup_id, const ModelBaseId modelbase_id ) : 
		IdHandle ( idhandle ), instance_datagroup_id ( instance_datagroup_id ), modelbase_id ( modelbase_id ) { }
};

struct InstanceGroup {
	InstanceGroup() {};
	virtual ~InstanceGroup() {};
	//returns offset
	virtual u64 register_instances ( const ModelInstanceBase* base, u32 count ) = 0;
	//get base offset
	virtual void* finish_register () = 0;
	virtual void clear() = 0;
};
struct ContextGroup {
	ContextGroup() {};
	virtual ~ContextGroup() {};
	virtual void set_context ( Context* context ) = 0;
	virtual void remove_context ( ContextBaseId base_id ) = 0;
	virtual void clear() = 0;
};

struct RenderBundle {

	virtual ~RenderBundle() {}

	virtual void set_rendertarget ( u32 index, Image* image ) = 0;
};

#define GEN_SETTER(__VALUETYPE, __TYPE) inline void set_value ( void* datablock, const DataGroupDef* groupdef, const __TYPE value, u32 group_index, u32 val_index, u32 array_index = 0 ) {\
		assert(group_index < groupdef->arraycount && val_index < groupdef->valuedefs.size && array_index < groupdef->valuedefs[val_index].arraycount && groupdef->valuedefs[val_index].type == __VALUETYPE);\
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
		assert(group_index < groupdef->arraycount && val_index < groupdef->valuedefs.size && array_index < groupdef->valuedefs[val_index].arraycount && groupdef->valuedefs[val_index].type == __VALUETYPE);\
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
		assert(group_index < groupdef->arraycount && val_index < groupdef->valuedefs.size && array_index < groupdef->valuedefs[val_index].arraycount && groupdef->valuedefs[val_index].type == __VALUETYPE);\
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
