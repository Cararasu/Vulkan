#include "VulkanModel.h"

VulkanModel::VulkanModel(VulkanInstance* v_instance, RId modelbase) : id(0), modelbase(modelbase), vertexbuffer(v_instance), indexbuffer(v_instance){
	
}
VulkanModelBase::VulkanModelBase(RId datagroup) : datagroup(datagroup) {
	
}