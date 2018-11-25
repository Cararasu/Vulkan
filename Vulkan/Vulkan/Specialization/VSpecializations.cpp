#include "VSpecializations.h"

#include "../VInstance.h"

ContextBaseId camera_context_base_id;
ContextBaseId lightvector_base_id;
ContextBaseId tex_simplemodel_context_base_id;
ContextBaseId flat_simplemodel_context_base_id;
ContextBaseId skybox_context_base_id;

ModelBaseId simple_modelbase_id;
ModelBaseId shot_modelbase_id;

InstanceBaseId textured_instance_base_id;
InstanceBaseId flat_instance_base_id;
InstanceBaseId skybox_instance_base_id;
InstanceBaseId shot_instance_base_id;

ModelBaseId simplequad_base_id;
InstanceBaseId simplequad_instance_base_id;

void register_specializations ( VInstance* instance ) {

	DataGroupDef vertex_datagroup = { { {ValueType::eF32Vec3, 1, 0}, {ValueType::eF32Vec3, 1, sizeof ( glm::vec3 ) }, {ValueType::eF32Vec3, 1, 2 * sizeof ( glm::vec3 ) } }, 3 * sizeof ( glm::vec3 ), 1 };

	DataGroupDef vector1_datagroup = { { {ValueType::eF32Vec4, 1, 0}}, sizeof ( glm::vec4 ), 1 };
	DataGroupDef vector2_datagroup = { { {ValueType::eF32Vec4, 1, 0}, {ValueType::eF32Vec4, 1, 16} }, sizeof ( glm::vec4 ) * 2, 1 };

	DataGroupDef camera_datagroup = { { {ValueType::eF32Mat4, 1, 0}, {ValueType::eF32Vec4, 1, 64} }, sizeof ( glm::mat4 ) + sizeof ( glm::vec4 ), 1 };
	
	DataGroupDef shot_datagroup = { { {ValueType::eF32Mat4, 1, 0}, {ValueType::eF32Vec4, 2, 64} }, sizeof ( glm::mat4 ) + sizeof ( glm::vec4 )*2, 1 };
	DataGroupDef matrix_datagroup = { { {ValueType::eF32Mat4, 1, 0} }, sizeof ( glm::mat4 ), 1 };
	DataGroupDef matrix_2_datagroup = { { {ValueType::eF32Mat4, 2, 0} }, sizeof ( glm::mat4 ) * 2, 1 };

	DataGroupDef empty_datagroup = { {}, 0, 0 };

	//create contextbase from datagroups
	ContextBase camera_context_base = { 0, camera_datagroup };
	camera_context_base_id = instance->static_contextbase_store.insert ( camera_context_base );
	instance->contextbase_registered ( camera_context_base_id );

	ContextBase lightvector_base = { 0, vector2_datagroup };
	lightvector_base_id = instance->static_contextbase_store.insert ( lightvector_base );
	instance->contextbase_registered ( lightvector_base_id );

	ContextBase tex_simplemodel_context_base = { 0, empty_datagroup, 1, 1 };
	tex_simplemodel_context_base_id = instance->static_contextbase_store.insert ( tex_simplemodel_context_base );
	instance->contextbase_registered ( tex_simplemodel_context_base_id );

	ContextBase skybox_simplemodel_context_base = { 0, matrix_datagroup, 1, 1 };
	skybox_context_base_id = instance->static_contextbase_store.insert ( skybox_simplemodel_context_base );
	instance->contextbase_registered ( skybox_context_base_id );

	ContextBase flat_simplemodel_context_base = { 0, vector2_datagroup, 0, 0 };
	flat_simplemodel_context_base_id = instance->static_contextbase_store.insert ( flat_simplemodel_context_base );
	instance->contextbase_registered ( flat_simplemodel_context_base_id );

	//create modelbase from datagroup and contextbases
	ModelBase tex_simplemodel = { 0, vertex_datagroup, {tex_simplemodel_context_base_id, flat_simplemodel_context_base_id, skybox_context_base_id} };
	simple_modelbase_id = instance->static_modelbase_store.insert ( tex_simplemodel );
	instance->modelbase_registered ( simple_modelbase_id );

	ModelBase shot_simplemodel = { 0, vector1_datagroup, {} };
	shot_modelbase_id = instance->static_modelbase_store.insert ( shot_simplemodel );
	instance->modelbase_registered ( shot_modelbase_id );

	//create modelbase from datagroup and contextbases
	InstanceBase texturedinstance = { 0, matrix_2_datagroup };
	textured_instance_base_id = instance->static_instancebase_store.insert ( texturedinstance );
	instance->instancebase_registered ( textured_instance_base_id );

	flat_instance_base_id = instance->static_instancebase_store.insert ( texturedinstance );
	instance->instancebase_registered ( flat_instance_base_id );

	InstanceBase skyboxinstance = { 0, empty_datagroup };
	skybox_instance_base_id = instance->static_instancebase_store.insert ( skyboxinstance );
	instance->instancebase_registered ( skybox_instance_base_id );

	InstanceBase shotinstance = { 0, shot_datagroup };
	shot_instance_base_id = instance->static_instancebase_store.insert ( shotinstance );
	instance->instancebase_registered ( shot_instance_base_id );

}
