#include "VSpecializations.h"

#include "../VInstance.h"

ContextBaseId camera_context_base_id;
ContextBaseId lightvector_base_id;
ContextBaseId m2wmatrix_base_id;
ContextBaseId simplemodel_context_base_id;

ModelBaseId simplemodel_base_id;
InstanceBaseId simplemodel_instance_base_id;

ModelBaseId simplequad_base_id;
InstanceBaseId simplequad_instance_base_id;

void register_specializations ( VInstance* instance ) {

	DataGroupDef vertex_datagroup = { { {ValueType::eF32Vec3, 1, 0}, {ValueType::eF32Vec3, 1, sizeof ( glm::vec3 ) }, {ValueType::eF32Vec3, 1, 2 * sizeof ( glm::vec3 ) } }, 3 * sizeof ( glm::vec3 ), 1 };
	
	DataGroupDef vector_datagroup = { { {ValueType::eF32Vec4, 1, 0} }, sizeof ( glm::vec4 ), 1 };
	
	DataGroupDef matrix_datagroup = { { {ValueType::eF32Mat4, 1, 0} }, sizeof ( glm::mat4 ), 1 };
	DataGroupDef matrix_2_datagroup = { { {ValueType::eF32Mat4, 2, 0} }, sizeof ( glm::mat4 ) * 2, 1 };
	DataGroupDef simplemodel_datagroup = { {}, 0, 0 };

	//create contextbase from datagroups
	ContextBase camera_context_base = { 0, matrix_datagroup };
	camera_context_base_id = instance->static_contextbase_store.insert ( camera_context_base );
	instance->contextbase_registered ( camera_context_base_id );
	
	ContextBase lightvector_base = { 0, vector_datagroup };
	lightvector_base_id = instance->static_contextbase_store.insert ( lightvector_base );
	instance->contextbase_registered ( lightvector_base_id );

	ContextBase m2wmatrix_base = { 0, matrix_datagroup };
	m2wmatrix_base_id = instance->static_contextbase_store.insert ( m2wmatrix_base );
	instance->contextbase_registered ( m2wmatrix_base_id );

	ContextBase simplemodel_context_base = { 0, simplemodel_datagroup, 1, 1 };
	simplemodel_context_base_id = instance->static_contextbase_store.insert ( simplemodel_context_base );
	instance->contextbase_registered ( simplemodel_context_base_id );

	//create modelbase from datagroup and contextbases
	ModelBase simplemodel = { 0, vertex_datagroup, {simplemodel_context_base_id} };
	simplemodel_base_id = instance->static_modelbase_store.insert ( simplemodel );
	instance->modelbase_registered ( simplemodel_base_id );

	//create modelbase from datagroup and contextbases
	InstanceBase simpleinstance = { 0, matrix_2_datagroup };
	simplemodel_instance_base_id = instance->static_instancebase_store.insert ( simpleinstance );
	instance->instancebase_registered ( simplemodel_instance_base_id );

}
