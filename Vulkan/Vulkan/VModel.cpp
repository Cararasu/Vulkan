#include "VModel.h"
#include "VInstance.h"

VModel::VModel ( VInstance* instance, ModelBaseId modelbase_id ) :
	id ( 0 ), modelbase_id ( modelbase_id ), v_instance ( instance ), vertexbuffer ( instance ), indexbuffer ( instance ) {
	const ModelBase* modelbase_ptr = instance->modelbase ( modelbase_id );
	contexts = modelbase_ptr->contextbase_ids;
	v_contexts.resize ( modelbase_ptr->contextbase_ids.size );
	for ( int i = 0; i < v_contexts.size; i++ )
		v_contexts[i] = nullptr;
}
void VInstanceGroup::register_instances ( InstanceBaseId instancebase_id, Model& model, void* data, u32 count ) {
	const InstanceBase* instancebaseptr = v_instance->instancebase ( instancebase_id );
	u64 offset = buffer_storeage.allocate_chunk ( instancebaseptr->instance_datagroup.size * count );
	instance_to_data_map[instancebase_id].push_back ( {instancebase_id, model.id, model.modelbase_id, offset, data, count} );
}
void VInstanceGroup::register_instances ( InstanceBaseId instancebase_id, Model* model, u32 model_count, void* data, u32 count ) {
	const InstanceBase* instancebaseptr = v_instance->instancebase ( instancebase_id );
	u64 offset = buffer_storeage.allocate_chunk ( instancebaseptr->instance_datagroup.size * count );
	for ( u32 i = 0; i < model_count; i++ ) {
		instance_to_data_map[instancebase_id].push_back ( {instancebase_id, model[i].id, model[i].modelbase_id, offset, data, count} );
	}
}
void VInstanceGroup::register_instances ( InstanceBaseId instancebase_id, Array<Model>& models, void* data, u32 count ) {
	const InstanceBase* instancebaseptr = v_instance->instancebase ( instancebase_id );
	u64 offset = buffer_storeage.allocate_chunk ( instancebaseptr->instance_datagroup.size * count );
	for ( Model& model : models ) {
		instance_to_data_map[instancebase_id].push_back ( {instancebase_id, model.id, model.modelbase_id, offset, data, count} );
	}
}
void VInstanceGroup::clear() {
	buffer_storeage.clear_transfer();
	for(auto& ele : instance_to_data_map) {
		ele.second.clear();
	}
}
