#include "VSpecializations.h"

#include "../VInstance.h"

ContextBaseId camera_context_base_id;

ContextBaseId lightvector_base_id;
ContextBaseId tex_simplemodel_context_base_id;
ContextBaseId flat_simplemodel_context_base_id;
ContextBaseId skybox_context_base_id;
ContextBaseId explosion_context_base_id;
ContextBaseId shadowmap_context_base_id;

ContextBaseId postproc_context_base_id;

ModelBaseId simple_modelbase_id;

ModelBaseId fullscreen_modelbase_id;

InstanceBaseId textured_instance_base_id;
InstanceBaseId flat_instance_base_id;
InstanceBaseId skybox_instance_base_id;
InstanceBaseId shot_instance_base_id;
InstanceBaseId billboard_instance_base_id;

InstanceBaseId dirlight_instance_base_id;
InstanceBaseId single_instance_base_id;
InstanceBaseId bloom_instance_base_id;

ModelBaseId simplequad_base_id;
InstanceBaseId simplequad_instance_base_id;


void register_specializations ( VInstance* instance ) {

	{
		//create contextbase from datagroups
		ContextBase camera_context_base = { 0, 
			{ { {ValueType::F32Mat4, 1, 0}, {ValueType::F32Mat4, 1, 64}, {ValueType::F32Mat4, 1, 128}, {ValueType::F32Mat4, 1, 192}, {ValueType::F32Vec4, 1, 256} }, 
			sizeof ( glm::mat4 ) * 4 + sizeof ( glm::vec4 ), 1 }
		};
		camera_context_base_id = instance->static_contextbase_store.insert ( camera_context_base );
		instance->contextbase_registered ( camera_context_base_id );

		ContextBase lightvector_base = { 0, { { {ValueType::F32Vec4, 1, 0}, {ValueType::F32Vec4, 1, 16} }, sizeof ( glm::vec4 ) * 2, 1 } };
		lightvector_base_id = instance->static_contextbase_store.insert ( lightvector_base );
		instance->contextbase_registered ( lightvector_base_id );

		ContextBase tex_simplemodel_context_base = { 0, { {}, 0, 0 }, {{TextureResourceType::ImageSampled}}};
		tex_simplemodel_context_base_id = instance->static_contextbase_store.insert ( tex_simplemodel_context_base );
		instance->contextbase_registered ( tex_simplemodel_context_base_id );

		ContextBase skybox_simplemodel_context_base = { 0, 
			{ { {ValueType::F32Mat4, 1, 0} }, sizeof ( glm::mat4 ), 1 }, 
			{{TextureResourceType::ImageSampled}}
		};
		skybox_context_base_id = instance->static_contextbase_store.insert ( skybox_simplemodel_context_base );
		instance->contextbase_registered ( skybox_context_base_id );
		
		
		ContextBase explosion_simplemodel_context_base = { 0, 
			{ { }, 0, 1 }, 
			{{TextureResourceType::ImageSampled}}
		};
		explosion_context_base_id = instance->static_contextbase_store.insert ( explosion_simplemodel_context_base );
		instance->contextbase_registered ( explosion_context_base_id );

		ContextBase flat_simplemodel_context_base = { 0, { { {ValueType::F32Vec4, 1, 0}, {ValueType::F32Vec4, 1, 16} }, sizeof ( glm::vec4 ) * 2, 1 }, {} };
		flat_simplemodel_context_base_id = instance->static_contextbase_store.insert ( flat_simplemodel_context_base );
		instance->contextbase_registered ( flat_simplemodel_context_base_id );
		
		ContextBase bloom_context_base = { 0, {{}, 0, 0 }, { {TextureResourceType::ImageSampled} } };
		postproc_context_base_id = instance->static_contextbase_store.insert ( bloom_context_base );
		instance->contextbase_registered ( postproc_context_base_id );
		
		ContextBase shadowmap_context_base = { 0, { { {ValueType::F32Mat4, 3, 0}, {ValueType::F32Vec4, 3, 196} }, sizeof ( glm::mat4 ) * 3 + sizeof ( glm::vec4 ) * 3, 1 }, {{TextureResourceType::ImageSampled}} };
		shadowmap_context_base_id = instance->static_contextbase_store.insert ( shadowmap_context_base );
		instance->contextbase_registered ( shadowmap_context_base_id );
	}

	DataGroupDef vertex_datagroup = { { {ValueType::F32Vec3, 1, 0}, {ValueType::F32Vec3, 1, sizeof ( glm::vec3 ) }, {ValueType::F32Vec3, 1, 2 * sizeof ( glm::vec3 ) } }, 3 * sizeof ( glm::vec3 ), 1 };
	//create modelbase from datagroup and contextbases
	ModelBase tex_simplemodel = { 0, vertex_datagroup, {tex_simplemodel_context_base_id, flat_simplemodel_context_base_id, skybox_context_base_id, explosion_context_base_id} };
	simple_modelbase_id = instance->static_modelbase_store.insert ( tex_simplemodel );
	instance->modelbase_registered ( simple_modelbase_id );

	ModelBase fullscreen_model = { 0, { { {ValueType::F32Vec3, 1, 0}}, sizeof ( glm::vec3 ), 1 }, {} };
	fullscreen_modelbase_id = instance->static_modelbase_store.insert ( fullscreen_model );
	instance->modelbase_registered ( fullscreen_modelbase_id );

	//create modelbase from datagroup and contextbases
	InstanceBase texturedinstance = { 0, 
		{ { {ValueType::F32Mat4, 2, 0} }, sizeof ( glm::mat4 ) * 2, 1 }
	};
	textured_instance_base_id = instance->static_instancebase_store.insert ( texturedinstance );
	instance->instancebase_registered ( textured_instance_base_id );

	flat_instance_base_id = instance->static_instancebase_store.insert ( texturedinstance );
	instance->instancebase_registered ( flat_instance_base_id );

	InstanceBase skyboxinstance = { 0, { {}, 0, 0 } };
	skybox_instance_base_id = instance->static_instancebase_store.insert ( skyboxinstance );
	instance->instancebase_registered ( skybox_instance_base_id );

	InstanceBase dirlightinstance = { 0, { {}, 0, 0 } };
	dirlight_instance_base_id = instance->static_instancebase_store.insert ( dirlightinstance );
	instance->instancebase_registered ( dirlight_instance_base_id );
	
	single_instance_base_id = instance->static_instancebase_store.insert ( dirlightinstance );
	instance->instancebase_registered ( single_instance_base_id );
	
	bloom_instance_base_id = instance->static_instancebase_store.insert ( dirlightinstance );
	instance->instancebase_registered ( bloom_instance_base_id );

	shot_instance_base_id = instance->static_instancebase_store.insert ( 
		{ 0, 
			{ { {ValueType::F32Mat4, 1, 0}, {ValueType::F32Vec4, 1, 64}}, sizeof ( glm::mat4 ) + sizeof ( glm::vec4 ), 1 } 
		}
	);
	instance->instancebase_registered ( shot_instance_base_id );

	billboard_instance_base_id = instance->static_instancebase_store.insert ( 
		{ 0, 
			{ { {ValueType::F32Vec4, 1, 0}, {ValueType::F32Vec4, 1, 16}}, sizeof ( glm::vec4 ) * 2, 1 } 
		}
	);
	instance->instancebase_registered ( billboard_instance_base_id );

}
