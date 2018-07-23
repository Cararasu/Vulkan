#pragma once

#include "Model.h"
#include "Header.h"

struct DataValueDef {
	ValueType type;
	u32 offset;
};
struct DataGroupDef {
	Array<DataValueDef> valuedefs;
	u32 size;
};
struct ModelBaseDef {
	RId id;
	Array<DataGroupDef> vertex_input_defs;
};
struct ModelBase {
	const ModelBaseDef def;
};
struct ContextBaseDef {
	RId id;
	Array<DataGroupDef> vertex_input_defs;
};
struct ContextBase {
	const ContextBaseDef def;
};
struct InstanceBaseDef {
	RId id;
	Array<DataGroupDef> instance_input_defs;
};
struct InstanceBase {
	const InstanceBaseDef def;
};

struct ModelDef {
	RId id;
	RId modelbase_id;
	ValueType indextype;
};
struct Model {
	const ModelDef def;
};

struct ModelInstanceDef {
	RId id;
	RId model_id;
	RId instancebase_id;
	RId contextbase_id;
};
struct ModelInstance {
	const ModelInstanceDef def;
};


inline ResourceHandle resource_handle ( ModelBase* base ) {
	return {ResourceType::eModelBase, base->def.id};
}
inline ResourceHandle resource_handle ( InstanceBase* base ) {
	return {ResourceType::eInstanceBase, base->def.id};
}
inline ResourceHandle resource_handle ( ContextBase* base ) {
	return {ResourceType::eContextBase, base->def.id};
}
inline ResourceHandle resource_handle ( Model* model ) {
	return {ResourceType::eModel, model->def.id};
}
inline ResourceHandle resource_handle ( ModelInstance* instance ) {
	return {ResourceType::eModelInstance, instance->def.id};
}
