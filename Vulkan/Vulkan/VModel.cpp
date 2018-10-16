#include "VModel.h"

VModel::VModel(VInstance* v_instance, VModelBase* modelbase) : 
	Model(modelbase), vertexbuffer(v_instance), indexbuffer(v_instance){
	
}
VModelBase::VModelBase(const DataGroupDef* datagroup) : ModelBase(datagroup) {
	
}

void VInstanceGroup::register_instances(const ModelInstanceBase* base, u32 count) {
	RId id = buffer_storeage.allocate_chunk(base->datagroup->size * count);
	base_to_id_map.insert(std::make_pair(base, id));
}
void VInstanceGroup::finish_register() {
	buffer_storeage.allocate_transferbuffer();
}
void* VInstanceGroup::get_data_ptr(const ModelInstanceBase* base) {
	return buffer_storeage.get_data_ptr(base_to_id_map[base]);
}
void VInstanceGroup::finish() {
	finished = false;
}
void VInstanceGroup::clear() {
	buffer_storeage.clear_transfer();
	base_to_id_map.clear();
}