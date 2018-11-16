#include "VModel.h"

VModel::VModel(VInstance* v_instance, ModelBaseId modelbase_id) : 
	Model(modelbase_id), vertexbuffer(v_instance), indexbuffer(v_instance){
	
}
VModelBase::VModelBase(const DataGroupDefId datagroup_id) : ModelBase(datagroup_id) {
	
}

u64 VInstanceGroup::register_instances(const ModelInstanceBase* base, u32 count) {
	RId id = buffer_storeage.allocate_chunk(instance->datagroupdef(base->instance_datagroup_id)->size * count);
	base_to_id_map.insert(std::make_pair(base, id));
	return id;
}
void* VInstanceGroup::finish_register() {
	return buffer_storeage.allocate_transferbuffer();
}
void VInstanceGroup::clear() {
	buffer_storeage.clear_transfer();
	base_to_id_map.clear();
}