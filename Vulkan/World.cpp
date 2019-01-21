#include "World.h"
#include <render/Specialization.h>
#include <render/Logger.h>

extern Logger g_logger;

struct SimpleVertex {
	glm::vec3 pos;
	glm::vec3 uv;
	glm::vec3 normal;
};

#include <fstream>
#include <sstream>

u32 loadDataFile ( std::string file, std::vector<SimpleVertex>& vertices, std::vector<u32>& indices, std::tuple<u32, u32>& vertex_data, std::tuple<u32, u32>& index_data ) {

	g_logger.log<LogLevel::eDebug> ( "Loading File %s", file.c_str() );
	std::ifstream input ( file, std::ios::binary );

	if ( !input.is_open() ) {
		g_logger.log<LogLevel::eError> ( "Couldn't open File %s", file.c_str() );
		return false;
	}

	std::string str;
	std::getline ( input, str, '\0' );

	input.read ( reinterpret_cast<char*> ( &std::get<0> ( vertex_data ) ), sizeof ( u32 ) );
	std::get<1> ( vertex_data ) = vertices.size();
	vertices.resize ( vertices.size() + std::get<0> ( vertex_data ) );
	input.read ( reinterpret_cast<char*> ( vertices.data() + std::get<1> ( vertex_data ) ), sizeof ( SimpleVertex ) * std::get<0> ( vertex_data ) );

	input.read ( reinterpret_cast<char*> ( &std::get<0> ( index_data ) ), sizeof ( u32 ) );
	std::get<1> ( index_data ) = indices.size();
	indices.resize ( indices.size() + std::get<0> ( index_data ) );
	input.read ( reinterpret_cast<char*> ( indices.data() + std::get<1> ( index_data ) ), sizeof ( u32 ) * std::get<0> ( index_data ) );

	input.close();

	if ( g_logger.level <= LogLevel::eDebug ) {
		g_logger.log<LogLevel::eDebug> ( "Model %s", str.c_str() );
		g_logger.log<LogLevel::eDebug> ( "\tVertices %u", std::get<0> ( vertex_data ) );
		g_logger.log<LogLevel::eDebug> ( "\tInstances %u", std::get<0> ( index_data ) );
		g_logger.log<LogLevel::eDebug> ( "\t%u Bytes Read", ( sizeof ( SimpleVertex ) * std::get<0> ( vertex_data ) + sizeof ( u32 ) * std::get<0> ( index_data ) + 8 + str.size() ) );
	}
	return true;
}


