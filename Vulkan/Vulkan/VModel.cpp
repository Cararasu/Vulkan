#include "VModel.h"

VModel::VModel(VInstance* v_instance, VModelBase* modelbase) : 
	Model(modelbase), vertexbuffer(v_instance), indexbuffer(v_instance){
	
}
VModelBase::VModelBase(const DataGroupDef* datagroup) : ModelBase(datagroup) {
	
}

void VInstanceGroup::register_instances(ModelInstanceBase* base, u32 count) {
	RId id = buffer_storeage.allocate_chunk(base->datagroup->size * count);
	
}
void* VInstanceGroup::get_data_ptr(ModelInstanceBase* base) {
	//buffer_storeage.get_data_ptr(id);
}
void VInstanceGroup::clear() {
	
}