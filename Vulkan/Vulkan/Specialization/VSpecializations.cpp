#include "VSpecializations.h"

#include "../VInstance.h"

ContextBaseId camera_context_base_id;
ContextBaseId lightvector_base_id;
ContextBaseId m2wmatrix_base_id;
ContextBaseId m2smatrix_base_id;
ContextBaseId tex_simplemodel_context_base_id;
ContextBaseId flat_simplemodel_context_base_id;
ContextBaseId skybox_context_base_id;

ModelBaseId simplemodel_base_id;

InstanceBaseId textured_instance_base_id;
InstanceBaseId flat_instance_base_id;
InstanceBaseId skybox_instance_base_id;

ModelBaseId simplequad_base_id;
InstanceBaseId simplequad_instance_base_id;

void register_specializations ( VInstance* instance ) {

	DataGroupDef vertex_datagroup = { { {ValueType::eF32Vec3, 1, 0}, {ValueType::eF32Vec3, 1, sizeof ( glm::vec3 ) }, {ValueType::eF32Vec3, 1, 2 * sizeof ( glm::vec3 ) } }, 3 * sizeof ( glm::vec3 ), 1 };

	DataGroupDef vector_datagroup = { { {ValueType::eF32Vec4, 1, 0}, {ValueType::eF32Vec4, 1, 16} }, sizeof ( glm::vec4 ) * 2, 1 };

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

	ContextBase m2smatrix_base = { 0, matrix_datagroup };
	m2smatrix_base_id = instance->static_contextbase_store.insert ( m2smatrix_base );
	instance->contextbase_registered ( m2smatrix_base_id );

	ContextBase tex_simplemodel_context_base = { 0, simplemodel_datagroup, 1, 1 };
	tex_simplemodel_context_base_id = instance->static_contextbase_store.insert ( tex_simplemodel_context_base );
	instance->contextbase_registered ( tex_simplemodel_context_base_id );

	ContextBase skybox_simplemodel_context_base = { 0, matrix_datagroup, 1, 1 };
	skybox_context_base_id = instance->static_contextbase_store.insert ( skybox_simplemodel_context_base );
	instance->contextbase_registered ( skybox_context_base_id );

	ContextBase flat_simplemodel_context_base = { 0, vector_datagroup, 0, 0 };
	flat_simplemodel_context_base_id = instance->static_contextbase_store.insert ( flat_simplemodel_context_base );
	instance->contextbase_registered ( flat_simplemodel_context_base_id );

	//create modelbase from datagroup and contextbases
	ModelBase tex_simplemodel = { 0, vertex_datagroup, {tex_simplemodel_context_base_id, flat_simplemodel_context_base_id, skybox_context_base_id} };
	simplemodel_base_id = instance->static_modelbase_store.insert ( tex_simplemodel );
	instance->modelbase_registered ( simplemodel_base_id );

	//create modelbase from datagroup and contextbases
	InstanceBase texturedinstance = { 0, matrix_2_datagroup };
	textured_instance_base_id = instance->static_instancebase_store.insert ( texturedinstance );
	instance->instancebase_registered ( textured_instance_base_id );

	flat_instance_base_id = instance->static_instancebase_store.insert ( texturedinstance );
	instance->instancebase_registered ( flat_instance_base_id );

	InstanceBase skyboxinstance = { 0, simplemodel_datagroup };
	skybox_instance_base_id = instance->static_instancebase_store.insert ( skyboxinstance );
	instance->instancebase_registered ( skybox_instance_base_id );


}
