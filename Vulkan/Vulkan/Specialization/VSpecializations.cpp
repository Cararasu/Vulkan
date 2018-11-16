#include "VSpecializations.h"

#include "../VInstance.h"

DataGroupDefId vertex_datagroup_def_id;
DataGroupDefId matrix_datagroup_def_id;

ContextBaseId w2smatrix_base_id;
ContextBaseId m2wmatrix_base_id;

ModelBaseId simplemodel_base_id;

ModelInstanceBaseId simplemodel_instance_base_id;


void register_specializations ( VInstance* instance ) {

	DataGroupDef vertex_datagroup ( { {ValueType::eF32Vec3, 1, 0}, {ValueType::eF32Vec3, 1, sizeof ( glm::vec3 ) }, {ValueType::eF32Vec3, 1, 2 * sizeof ( glm::vec3 ) } }, 3 * sizeof ( glm::vec3 ), 1 );
	vertex_datagroup_def_id = instance->static_datagroup_store.insert ( vertex_datagroup );
	instance->datagroupdef_registered ( vertex_datagroup_def_id );

	DataGroupDef matrix_datagroup ( { {ValueType::eF32Mat4, 1, 0} }, sizeof ( glm::mat4 ), 1 );
	matrix_datagroup_def_id = instance->static_datagroup_store.insert ( matrix_datagroup );
	instance->datagroupdef_registered ( matrix_datagroup_def_id );
	
	//create contextbase from datagroups
	ContextBase w2smatrix_base ( matrix_datagroup_def_id );
	w2smatrix_base_id = instance->static_contextbase_store.insert ( w2smatrix_base );
	instance->contextbase_registered ( w2smatrix_base_id );

	ContextBase m2wmatrix_base ( matrix_datagroup_def_id );
	m2wmatrix_base_id = instance->static_contextbase_store.insert ( m2wmatrix_base );
	instance->contextbase_registered ( m2wmatrix_base_id );

	//create modelbase from datagroup and contextbases
	ModelBase simplemodel ( vertex_datagroup_def_id );
	simplemodel_base_id = instance->static_modelbase_store.insert ( simplemodel );
	instance->modelbase_registered ( simplemodel_base_id );

	//create modelbase from datagroup and contextbases
	ModelInstanceBase simplemodelinstance ( m2wmatrix_base_id, simplemodel_base_id );
	simplemodel_instance_base_id = instance->static_modelinstancebase_store.insert ( simplemodelinstance );
	instance->modelinstancebase_registered ( simplemodel_instance_base_id );

}
