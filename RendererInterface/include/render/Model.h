#pragma once

#include "Model.h"
#include "Header.h"

struct DataValueDef {
	ValueType type;
	u32 offset;
};
struct DataGroupDef {
	Array<DataValueDef> valuedefs;
};

struct ModelBase {
	RId id;
	//define the layout of the vertexdata
	Array<DataGroupDef> vertex_input_defs;
};
struct InstanceBase {
	RId id;
	//define the layout of the instancedata
	Array<DataGroupDef> instance_input_defs;
};

struct Model {
	RId id;
	RId modelbase_id;
	//and the indices
	ValueType indextype;
};

struct ModelInstance {
	RId id;
	RId model_id;
	RId instancebase_id;
	//has a list of per object data, which allows usage for different stuff

};

inline ResourceHandle resource_handle ( ModelBase* base ) {
	return {ResourceType::eModelBase, base->id};
}
inline ResourceHandle resource_handle ( Model* model ) {
	return {ResourceType::eModel, model->id};
}
inline ResourceHandle resource_handle ( ModelInstance* instance ) {
	return {ResourceType::eModelInstance, instance->id};
}
