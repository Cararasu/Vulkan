#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Initialization.h"

#include <chrono>
#include <thread>
#include <algorithm>
#include <vector>
#include <fstream>
#include <sstream>

#include <render/Window.h>
#include <render/Logger.h>
#include <render/Instance.h>
#include <render/Specialization.h>
#include <render/Timing.h>
#include <render/UTF.h>

#include "Camera.h"
#include "GameState.h"

Logger g_logger ( "main" );

struct SimpleVertex {
	glm::vec3 pos;
	glm::vec3 uv;
	glm::vec3 normal;
};

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
//from http://lukas-polok.cz/tutorial_sphere.htm
float f_rand() {
	return float ( rand() ) / RAND_MAX * 2 - 1;
	// generates random number in range [-1, 1] with uniform distribution
}
float f_rand2() {
	float f = f_rand();
	return f / cos ( f );
}
using namespace std::chrono_literals;

GameState g_state;

#include <render/Queues.h>

int main ( int argc, char **argv ) {

	Instance* instance = create_instance ( "Vulkan" );
	instance->initialize ( InstanceOptions() );

	register_shaders ( instance );

	Monitor* primMonitor = instance->get_primary_monitor();

	if ( g_logger.level <= LogLevel::eDebug ) {
		g_logger.log<LogLevel::eDebug> ( "Monitors" );
		for ( Monitor* monitor : instance->monitors ) {
			if ( monitor == primMonitor )
				g_logger.log<LogLevel::eDebug> ( "\tPrimary Monitor | %s %dx%d", monitor->name, monitor->extend.x, monitor->extend.y );
			else
				g_logger.log<LogLevel::eDebug> ( "\t%s %dx%d", monitor->name, monitor->extend.x, monitor->extend.y );
		}
		g_logger.log<LogLevel::eDebug> ( "Devices" );
		for ( Device* device : instance->devices ) {
			g_logger.log<LogLevel::eDebug> ( "\t%s %" PRId32, device->name, device->rating );
		}
	}

	Model dot_model = instance->create_model ( dot_modelbase_id );
	{
		Array<glm::vec3> data_to_load = {
			glm::vec3 ( 0.0f, 0.0f, 0.0f )
		};
		Array<u16> indices = { 0 };
		instance->load_generic_model ( dot_model, data_to_load.data, data_to_load.size, indices.data, indices.size );
	}
	Model fullscreen_model = instance->create_model ( fullscreen_modelbase_id );
	{
		Array<glm::vec3> data_to_load = {
			glm::vec3 ( 0.0f, 0.0f, 0.0f ),
			glm::vec3 ( 2.0f, 0.0f, 0.0f ),
			glm::vec3 ( 0.0f, 2.0f, 0.0f )
		};
		Array<u16> indices = { 0, 1, 2 };
		instance->load_generic_model ( fullscreen_model, data_to_load.data, data_to_load.size, indices.data, indices.size );
	}

	Model cube = instance->create_model ( simple_modelbase_id );
	{
		Array<SimpleVertex> data_to_load = {
			{glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3 ( 1.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },
			{glm::vec3 ( -1.0f, 1.0f, 1.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },
			{glm::vec3 ( 1.0f, -1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },
			{glm::vec3 ( -1.0f, -1.0f, 1.0f ), glm::vec3 ( 0.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },

			{glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ) },
			{glm::vec3 ( 1.0f, 1.0f, -1.0f ), glm::vec3 ( 1.0f, 0.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ) },
			{glm::vec3 ( -1.0f, 1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ) },
			{glm::vec3 ( -1.0f, 1.0f, -1.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ) },

			{glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3 ( 1.0f, 1.0f, 2.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ) },
			{glm::vec3 ( 1.0f, -1.0f, 1.0f ), glm::vec3 ( 1.0f, 0.0f, 2.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ) },
			{glm::vec3 ( 1.0f, 1.0f, -1.0f ), glm::vec3 ( 0.0f, 1.0f, 2.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ) },
			{glm::vec3 ( 1.0f, -1.0f, -1.0f ), glm::vec3 ( 0.0f, 0.0f, 2.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ) },

			{glm::vec3 ( 1.0f, 1.0f, -1.0f ), glm::vec3 ( 0.0f, 0.0f, 3.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ) },
			{glm::vec3 ( 1.0f, -1.0f, -1.0f ), glm::vec3 ( 0.0f, 1.0f, 3.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ) },
			{glm::vec3 ( -1.0f, 1.0f, -1.0f ), glm::vec3 ( 1.0f, 0.0f, 3.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ) },
			{glm::vec3 ( -1.0f, -1.0f, -1.0f ), glm::vec3 ( 1.0f, 1.0f, 3.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ) },

			{glm::vec3 ( 1.0f, -1.0f, 1.0f ), glm::vec3 ( 0.0f, 0.0f, 4.0f ), glm::vec3 ( 0.0f, -1.0f, 0.0f ) },
			{glm::vec3 ( -1.0f, -1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 4.0f ), glm::vec3 ( 0.0f, -1.0f, 0.0f ) },
			{glm::vec3 ( 1.0f, -1.0f, -1.0f ), glm::vec3 ( 1.0f, 0.0f, 4.0f ), glm::vec3 ( 0.0f, -1.0f, 0.0f ) },
			{glm::vec3 ( -1.0f, -1.0f, -1.0f ), glm::vec3 ( 1.0f, 1.0f, 4.0f ), glm::vec3 ( 0.0f, -1.0f, 0.0f ) },

			{glm::vec3 ( -1.0f, 1.0f, 1.0f ), glm::vec3 ( 0.0f, 0.0f, 5.0f ), glm::vec3 ( -1.0f, 0.0f, 0.0f ) },
			{glm::vec3 ( -1.0f, 1.0f, -1.0f ), glm::vec3 ( 0.0f, 1.0f, 5.0f ), glm::vec3 ( -1.0f, 0.0f, 0.0f ) },
			{glm::vec3 ( -1.0f, -1.0f, 1.0f ), glm::vec3 ( 1.0f, 0.0f, 5.0f ), glm::vec3 ( -1.0f, 0.0f, 0.0f ) },
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
		instance->load_generic_model ( cube, data_to_load.data, data_to_load.size, indices.data, indices.size );
	}

	Model square = instance->create_model ( simple_modelbase_id );
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
		instance->load_generic_model ( square, data_to_load.data, data_to_load.size, indices.data, indices.size );
	}
	ResourceManager* resource_manager = instance->resource_manager();
	Image* skybox_teximage = resource_manager->create_texture ( 4096, 4096, 0, 6, 1, ImageFormat::e4Unorm8 );
	resource_manager->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_PositiveZ.png", skybox_teximage, 0, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_PositiveY.png", skybox_teximage, 1, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_PositiveX.png", skybox_teximage, 2, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_NegativeZ.png", skybox_teximage, 3, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_NegativeY.png", skybox_teximage, 4, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_NegativeX.png", skybox_teximage, 5, 0 );

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

	Array<Model> x_models ( 6 );

	x_models[0] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Body.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( x_models[0], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	x_models[1] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Windows.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( x_models[1], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	x_models[2] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Wing_LB.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( x_models[2], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	x_models[3] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Wing_LT.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( x_models[3], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	x_models[4] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Wing_RB.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( x_models[4], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	x_models[5] = instance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Wing_RT.data", vvv, iiii, vertex_data, index_data );
	instance->load_generic_model ( x_models[5], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	Array<Model> tie_models ( 6 );

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

	Array<Model> gallofree_models ( 11 );

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


	InstanceGroup* instancegroup = instance->create_instancegroup();//maybe list of modelinstancebases for optimization
	ContextGroup* contextgroup = instance->create_contextgroup();//maybe list of contextbases for optimization


	Sampler* sampler = resource_manager->create_sampler (
	                       FilterType::eLinear, FilterType::eLinear, FilterType::eLinear,
	                       EdgeHandling::eRepeat, EdgeHandling::eRepeat, EdgeHandling::eRepeat,
	                       0.0f, {0.0f, 1.0f}, 0.0f, DepthComparison::eNone );

	Context light_vector_context = instance->create_context ( lightvector_base_id );
	contextgroup->set_context ( light_vector_context );

	Context camera_matrix = instance->create_context ( camera_context_base_id );
	contextgroup->set_context ( camera_matrix );


	Context x_context = instance->create_context ( tex_simplemodel_context_base_id );
	Image* x_teximage = resource_manager->load_image_to_texture ( "assets/X/XWing_Diffuse_01_1k.png", 4 );
	instance->update_context_image ( x_context, 0, x_teximage->create_use ( ImagePart::eColor, {0, 1}, {0, 1} ) );
	instance->update_context_sampler ( x_context, 0, sampler );
	instance->set_context ( x_models[0], x_context );
	instance->set_context ( x_models[1], x_context );
	instance->set_context ( x_models[2], x_context );
	instance->set_context ( x_models[3], x_context );
	instance->set_context ( x_models[4], x_context );
	instance->set_context ( x_models[5], x_context );

	Context skybox_context = instance->create_context ( skybox_context_base_id );
	instance->update_context_image ( skybox_context, 0, skybox_teximage->create_use ( ImagePart::eColor, {0, 1}, {0, 1} ) );
	instance->update_context_sampler ( skybox_context, 0, sampler );
	instance->set_context ( cube, skybox_context );

	Context explosion_context = instance->create_context ( explosion_context_base_id );
	instance->update_context_image ( explosion_context, 0, explosion_teximage->create_use ( ImagePart::eColor, {0, 1}, {0, 32} ) );
	instance->update_context_sampler ( explosion_context, 0, sampler );
	instance->set_context ( square, explosion_context );

	Context tie_body_context = instance->create_context ( tex_simplemodel_context_base_id );
	Context tie_arm_context = instance->create_context ( tex_simplemodel_context_base_id );
	Context tie_wing_context = instance->create_context ( tex_simplemodel_context_base_id );
	Image* tie_body_teximage = resource_manager->load_image_to_texture ( "assets/Tie/Tie_Fighter_Body_Diffuse_1k.png", 4 );
	Image* tie_arm_teximage = resource_manager->load_image_to_texture ( "assets/Tie/Tie_Fighter_Arm_Diffuse_1k.png", 4 );
	Image* tie_wing_teximage = resource_manager->load_image_to_texture ( "assets/Tie/Tie_Fighter_Wing_Diffuse_1k.png", 4 );
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

	instance->set_context ( gallofree_models[0], gallofree_context );
	instance->set_context ( gallofree_models[1], gallofree_context );
	instance->set_context ( gallofree_models[2], gallofree_context );
	instance->set_context ( gallofree_models[3], gallofree_context );
	instance->set_context ( gallofree_models[4], gallofree_context );
	instance->set_context ( gallofree_models[5], gallofree_context );
	instance->set_context ( gallofree_models[6], gallofree_context );
	instance->set_context ( gallofree_models[7], gallofree_context );
	instance->set_context ( gallofree_models[8], gallofree_context );
	instance->set_context ( gallofree_models[9], gallofree_context );
	instance->set_context ( gallofree_models[10], gallofree_context );

	instance->set_context ( gallofree_models[0], flat_gallofree_context );
	instance->set_context ( gallofree_models[1], flat_gallofree_context );
	instance->set_context ( gallofree_models[2], flat_gallofree_context );
	instance->set_context ( gallofree_models[3], flat_gallofree_context );
	instance->set_context ( gallofree_models[4], flat_gallofree_context );
	instance->set_context ( gallofree_models[5], flat_gallofree_context );
	instance->set_context ( gallofree_models[6], flat_gallofree_context );
	instance->set_context ( gallofree_models[7], flat_gallofree_context );
	instance->set_context ( gallofree_models[8], flat_gallofree_context );
	instance->set_context ( gallofree_models[9], flat_gallofree_context );
	instance->set_context ( gallofree_models[10], flat_gallofree_context );

	instancegroup->clear();

	Window* window = instance->create_window();

	Extent2D<s32> window_size ( 1000, 600 );
	window->position() = primMonitor->offset + ( ( primMonitor->extend / 2 ) - ( window_size / 2 ) );
	window->size() = window_size;
	window->visible() = true;

	window->update();
	//*window->position() = {100, 100};
	//*window->size() = {800, 800};
	window->cursor_mode() = CursorMode::eInvisible;

	RenderBundle* bundle = setup_renderbundle ( instance, window, instancegroup, contextgroup );

	struct Light {
		glm::vec4 direction_amb;
		glm::vec4 color;
	} global_light;
	//setup objects

	glm::vec4 light_vector ( -1.0, -1.0, -1.0, 0.0f );

	DynArray<AModel> xwings ( 2 );
	xwings[0].init ( glm::vec3 ( 5.0f, -5.0f, -10.0f ), glm::angleAxis ( glm::pi<float>() * 0.5f, glm::normalize ( glm::vec3 ( 0.3f, -1.0f, 0.0f ) ) ), 6.25f, 10.0f );
	xwings[1].init ( glm::vec3 ( 20.0f, -0.0f, -5.0f ), glm::angleAxis ( glm::pi<float>() * 0.5f, glm::normalize ( glm::vec3 ( 0.25f, -1.0f, 0.0f ) ) ), 6.25f, 10.0f );

	DynArray<AModel> ties ( 2 );
	ties[0].init ( glm::vec3 ( -75.0f, -30.0f, -7.5f ), glm::angleAxis ( glm::pi<float>() * 0.5f, glm::normalize ( glm::vec3 ( 0.27f, -1.0f, 0.0f ) ) ), 4.5f, 10.0f );
	ties[1].init ( glm::vec3 ( 0.0f, 100.0f, 0.0f ), glm::angleAxis ( glm::pi<float>() * 0.7f, glm::normalize ( glm::vec3 ( 1.0f, -1.0f, 1.0f ) ) ), 4.5f, 10.0f );

	DynArray<AModel> gallofrees ( 1 );
	gallofrees[0].init ( glm::vec3 ( 30.0f, -20.0f, -30.0f ), glm::angleAxis ( glm::pi<float>() * 0.5f, glm::vec3 ( 0.0f, -1.0f, 0.0f ) ), 45.0f, 2.0f );

	DynArray<AModel> red_shots ( 2 );
	{
		glm::vec3 shot1_translation ( 4.6f, 0.25f, 7.0f );
		glm::vec3 shot2_translation ( -4.6f, -0.4f, 7.0f );
		shot1_translation = glm::rotate ( xwings[0].rotation, shot1_translation );
		shot2_translation = glm::rotate ( xwings[0].rotation, shot2_translation );
		red_shots[0].init ( xwings[0].position + shot1_translation, xwings[0].rotation, glm::vec3 ( 0.3f, 0.3f, 7.5f ), 100.0f );
		red_shots[1].init ( xwings[0].position + shot2_translation, xwings[0].rotation, glm::vec3 ( 0.3f, 0.3f, 7.5f ), 100.0f );
	}
	DynArray<AModel> green_shots ( 2 );
	{
		glm::vec3 shot1_translation ( 0.3f, -0.7f, 40.0f );
		glm::vec3 shot2_translation ( -0.3f, -0.7f, 40.0f );
		shot1_translation = glm::rotate ( ties[1].rotation, shot1_translation );
		shot2_translation = glm::rotate ( ties[1].rotation, shot2_translation );
		green_shots[0].init ( ties[1].position + shot1_translation, ties[1].rotation, glm::vec3 ( 0.3f, 0.3f, 7.5f ), 100.0f );
		green_shots[1].init ( ties[1].position + shot2_translation, ties[1].rotation, glm::vec3 ( 0.3f, 0.3f, 7.5f ), 100.0f );
	}
	DynArray<BModel> billboards ( 0 );

	g_logger.log<LogLevel::eInfo> ( "Starting Main Loop" );

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

	DynArray<AnInstance> x_instances ( xwings.size() );
	DynArray<AnInstance> tie_instances ( ties.size() );
	DynArray<AnInstance> gallofree_instances ( gallofrees.size() );
	DynArray<ShotInstance> shot_instances ( red_shots.size() + green_shots.size() );
	DynArray<BillboardInstance> billboard_instances ( 0 );

	g_logger.log<LogLevel::eInfo> ( "Starting Main Loop" );

	g_state.current_time = 0.0;

	g_state.camera = Camera ( glm::vec3 ( 0.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, -40.0f, -40.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ), 120.0f, 1.0f, 1.0f, 100000.0f );
	g_state.init();
	
	setup_camerapoints(&g_state);
	
	bool explosion = false;
	bool shot_despawn = false;

	srand ( time ( NULL ) );

	g_state.timescale = 1.0 / 100.0;
	
	while ( instance->is_window_open() ) {
		g_state.update_tick();

		g_state.update_camera();

		Timing timer ( &g_logger, "Main-Loop" );

		if ( g_state.current_time >= 0.81 && !explosion ) {
			billboards.resize ( 200 );
			for ( size_t i = 0; i < billboards.size(); i++ ) {
				float size = f_rand() * 8.0f + 2.0f;
				float speed = f_rand();
				speed *= speed;
				speed *= 300.0f;
				float ttl = f_rand() * 0.5f + 0.5;

				billboards[i].init ( ties[0].position, glm::vec3 ( f_rand2(), f_rand2(), f_rand2() ), glm::vec2 ( size, size ), speed, ttl, rand() % 32 );
			}
			printf ( "\tPosition: (%f, %f, %f)\n", ties[0].position.x, ties[0].position.y, ties[0].position.z );
			explosion = true;
		}
		if ( g_state.current_time >= 0.814 && !shot_despawn ) {
			red_shots.erase ( red_shots.begin() );
			ties.erase ( ties.begin() );
			shot_despawn = true;
		}
		printf ( "Time %lf\n", g_state.current_time );

		OSEvent event;
		while ( window->eventqueue.pop ( &event ) ) {
			switch ( event.type ) {
			case OSEventType::eButton: {
				bool ispressed = event.button.action != PressAction::eRelease;
				if ( event.button.keycode == KeyCode::eF10 && event.button.action == PressAction::ePress ) {
					window->maximized() = !window->maximized();
					window->update();
				}
				if ( event.button.keycode != KeyCode::eUnknown ) {
					KeyState& keystate = g_state.basic_keystates[ ( u32 ) event.button.keycode];
					keystate.pressed = ispressed;
					keystate.time_pressed = g_state.current_time;
				}
				if ( event.button.utf8[0] ) {
					KeyState& keystate = g_state.utf32_keystates[utf8_to_utf32 ( event.button.utf8 )];
					keystate.pressed = ispressed;
					keystate.time_pressed = g_state.current_time;
				}
				KeyState& keystate = g_state.keystates[ ( u32 ) event.button.keycode];
				keystate.pressed = ispressed;
				keystate.time_pressed = g_state.current_time;

				if ( event.button.keycode == KeyCode::ePlus || event.button.keycode == KeyCode::eKPAdd ) {
					if(!g_state.debug_camera) {
						if(event.button.action == PressAction::ePress) {
							g_state.timescale = 1.0 / 10.0;
						} else if(event.button.action == PressAction::eRelease) {
							g_state.timescale = 1.0 / 100.0;
						}
					}
				}
				if ( event.button.action == PressAction::ePress && event.button.keycode == KeyCode::eC ) {
					puts ( "Camera:" );
					printf ( "\tPosition: (%f, %f, %f)\n", g_state.camera.orientation.look_at.x, g_state.camera.orientation.look_at.y, g_state.camera.orientation.look_at.z );
					printf ( "\tView-Vec: (%f, %f, %f)\n", g_state.camera.orientation.view_vector.x, g_state.camera.orientation.view_vector.y, g_state.camera.orientation.view_vector.z );
					printf ( "\tUp-Vec: (%f, %f, %f)\n", g_state.camera.orientation.up_vector.x, g_state.camera.orientation.up_vector.y, g_state.camera.orientation.up_vector.z );
				}
				if ( event.button.action == PressAction::ePress && event.button.keycode == KeyCode::eI ) {
					g_state.debug_camera = !g_state.debug_camera;
					if(g_state.debug_camera) {
						g_state.timescale = 0.0;
					} else {
						g_state.timescale = 1.0 / 10.0;
					}
				}
			}
			break;
			case OSEventType::eMouse: {
				switch ( event.mouse.action ) {
				case MouseMoveAction::eMoved:
					if ( g_state.basic_keystates[ ( u32 ) KeyCode::eMouseLeft].pressed ) {
						g_state.camera.orientation.turn ( event.mouse.deltay / 1000.0, event.mouse.deltax / 1000.0 );
					}
					break;
				case MouseMoveAction::eEntered:
					break;
				case MouseMoveAction::eLeft:
					break;
				}
			}
			break;
			case OSEventType::eScroll: {
				g_state.camera.orientation.zoom ( -event.scroll.deltay );
			}
			break;
			case OSEventType::eChar: {
				printf ( "eChar\n" );
			}
			break;
			case OSEventType::eWindow: {
				switch ( event.window.action ) {
				case WindowAction::eMoved:
					break;
				case WindowAction::eResized:
					g_state.camera.aspect = ( ( float ) event.window.x ) / ( ( float ) event.window.y );
					break;
				case WindowAction::eIconify:
					break;
				case WindowAction::eMaximize:
					break;
				case WindowAction::eFocused:
					break;
				case WindowAction::eClosed:
					break;
				}
			}
			break;
			}
		}


		for ( AModel& xwing : xwings ) {
			xwing.move ( g_state.delta_time );
		}
		for ( AModel& tie : ties ) {
			tie.move ( g_state.delta_time );
		}
		for ( AModel& gallofree : gallofrees ) {
			gallofree.move ( g_state.delta_time );
		}
		for ( AModel& shot : red_shots ) {
			shot.move ( g_state.delta_time );
		}
		for ( AModel& shot : green_shots ) {
			shot.move ( g_state.delta_time );
		}
		for ( auto it = billboards.begin(); it != billboards.end(); ) {
			BModel& billboard = *it;
			billboard.move ( g_state.delta_time );
			if ( billboard.ttl <= 0.0 ) {
				it = billboards.erase ( it );
				continue;
			}
			it++;
		}
		float camera_move_factor = glm::length ( g_state.camera.orientation.view_vector ) * g_state.delta_real_time * 0.25f;

		glm::vec3 move_vec ( 0.0f, 0.0f, 0.0f );

		if ( g_state.basic_keystates[ ( u32 ) KeyCode::eA].pressed ) move_vec.x -= camera_move_factor;
		if ( g_state.basic_keystates[ ( u32 ) KeyCode::eD].pressed ) move_vec.x += camera_move_factor;
		if ( g_state.basic_keystates[ ( u32 ) KeyCode::eS].pressed ) move_vec.z -= camera_move_factor;
		if ( g_state.basic_keystates[ ( u32 ) KeyCode::eW].pressed ) move_vec.z += camera_move_factor;

		if ( g_state.basic_keystates[ ( u32 ) KeyCode::eSpace].pressed ) move_vec.y += camera_move_factor;
		if ( g_state.basic_keystates[ ( u32 ) KeyCode::eLCntrl].pressed ) move_vec.y -= camera_move_factor;

		g_state.camera.orientation.move ( move_vec );

		//this should happen internally in a seperate thread
		//or outside in a seperate thread but probably internally is better
		instance->process_events();
		instancegroup->clear();

		//eye, center, up
		struct CameraStruct {
			glm::mat4 camera_matrixes;
			glm::mat4 inverse_camera_matrix;
			glm::vec3 camera_pos;
		} camera;
		camera.camera_matrixes = g_state.camera.v2s_mat();
		camera.inverse_camera_matrix = glm::inverse ( camera.camera_matrixes );
		camera.camera_pos = g_state.camera.orientation.view_vector * 1.0f;

		instance->update_context_data ( camera_matrix, &camera );

		glm::mat4 w2v_matrix = g_state.camera.orientation.w2v_mat();
		glm::mat4 w2v_rot_matrix = g_state.camera.orientation.w2v_rot_mat();
		global_light.direction_amb = glm::normalize ( w2v_rot_matrix * light_vector );
		global_light.direction_amb.w = 0.4f;
		global_light.color = glm::vec4 ( 0.5f, 0.5f, 0.5f, 0.2f );

		instance->update_context_data ( light_vector_context, &global_light );

		glm::mat4 m2s_matrixes[1] = {
			g_state.camera.v2s_mat() * w2v_rot_matrix
		};
		instance->update_context_data ( skybox_context, m2s_matrixes );
		instancegroup->register_instances ( skybox_instance_base_id, cube, nullptr, 1 );

		x_instances.resize ( xwings.size() );

		for ( u32 i = 0; i < xwings.size(); i++ ) {
			x_instances[i].mv2_matrix = w2v_matrix * xwings[i].m2w_mat();
			x_instances[i].normal_matrix = glm::transpose ( glm::inverse ( x_instances[i].mv2_matrix ) );
		}
		instancegroup->register_instances ( textured_instance_base_id, x_models, x_instances.data(), x_instances.size() );

		tie_instances.resize ( ties.size() );
		for ( u32 i = 0; i < ties.size(); i++ ) {
			tie_instances[i].mv2_matrix = w2v_matrix * ties[i].m2w_mat();
			tie_instances[i].normal_matrix = glm::transpose ( glm::inverse ( tie_instances[i].mv2_matrix ) );
		}
		instancegroup->register_instances ( textured_instance_base_id, tie_models, tie_instances.data(), tie_instances.size() );

		//AnInstance gallofree_matrixes1[1] = {
		//	{ glm::scale ( glm::translate ( w2v_matrix, glm::vec3 ( -4.0f, 0.0f, 0.0f ) ), glm::vec3 ( 10.0f, 10.0f, 10.0f ) ), glm::mat4() }
		//};
		//gallofree_matrixes1[0].normal_matrix = glm::transpose ( glm::inverse ( gallofree_matrixes1[0].mv2_matrix ) );
		//instancegroup->register_instances ( textured_instance_base_id, gallofree_models, gallofree_matrixes1, 1 );

		gallofree_instances.resize ( gallofrees.size() );
		for ( u32 i = 0; i < gallofrees.size(); i++ ) {
			gallofree_instances[i].mv2_matrix = w2v_matrix * gallofrees[i].m2w_mat();
			gallofree_instances[i].normal_matrix = glm::transpose ( glm::inverse ( gallofree_instances[i].mv2_matrix ) );
		}
		instancegroup->register_instances ( flat_instance_base_id, gallofree_models, gallofree_instances.data(), gallofree_instances.size() );

		shot_instances.resize ( red_shots.size() + green_shots.size() );
		{
			u32 i = 0;
			for ( ; i < red_shots.size(); i++ ) {
				shot_instances[i].mv2_matrix = w2v_matrix * red_shots[i].m2w_mat();
				shot_instances[i].umbracolor_range = glm::vec4 ( 1.0, 0.1, 0.1, 30.0 );
			}
			u32 j = 0;
			for ( ; j < green_shots.size(); j++ ) {
				shot_instances[i + j].mv2_matrix = w2v_matrix * green_shots[j].m2w_mat();
				shot_instances[i + j].umbracolor_range = glm::vec4 ( 0.1, 1.0, 0.1, 30.0 );
			}
		}
		instancegroup->register_instances ( shot_instance_base_id, cube, shot_instances.data(), shot_instances.size() );

		billboard_instances.resize ( billboards.size() );
		for ( u32 i = 0; i < billboards.size(); i++ ) {
			BModel& bm = billboards[i];
			billboard_instances[i].position = w2v_matrix * glm::vec4 ( bm.position, 1.0 );
			billboard_instances[i].scale_index_time = glm::vec4 ( bm.scale.x, bm.scale.y, ( float ) bm.index, 1.0f - ( bm.ttl / bm.max_ttl ) );
		}
		instancegroup->register_instances ( billboard_instance_base_id, square, billboard_instances.data(), billboard_instances.size() );

		instancegroup->register_instances ( dirlight_instance_base_id, fullscreen_model, nullptr, 1 );
		instancegroup->register_instances ( single_instance_base_id, fullscreen_model, nullptr, 1 );

		instance->render_bundles ( {bundle} );
	}
	window->destroy();

	delete bundle;

	destroy_instance ( instance );
	return 0;
}
