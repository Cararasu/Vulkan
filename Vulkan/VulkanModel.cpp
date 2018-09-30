#include "VulkanModel.h"

VulkanModel::VulkanModel(VulkanInstance* v_instance, VulkanModelBase* modelbase) : 
	Model(modelbase), vertexbuffer(v_instance), indexbuffer(v_instance){
	
}
VulkanModelBase::VulkanModelBase(const DataGroupDef* datagroup) : ModelBase(datagroup) {
	
}

void VulkanInstanceGroup::register_instances(ModelInstanceBase* base, u32 count) {
	
}
void* VulkanInstanceGroup::get_data_ptr(ModelInstanceBase* base) {
	
}
void VulkanInstanceGroup::clear() {
	
}