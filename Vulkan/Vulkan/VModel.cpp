#include "VModel.h"

VModel::VModel(VInstance* v_instance, VModelBase* modelbase) : 
	Model(modelbase), vertexbuffer(v_instance), indexbuffer(v_instance){
	
}
VModelBase::VModelBase(const DataGroupDef* datagroup) : ModelBase(datagroup) {
	
}

u64 VInstanceGroup::register_instances(const ModelInstanceBase* base, u32 count) {
	RId id = buffer_storeage.allocate_chunk(base->datagroup->size * count);
	base_to_id_map.insert(std::make_pair(base, id));
}
void* VInstanceGroup::finish_register() {
	return buffer_storeage.allocate_transferbuffer();
}
void VInstanceGroup::clear() {
	buffer_storeage.clear_transfer();
	base_to_id_map.clear();
}