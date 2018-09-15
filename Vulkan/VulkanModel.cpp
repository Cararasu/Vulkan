#include "VulkanModel.h"

VulkanModel::VulkanModel(VulkanInstance* v_instance, VulkanModelBase* modelbase) : 
	Model(modelbase), vertexbuffer(v_instance), indexbuffer(v_instance){
	
}
VulkanModelBase::VulkanModelBase(const DataGroupDef* datagroup) : ModelBase(datagroup) {
	
}

ModelInstance VulkanInstanceGroup::add_instance(ModelInstanceBase* base) {
	
}
void* VulkanInstanceGroup::get_data_ptr(ModelInstance instance) {
	
}
void VulkanInstanceGroup::clear() {
	
}