void World::init ( Instance* instance ) {
	this->instance = instance;
	fullscreen_model = instance->create_model ( fullscreen_modelbase_id );
	{
		Array<glm::vec3> data_to_load = {
			glm::vec3 ( 0.0f, 0.0f, 0.0f ),
			glm::vec3 ( 2.0f, 0.0f, 0.0f ),
			glm::vec3 ( 0.0f, 2.0f, 0.0f )
		};
		Array<u16> indices = { 0, 1, 2 };
		instance->load_generic_model ( fullscreen_model, data_to_load.data, data_to_load.size, indices.data, indices.size );
	}

	cube_model = instance->create_model ( simple_modelbase_id );
	{
		Array<SimpleVertex> data_to_load = {
			{glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3 ( 0.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },
			{glm::vec3 ( -1.0f, 1.0f, 1.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },
			{glm::vec3 ( 1.0f, -1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },
			{glm::vec3 ( -1.0f, -1.0f, 1.0f ), glm::vec3 ( 1.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },

			{glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ) },
			{glm::vec3 ( 1.0f, 1.0f, -1.0f ), glm::vec3 ( 1.0f, 0.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ) },
			{glm::vec3 ( -1.0f, 1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ) },
			{glm::vec3 ( -1.0f, 1.0f, -1.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ) },

			{glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3 ( 0.0f, 0.0f, 2.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ) },
			{glm::vec3 ( 1.0f, -1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 2.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ) },
			{glm::vec3 ( 1.0f, 1.0f, -1.0f ), glm::vec3 ( 1.0f, 0.0f, 2.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ) },
			{glm::vec3 ( 1.0f, -1.0f, -1.0f ), glm::vec3 ( 1.0f, 1.0f, 2.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ) },

			{glm::vec3 ( 1.0f, 1.0f, -1.0f ), glm::vec3 ( 0.0f, 0.0f, 3.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ) },
			{glm::vec3 ( 1.0f, -1.0f, -1.0f ), glm::vec3 ( 0.0f, 1.0f, 3.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ) },
			{glm::vec3 ( -1.0f, 1.0f, -1.0f ), glm::vec3 ( 1.0f, 0.0f, 3.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ) },
			{glm::vec3 ( -1.0f, -1.0f, -1.0f ), glm::vec3 ( 1.0f, 1.0f, 3.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ) },

			{glm::vec3 ( 1.0f, -1.0f, 1.0f ), glm::vec3 ( 0.0f, 0.0f, 4.0f ), glm::vec3 ( 0.0f, -1.0f, 0.0f ) },
			{glm::vec3 ( -1.0f, -1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 4.0f ), glm::vec3 ( 0.0f, -1.0f, 0.0f ) },
			{glm::vec3 ( 1.0f, -1.0f, -1.0f ), glm::vec3 ( 1.0f, 0.0f, 4.0f ), glm::vec3 ( 0.0f, -1.0f, 0.0f ) },
			{glm::vec3 ( -1.0f, -1.0f, -1.0f ), glm::vec3 ( 1.0f, 1.0f, 4.0f ), glm::vec3 ( 0.0f, -1.0f, 0.0f ) },

			{glm::vec3 ( -1.0f, 1.0f, 1.0f ), glm::vec3 ( 0.0f, 0.0f, 5.0f ), glm::vec3 ( -1.0f, 0.0f, 0.0f ) },
			{glm::vec3 ( -1.0f, 1.0f, -1.0f ), glm::vec3 ( 1.0f, 0.0f, 5.0f ), glm::vec3 ( -1.0f, 0.0f, 0.0f ) },
			{glm::vec3 ( -1.0f, -1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 5.0f ), glm::vec3 ( -1.0f, 0.0f, 0.0f ) },
			{glm::vec3 ( -1.0f, -1.0f, -1.0f ), glm::vec3 ( 1.0f, 1.0f, 5.0f ), glm::vec3 ( -1.0f, 0.0f, 0.0f ) },
		};
		Array<u16> indices = {
			0, 1, 2, 2, 1, 3,
			4, 5, 6, 6, 5, 7,
			8, 9, 10, 10, 9, 11,
			12, 13, 14, 14, 13, 15,
			16, 17, 18, 18, 17, 19,
			20, 21, 22, 22, 21, 23,
		};
		instance->load_generic_model ( cube_model, data_to_load.data, data_to_load.size, indices.data, indices.size );
	}

	square_model = instance->create_model ( simple_modelbase_id );
	{
		Array<SimpleVertex> data_to_load = {
			{glm::vec3 ( 1.0f, 1.0f, 0.0f ), glm::vec3 ( 1.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },
			{glm::vec3 ( -1.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },
			{glm::vec3 ( 1.0f, -1.0f, 0.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },
			{glm::vec3 ( -1.0f, -1.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) }
		};
		Array<u16> indices = {
			0, 1, 2, 2, 1, 3
		};
		instance->load_generic_model ( square_model, data_to_load.data, data_to_load.size, indices.data, indices.size );
	}
	ResourceManager* resource_manager = instance->resource_manager();
	Image* skybox_teximage = resource_manager->create_texture ( 2048, 2048, 0, 6, 1, ImageFormat::e4Unorm8 );
	resource_manager->load_image_to_texture ( "assets/SkyboxGalaxy/GalaxyTex_PositiveZ_2k.png", skybox_teximage, 0, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxGalaxy/GalaxyTex_PositiveY_2k.png", skybox_teximage, 1, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxGalaxy/GalaxyTex_PositiveX_2k.png", skybox_teximage, 2, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxGalaxy/GalaxyTex_NegativeZ_2k.png", skybox_teximage, 3, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxGalaxy/GalaxyTex_NegativeY_2k.png", skybox_teximage, 4, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxGalaxy/GalaxyTex_NegativeX_2k.png", skybox_teximage, 5, 0 );

	Image* explosion_teximage = resource_manager->create_texture ( 256, 256, 0, 32, 1, ImageFormat::e4Unorm8 );
	for ( int i = 0; i < 32; i++ ) {
		char buffer[100];
		snprintf ( buffer, 100, "assets/Explosion/explode_%d.bmp", i + 1 );
		resource_manager->load_image_to_texture ( buffer, explosion_teximage, i, 0 );
	}

	std::vector<SimpleVertex> vvv;
	std::vector<u32> iiii;
	std::tuple<u32, u32> vertex_data;
	std::tuple<u32, u32> index_data;

	xwing_models.resize ( 6 );

	xwing_models[0] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Body.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( xwing_models[0], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	xwing_models[1] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Windows.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( xwing_models[1], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	xwing_models[2] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Wing_LB.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( xwing_models[2], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	xwing_models[3] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Wing_LT.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( xwing_models[3], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	xwing_models[4] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Wing_RB.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( xwing_models[4], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	xwing_models[5] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Wing_RT.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( xwing_models[5], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	tie_models.resize ( 6 );

	tie_models[0] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Tie/Tie_Fighter_Body.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( tie_models[0], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	tie_models[1] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Tie/Tie_Fighter_Windows.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( tie_models[1], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	tie_models[2] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Tie/Tie_Fighter_Arm_L.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( tie_models[2], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	tie_models[3] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Tie/Tie_Fighter_Arm_R.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( tie_models[3], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	tie_models[4] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Tie/Tie_Fighter_Wing_L.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( tie_models[4], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	tie_models[5] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Tie/Tie_Fighter_Wing_R.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( tie_models[5], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	gallofree_models.resize ( 11 );

	gallofree_models[0] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/BottomEngine2.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( gallofree_models[0], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[1] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/BottomEngines1.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( gallofree_models[1], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[2] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/Cargo.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( gallofree_models[2], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[3] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/CockPit_Cockpit.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( gallofree_models[3], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[4] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/Greebles.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( gallofree_models[4], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[5] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/HullPlatesBottom.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( gallofree_models[5], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[6] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/HullPlatesTop.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( gallofree_models[6], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[7] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/SectionBottom.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( gallofree_models[7], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[8] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/SectionTop.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( gallofree_models[8], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[9] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/SectionTopInterior.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( gallofree_models[9], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[10] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/TopEngines.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( gallofree_models[10], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();


	world_shard.igroup = instance->create_instancegroup();//maybe list of modelinstancebases for optimization
	world_shard.cgroup = instance->create_contextgroup();//maybe list of contextbases for optimization
	
	shadow_shard[0].cgroup = instance->create_contextgroup();
	shadow_shard[1].cgroup = instance->create_contextgroup();
	shadow_shard[2].cgroup = instance->create_contextgroup();

	Sampler* sampler = resource_manager->create_sampler (
	                       FilterType::eLinear, FilterType::eLinear, FilterType::eLinear,
	                       EdgeHandling::eRepeat, EdgeHandling::eRepeat, EdgeHandling::eRepeat,
	                       0.0f, {0.0f, 1.0f}, 0.0f, DepthComparison::eNone );

	world_shard.light_vector_context = instance->create_context ( lightvector_base_id );
	world_shard.cgroup->set_context ( world_shard.light_vector_context );

	world_shard.camera_matrix = instance->create_context ( camera_context_base_id );
	world_shard.cgroup->set_context ( world_shard.camera_matrix );

	shadow_shard[0].camera_matrix = instance->create_context ( camera_context_base_id );
	shadow_shard[0].cgroup->set_context ( shadow_shard[0].camera_matrix );
	shadow_shard[1].camera_matrix = instance->create_context ( camera_context_base_id );
	shadow_shard[1].cgroup->set_context ( shadow_shard[1].camera_matrix );
	shadow_shard[2].camera_matrix = instance->create_context ( camera_context_base_id );
	shadow_shard[2].cgroup->set_context ( shadow_shard[2].camera_matrix );


	Context x_context = instance->create_context ( tex_simplemodel_context_base_id );
	Image* x_teximage = resource_manager->load_image_to_texture ( "assets/X/XWing_Diffuse_01_2k.png", 4 );
	instance->update_context_image ( x_context, 0, x_teximage->create_use ( ImagePart::eColor, {0, 1}, {0, 1} ) );
	instance->update_context_sampler ( x_context, 0, sampler );
	for ( Model& model : xwing_models ) instance->set_context ( model, x_context );

	world_shard.skybox_context = instance->create_context ( skybox_context_base_id );
	instance->update_context_image ( world_shard.skybox_context, 0, skybox_teximage->create_use ( ImagePart::eColor, {0, 1}, {0, 6} ) );
	instance->update_context_sampler ( world_shard.skybox_context, 0, sampler );
	instance->set_context ( cube_model, world_shard.skybox_context );

	Context explosion_context = instance->create_context ( explosion_context_base_id );
	instance->update_context_image ( explosion_context, 0, explosion_teximage->create_use ( ImagePart::eColor, {0, 1}, {0, 32} ) );
	instance->update_context_sampler ( explosion_context, 0, sampler );
	instance->set_context ( square_model, explosion_context );


	Context tie_body_context = instance->create_context ( tex_simplemodel_context_base_id );
	Context tie_arm_context = instance->create_context ( tex_simplemodel_context_base_id );
	Context tie_wing_context = instance->create_context ( tex_simplemodel_context_base_id );
	Image* tie_body_teximage = resource_manager->load_image_to_texture ( "assets/Tie/Tie_Fighter_Body_Diffuse_2k.png", 4 );
	Image* tie_arm_teximage = resource_manager->load_image_to_texture ( "assets/Tie/Tie_Fighter_Arm_Diffuse_2k.png", 4 );
	Image* tie_wing_teximage = resource_manager->load_image_to_texture ( "assets/Tie/Tie_Fighter_Wing_Diffuse_2k.png", 4 );
	instance->update_context_image ( tie_body_context, 0, tie_body_teximage->create_use ( ImagePart::eColor, {0, 1}, {0, 1} ) );
	instance->update_context_sampler ( tie_body_context, 0, sampler );
	instance->update_context_image ( tie_arm_context, 0, tie_arm_teximage->create_use ( ImagePart::eColor, {0, 1}, {0, 1} ) );
	instance->update_context_sampler ( tie_arm_context, 0, sampler );
	instance->update_context_image ( tie_wing_context, 0, tie_wing_teximage->create_use ( ImagePart::eColor, {0, 1}, {0, 1} ) );
	instance->update_context_sampler ( tie_wing_context, 0, sampler );
	instance->set_context ( tie_models[0], tie_body_context );
	instance->set_context ( tie_models[1], tie_body_context );
	instance->set_context ( tie_models[2], tie_arm_context );
	instance->set_context ( tie_models[3], tie_arm_context );
	instance->set_context ( tie_models[4], tie_wing_context );
	instance->set_context ( tie_models[5], tie_wing_context );

	Context gallofree_context = instance->create_context ( tex_simplemodel_context_base_id );
	Context flat_gallofree_context = instance->create_context ( flat_simplemodel_context_base_id );
	Image* gallofree_teximage = resource_manager->load_image_to_texture ( "assets/Gallofree/ScratchedMetal2.jpeg", 4 );

	instance->update_context_image ( gallofree_context, 0, gallofree_teximage->create_use ( ImagePart::eColor, {0, 1}, {0, 1} ) );
	instance->update_context_sampler ( gallofree_context, 0, sampler );

	glm::vec4 color ( 0.75f, 0.75f, 0.75f, 1.0f );
	instance->update_context_data ( flat_gallofree_context, &color );
	for ( Model& model : gallofree_models ) instance->set_context ( model, gallofree_context );
	for ( Model& model : gallofree_models ) instance->set_context ( model, flat_gallofree_context );

	world_shard.igroup->clear();

}
void World::update_shards ( float delta )  {
	for ( AModel& xwing : xwings ) {
		xwing.move ( delta );
	}
	for ( AModel& tie : ties ) {
		tie.move ( delta );
	}
	for ( AModel& gallofree : gallofrees ) {
		gallofree.move ( delta );
	}
	for ( AModel& shot : red_shots ) {
		shot.move ( delta );
	}
	for ( AModel& shot : green_shots ) {
		shot.move ( delta );
	}
	for ( auto it = billboards.begin(); it != billboards.end(); ) {
		BModel& billboard = *it;
		billboard.move ( delta );
		if ( billboard.ttl <= 0.0 ) {
			it = billboards.erase ( it );
			continue;
		}
		it++;
	}
	shadow_shard[0].update_shard ( this );
	shadow_shard[1].update_shard ( this );
	shadow_shard[2].update_shard ( this );
	
	world_shard.update_shard ( this );


}
void WorldShard::update_shard ( World* world ) {

	igroup->clear();

	glm::mat4 w2v_rot_matrix = camera.orientation.w2v_rot_mat();
	cameradata.v2s_mat = camera.v2s_mat();
	cameradata.inv_v2s_mat = glm::inverse ( cameradata.v2s_mat );
	cameradata.w2v_mat = camera.orientation.w2v_mat();
	cameradata.inv_w2v_mat = glm::inverse ( cameradata.w2v_mat );
	cameradata.camera_pos = cameradata.w2v_mat * glm::vec4 ( camera.orientation.look_at - camera.orientation.view_vector, 1.0f );
	cameradata.camera_pos /= cameradata.camera_pos.w;

	world->instance->update_context_data ( camera_matrix, &cameradata );

	for(int i = 0; i < 3; i++) {
		shadowmap_data.v2ls_mat[i] = world->shadow_shard[i].camera.v2s_mat() * world->shadow_shard[i].camera.orientation.w2v_mat();
		shadowmap_data.drawrange[i] = glm::vec4 ( world->shadow_shard[i].camera_near, world->shadow_shard[i].camera_far, 0.0f, 0.0f );
	}
	world->instance->update_context_data ( shadowmap_context, &shadowmap_data );

	x_instances.resize ( world->xwings.size() );

	for ( u32 i = 0; i < world->xwings.size(); i++ ) {
		x_instances[i].mv2_matrix = cameradata.w2v_mat * world->xwings[i].m2w_mat();
		x_instances[i].normal_matrix = glm::transpose ( glm::inverse ( x_instances[i].mv2_matrix ) );
	}
	igroup->register_instances ( textured_instance_base_id, world->xwing_models, x_instances.data(), x_instances.size() );

	tie_instances.resize ( world->ties.size() );
	for ( u32 i = 0; i < world->ties.size(); i++ ) {
		tie_instances[i].mv2_matrix = cameradata.w2v_mat * world->ties[i].m2w_mat();
		tie_instances[i].normal_matrix = glm::transpose ( glm::inverse ( tie_instances[i].mv2_matrix ) );
	}
	igroup->register_instances ( textured_instance_base_id, world->tie_models, tie_instances.data(), tie_instances.size() );

	//AnInstance gallofree_matrixes1[1] = {
	//	{ glm::scale ( glm::translate ( cameradata.w2v_mat, glm::vec3 ( -4.0f, 0.0f, 0.0f ) ), glm::vec3 ( 10.0f, 10.0f, 10.0f ) ), glm::mat4() }
	//};
	//gallofree_matrixes1[0].normal_matrix = glm::transpose ( glm::inverse ( gallofree_matrixes1[0].mv2_matrix ) );
	//instancegroup->register_instances ( textured_instance_base_id, gallofree_models, gallofree_matrixes1, 1 );

	gallofree_instances.resize ( world->gallofrees.size() );
	for ( u32 i = 0; i < world->gallofrees.size(); i++ ) {
		gallofree_instances[i].mv2_matrix = cameradata.w2v_mat * world->gallofrees[i].m2w_mat();
		gallofree_instances[i].normal_matrix = glm::transpose ( glm::inverse ( gallofree_instances[i].mv2_matrix ) );
	}
	igroup->register_instances ( flat_instance_base_id, world->gallofree_models, gallofree_instances.data(), gallofree_instances.size() );

	shot_instances.resize ( world->red_shots.size() + world->green_shots.size() );
	{
		u32 i = 0;
		for ( ; i < world->red_shots.size(); i++ ) {
			shot_instances[i].mv2_matrix = cameradata.w2v_mat * world->red_shots[i].m2w_mat();
			shot_instances[i].umbracolor_range = glm::vec4 ( 1.0, 0.1, 0.1, 30.0 );
		}
		u32 j = 0;
		for ( ; j < world->green_shots.size(); j++ ) {
			shot_instances[i + j].mv2_matrix = cameradata.w2v_mat * world->green_shots[j].m2w_mat();
			shot_instances[i + j].umbracolor_range = glm::vec4 ( 0.1, 1.0, 0.1, 30.0 );
		}
	}
	igroup->register_instances ( shot_instance_base_id, world->cube_model, shot_instances.data(), shot_instances.size() );

	billboard_instances.resize ( world->billboards.size() );
	for ( u32 i = 0; i < world->billboards.size(); i++ ) {
		BModel& bm = world->billboards[i];
		billboard_instances[i].position = cameradata.w2v_mat * glm::vec4 ( bm.position, 1.0 );
		billboard_instances[i].scale_index_time = glm::vec4 ( bm.scale.x, bm.scale.y, ( float ) bm.index, 1.0f - ( bm.ttl / bm.max_ttl ) );
	}
	
	igroup->register_instances ( billboard_instance_base_id, world->square_model, billboard_instances.data(), billboard_instances.size() );

	igroup->register_instances ( single_instance_base_id, world->fullscreen_model, nullptr, 1 );

	global_light.direction_amb = glm::normalize ( w2v_rot_matrix * glm::vec4 ( world->light_vector, 0.0f ) );
	//global_light.direction_amb /= global_light.direction_amb.w;
	global_light.direction_amb.w = 0.4f;
	global_light.color = glm::vec4 ( 1.5f, 1.4f, 1.1f, 1.0f );

	world->instance->update_context_data ( light_vector_context, &global_light );
	igroup->register_instances ( dirlight_instance_base_id, world->fullscreen_model, nullptr, 1 );

	skybox_matrix = cameradata.v2s_mat * w2v_rot_matrix;
	world->instance->update_context_data ( skybox_context, &skybox_matrix );
	igroup->register_instances ( skybox_instance_base_id, world->cube_model, nullptr, 1 );
}
void ShadowShard::update_shard ( World* world ) {

	cameradata.v2s_mat = camera.v2s_mat();
	cameradata.inv_v2s_mat = glm::inverse ( cameradata.v2s_mat );
	cameradata.w2v_mat = camera.orientation.w2v_mat();
	cameradata.inv_w2v_mat = glm::inverse ( cameradata.w2v_mat );
	cameradata.camera_pos = cameradata.w2v_mat * glm::vec4 ( camera.orientation.look_at - camera.orientation.view_vector, 1.0f );
	cameradata.camera_pos /= cameradata.camera_pos.w;

	world->instance->update_context_data ( camera_matrix, &cameradata );
}
