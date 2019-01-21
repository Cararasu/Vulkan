#pragma once

#include <render/Header.h>
#include <render/Model.h>
#include "AModel.h"
#include "Camera.h"
#include <render/Instance.h>
#include "Camera.h"

struct AnInstance {
	glm::mat4 mv2_matrix;
	glm::mat4 normal_matrix;
};
struct ShotInstance {
	glm::mat4 mv2_matrix;
	glm::vec4 umbracolor_range;
};
struct BillboardInstance {
	glm::vec4 position;
	glm::vec4 scale_index_time;
};
struct CameraStruct {
	glm::mat4 v2s_mat;
	glm::mat4 inv_v2s_mat;
	glm::mat4 w2v_mat;
	glm::mat4 inv_w2v_mat;
	glm::vec4 camera_pos;

};
struct Light {
	glm::vec4 direction_amb;
	glm::vec4 color;
};
struct ShadowMap {
	glm::mat4 v2ls_mat[3];
	glm::vec4 drawrange[3];
};
struct World;

struct WorldShard {
	Camera camera;
	Context camera_matrix;
	CameraStruct cameradata;

	Context light_vector_context;
	Light global_light;

	Context skybox_context;
	glm::mat4 skybox_matrix;

	Context shadowmap_context;
	ShadowMap shadowmap_data;

	DynArray<AnInstance> x_instances;
	DynArray<AnInstance> tie_instances;
	DynArray<AnInstance> gallofree_instances;
	DynArray<ShotInstance> shot_instances;
	DynArray<BillboardInstance> billboard_instances;

	InstanceGroup* igroup;
	ContextGroup* cgroup;

	void update_shard ( World* world );
};
struct ShadowShard {
	Camera camera;
	float camera_near, camera_far;
	Context camera_matrix;
	CameraStruct cameradata;

	ContextGroup* cgroup;

	void update_shard ( World* world );
};

struct World {
	Array<Model> xwing_models;
	Array<Model> tie_models;
	Array<Model> gallofree_models;
	Model cube_model;
	Model square_model;
	Model fullscreen_model;

	glm::vec3 light_vector;

	DynArray<AModel> xwings;
	DynArray<AModel> ties;
	DynArray<AModel> gallofrees;
	DynArray<AModel> red_shots;
	DynArray<AModel> green_shots;
	DynArray<BModel> billboards;

	Instance* instance;
	WorldShard world_shard;
	ShadowShard shadow_shard[3];

	void init ( Instance* instance );

	void update_shards ( float delta );
};
