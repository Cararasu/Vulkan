#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <chrono>
#include <thread>
#include <algorithm>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

#include <render/Window.h>
#include <render/Logger.h>
#include <render/Instance.h>
#include <render/Specialization.h>
#include <render/Timing.h>
#include <render/UTF.h>

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

struct Camera {
	glm::vec3 look_at;
	glm::vec3 view_vector;
	glm::vec3 up_vector;
	float fov, aspect, near, far;

	void turn ( float pitch, float yaw, float roll = 0.0f ) {
		view_vector = glm::rotate ( glm::rotate ( glm::mat4 ( 1.0f ), yaw, up_vector ), pitch, glm::cross ( up_vector, view_vector ) ) * glm::vec4 ( view_vector, 0.0f );
		up_vector = glm::rotate ( glm::mat4 ( 1.0f ), roll, view_vector ) * glm::vec4 ( up_vector, 0.0f );
	}
	void move ( float forward, float sidewards, float upwards = 0.0f ) {
		glm::vec3 sideward_vec = glm::cross ( view_vector, up_vector );
		glm::vec3 forward_vec = glm::cross ( up_vector, sideward_vec );
		glm::vec3 change_vec = glm::normalize ( forward_vec ) * forward + glm::normalize ( sideward_vec ) * sidewards + glm::normalize ( up_vector ) * upwards;
		look_at += change_vec;
	}
	void move ( glm::vec3 move_vec ) {
		glm::vec3 sideward_vec = glm::cross ( view_vector, up_vector );
		glm::vec3 forward_vec = glm::cross ( up_vector, sideward_vec );
		glm::vec3 change_vec = glm::normalize ( forward_vec ) * move_vec.z + glm::normalize ( sideward_vec ) * move_vec.x + glm::normalize ( up_vector ) * move_vec.y;
		look_at += change_vec;
	}
	void move_forward ( float forward, float sidewards, float upwards ) {
		look_at += view_vector * forward;
	}
	void zoom ( float zoom ) {
		view_vector *= std::pow ( 1.1f, zoom );
		if(glm::length(view_vector) < 1.0f) view_vector = glm::normalize(view_vector);
	}
	glm::mat4 v2s_mat() {
		return glm::infinitePerspective ( fov, aspect, near );
	}
	glm::mat4 w2v_mat() {
		return glm::lookAt ( look_at - view_vector, look_at, up_vector );
	}
	glm::mat4 w2v_rot_mat() {
		return glm::lookAt ( glm::vec3 ( 0.0f, 0.0f, 0.0f ), view_vector, up_vector );
	}
};

struct GameState {
	KeyState basic_keystates[(u32)KeyCode::eMax];
	Map<u32, KeyState> utf32_keystates;
	Map<u32, KeyState> keystates;
	
	u64 current_timestamp = 0;
	u64 delta_time = 0;
	
	Camera camera;
};
GameState g_state;

#include <render/Queues.h>

int main ( int argc, char **argv ) {
	
	Instance* instance = create_instance ( "Vulkan" );
	instance->initialize ( InstanceOptions() );

	//preload-shaders
	instance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_quad_shader", "shader/quad.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_quad_shader", "shader/quad.frag.sprv" );

	instance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_textured_shader", "shader/textured.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_textured_shader", "shader/textured.frag.sprv" );

	instance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_flat_shader", "shader/flat.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_flat_shader", "shader/flat.frag.sprv" );

	instance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_skybox_shader", "shader/skybox.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_skybox_shader", "shader/skybox.frag.sprv" );

	instance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_shot_shader", "shader/shot.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eGeometry, "geom_shot_shader", "shader/shot.geom.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_shot_shader", "shader/shot.frag.sprv" );
	
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_engine_shader", "shader/engine.frag.sprv" );
	
	instance->resource_manager()->load_shader ( ShaderType::eVertex, "passthrough_shader", "shader/passthrough.vert.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "dirlight_shader", "shader/dirlight.frag.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "lightless_shader", "shader/lightless.frag.sprv" );
	
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "brightness_shader", "shader/brightness.frag.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "hbloom_shader", "shader/hbloom.frag.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "vbloom_shader", "shader/vbloom.frag.sprv" );
	instance->resource_manager()->load_shader ( ShaderType::eFragment, "composition_shader", "shader/composition.frag.sprv" );
	

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

	ResourceManager* resource_manager = instance->resource_manager();
	Image* skybox_teximage = resource_manager->create_texture ( 4096, 4096, 0, 6, 1, ImageFormat::e4Unorm8 );
	resource_manager->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_PositiveZ.png", skybox_teximage, 0, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_PositiveY.png", skybox_teximage, 1, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_PositiveX.png", skybox_teximage, 2, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_NegativeZ.png", skybox_teximage, 3, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_NegativeY.png", skybox_teximage, 4, 0 );
	resource_manager->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_NegativeX.png", skybox_teximage, 5, 0 );

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
	

	Sampler* sampler = resource_manager->create_sampler(
		FilterType::eLinear, FilterType::eLinear, FilterType::eLinear,
		EdgeHandling::eRepeat, EdgeHandling::eRepeat, EdgeHandling::eRepeat, 
		0.0f, {0.0f, 1.0f}, 0.0f, DepthComparison::eNone);

	Context light_vector_context = instance->create_context ( lightvector_base_id );
	contextgroup->set_context ( light_vector_context );

	Context camera_matrix = instance->create_context ( camera_context_base_id );
	contextgroup->set_context ( camera_matrix );
	
	Context inverse_camera_matrix_context = instance->create_context ( inverse_camera_context_base_id );
	contextgroup->set_context ( inverse_camera_matrix_context );
	

	Context x_context = instance->create_context ( tex_simplemodel_context_base_id );
	Image* x_teximage = resource_manager->load_image_to_texture ( "assets/X/XWing_Diffuse_01_1k.png", 4 );
	instance->update_context_image ( x_context, 0, x_teximage->create_use(ImagePart::eColor, {0, 1}, {0, 1}) );
	instance->update_context_sampler ( x_context, 0, sampler );
	instance->set_context ( x_models[0], x_context );
	instance->set_context ( x_models[1], x_context );
	instance->set_context ( x_models[2], x_context );
	instance->set_context ( x_models[3], x_context );
	instance->set_context ( x_models[4], x_context );
	instance->set_context ( x_models[5], x_context );

	Context skybox_context = instance->create_context ( skybox_context_base_id );
	instance->update_context_image ( skybox_context, 0, skybox_teximage->create_use(ImagePart::eColor, {0, 1}, {0, 1}) );
	instance->update_context_sampler ( skybox_context, 0, sampler );
	instance->set_context ( cube, skybox_context );

	Context tie_body_context = instance->create_context ( tex_simplemodel_context_base_id );
	Context tie_arm_context = instance->create_context ( tex_simplemodel_context_base_id );
	Context tie_wing_context = instance->create_context ( tex_simplemodel_context_base_id );
	Image* tie_body_teximage = resource_manager->load_image_to_texture ( "assets/Tie/Tie_Fighter_Body_Diffuse_1k.png", 4 );
	Image* tie_arm_teximage = resource_manager->load_image_to_texture ( "assets/Tie/Tie_Fighter_Arm_Diffuse_1k.png", 4 );
	Image* tie_wing_teximage = resource_manager->load_image_to_texture ( "assets/Tie/Tie_Fighter_Wing_Diffuse_1k.png", 4 );
	instance->update_context_image ( tie_body_context, 0, tie_body_teximage->create_use(ImagePart::eColor, {0, 1}, {0, 1}) );
	instance->update_context_sampler ( tie_body_context, 0, sampler );
	instance->update_context_image ( tie_arm_context, 0, tie_arm_teximage->create_use(ImagePart::eColor, {0, 1}, {0, 1}) );
	instance->update_context_sampler ( tie_arm_context, 0, sampler );
	instance->update_context_image ( tie_wing_context, 0, tie_wing_teximage->create_use(ImagePart::eColor, {0, 1}, {0, 1}) );
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

	instance->update_context_image ( gallofree_context, 0, gallofree_teximage->create_use(ImagePart::eColor, {0, 1}, {0, 1}) );
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

	g_state.camera.look_at = glm::vec3 ( 0.0f, 0.0f, 0.0f );
	g_state.camera.view_vector = glm::vec3 ( 0.0f, -40.0f, -40.0f );
	g_state.camera.up_vector = glm::vec3 ( 0.0f, 1.0f, 0.0f );
	g_state.camera.fov = 120.0f;
	g_state.camera.aspect = 1.0f;
	g_state.camera.near = 0.1f;
	g_state.camera.far = 100000.0f;


	Window* window = instance->create_window();

	Extent2D<s32> window_size ( 1000, 600 );
	window->position() = primMonitor->offset + ( ( primMonitor->extend / 2 ) - ( window_size / 2 ) );
	window->size() = window_size;
	window->visible() = true;

	window->update();
	//*window->position() = {100, 100};
	//*window->size() = {800, 800};
	window->cursor_mode() = CursorMode::eInvisible;

	Image* windowimage = window->backed_image( 0 );

	RenderBundle* bundle = instance->create_main_bundle ( instancegroup );

	bundle->set_window_dependency(window);

	Image* diffuse = resource_manager->create_dependant_image ( windowimage, ImageFormat::e4Unorm8, 1, 1.0f );
	bundle->get_renderstage(0)->set_renderimage ( 0, diffuse );//ambient + intensity
	bundle->get_renderstage(0)->set_renderimage ( 1, resource_manager->create_dependant_image ( windowimage, ImageFormat::e2F16, 1, 1.0f ) );//normals
	bundle->get_renderstage(0)->set_renderimage ( 2, resource_manager->create_dependant_image ( windowimage, ImageFormat::e4Unorm8, 1, 1.0f ) );//specular power + intensity + ??? + ???
	Image* lightaccumulation = resource_manager->create_dependant_image ( windowimage, ImageFormat::e4F16, 1, 1.0f );
	bundle->get_renderstage(0)->set_renderimage ( 3, lightaccumulation );//light-accumulation + specularintensity
	bundle->get_renderstage(0)->set_renderimage ( 4, resource_manager->create_dependant_image ( windowimage, ImageFormat::eD24Unorm_St8U, 1, 1.0f ) );
	bundle->get_renderstage(0)->set_contextgroup ( contextgroup );

	Image* bloomimage1 = resource_manager->create_texture ( 512, 512, 0, 1, 5, ImageFormat::e4F16);
	Image* bloomimage2 = resource_manager->create_texture ( 512, 512, 0, 1, 5, ImageFormat::e4F16);
	
	{
		Sampler* downscale_sampler = resource_manager->create_sampler(
			FilterType::eLinear, FilterType::eLinear, FilterType::eLinear,
			EdgeHandling::eMirror, EdgeHandling::eMirror, EdgeHandling::eMirror, 
			0.0f, {0.0f, 0.0f}, 0.0f, DepthComparison::eNone);
		ContextGroup* brightness_contextgroup = instance->create_contextgroup();
		Context brightness_context = instance->create_context ( postproc_context_base_id );
		
		instance->update_context_image( brightness_context, 0, lightaccumulation->create_use(ImagePart::eColor, {0, 1}, {0, 1}) );
		instance->update_context_sampler( brightness_context, 0, downscale_sampler );
		
		brightness_contextgroup->set_context( brightness_context );
		//downscale + brightness low-pass filter
		bundle->get_renderstage(1)->set_renderimage ( 0, bloomimage1);
		bundle->get_renderstage(1)->set_contextgroup ( brightness_contextgroup );
		
		bundle->get_renderstage(2)->set_renderimage ( 0, bloomimage1, 0);
		
		Sampler* bloom_sampler = resource_manager->create_sampler(
			FilterType::eNearest, FilterType::eNearest, FilterType::eNearest,
			EdgeHandling::eMirror, EdgeHandling::eMirror, EdgeHandling::eMirror, 
			0.0f, {0.0f, 5.0f}, 0.0f, DepthComparison::eNone);
		ContextGroup* vbloom_contextgroup = instance->create_contextgroup();
		
		Context vbloom_context = instance->create_context ( postproc_context_base_id );
		vbloom_contextgroup->set_context( vbloom_context );
		
		instance->update_context_image( vbloom_context, 0, bloomimage1->create_use(ImagePart::eColor, {0, 5}, {0, 1}) );
		instance->update_context_sampler( vbloom_context, 0, bloom_sampler );
		
		ContextGroup* hbloom_contextgroup = instance->create_contextgroup();
		
		Context hbloom_context = instance->create_context ( postproc_context_base_id );
		hbloom_contextgroup->set_context( hbloom_context );
		
		instance->update_context_image( hbloom_context, 0, bloomimage2->create_use(ImagePart::eColor, {0, 5}, {0, 1}) );
		instance->update_context_sampler( hbloom_context, 0, bloom_sampler );
		
		//vertical bloom
		bundle->get_renderstage(3)->set_renderimage ( 0, bloomimage2);
		bundle->get_renderstage(3)->set_contextgroup ( vbloom_contextgroup );
		
		
		//horizontal bloom
		bundle->get_renderstage(4)->set_renderimage ( 0, bloomimage1 );
		bundle->get_renderstage(4)->set_contextgroup ( hbloom_contextgroup );
		
		ContextGroup* composition_contextgroup = instance->create_contextgroup();
		
		Context composition_context = instance->create_context ( postproc_context_base_id );
		composition_contextgroup->set_context( composition_context );
		
		instance->update_context_image( composition_context, 0, bloomimage1->create_use(ImagePart::eColor, {0, 5}, {0, 1}) );
		instance->update_context_sampler( composition_context, 0, downscale_sampler );
		
		//final bloom composition
		bundle->get_renderstage(5)->set_renderimage ( 0, lightaccumulation );
		bundle->get_renderstage(5)->set_contextgroup ( composition_contextgroup );
	}
	
	bundle->get_renderstage(6)->set_renderimage ( 0, lightaccumulation );
	bundle->get_renderstage(6)->set_renderwindow ( 0, window );
	

	struct Light {
		glm::vec4 direction_amb;
		glm::vec4 color;
	} global_light;

	//setup objects
	
	constexpr float fPIE = M_PI;
	
	glm::vec4 light_vector(-0.254, -0.817, -0.51f, 0.0f);
	
	DynArray<AModel> xwings(2);
	xwings[0].init(glm::vec3 ( 5.0f, -5.0f, -10.0f ), glm::angleAxis(fPIE * 0.5f, glm::normalize(glm::vec3(0.3f, -1.0f, 0.0f))), 6.25f, 20.0f);
	xwings[1].init(glm::vec3 ( 20.0f, -0.0f, -5.0f ), glm::angleAxis(fPIE * 0.5f, glm::normalize(glm::vec3(0.25f, -1.0f, 0.0f))), 6.25f, 20.0f);
	
	DynArray<AModel> ties(2);
	ties[0].init(glm::vec3 ( -75.0f, -30.0f, -7.5f ), glm::angleAxis(fPIE * 0.5f, glm::normalize(glm::vec3(0.27f, -1.0f, 0.0f))), 4.5f, 20.0f);
	ties[1].init(glm::vec3 ( 0.0f, 100.0f, 0.0f ), glm::angleAxis(fPIE * 0.70f, glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f))), 4.5f, 20.0f);
	
	DynArray<AModel> gallofrees(1);
	gallofrees[0].init(glm::vec3 ( 30.0f, -20.0f, -30.0f ), glm::angleAxis(fPIE * 0.5f, glm::vec3(0.0f, -1.0f, 0.0f)), 45.0f, 5.0f);
	
	
	
	DynArray<AModel> red_shots(2);
	{
		glm::vec3 shot1_translation(4.6f, 0.25f, 7.0f);
		glm::vec3 shot2_translation(-4.6f, -0.4f, 7.0f);
		shot1_translation = glm::rotate(xwings[0].rotation, shot1_translation);
		shot2_translation = glm::rotate(xwings[0].rotation, shot2_translation);
		red_shots[0].init(xwings[0].position + shot1_translation, xwings[0].rotation, glm::vec3(0.3f, 0.3f, 7.5f), 200.0f);
		red_shots[1].init(xwings[0].position + shot2_translation, xwings[0].rotation, glm::vec3(0.3f, 0.3f, 7.5f), 200.0f);
	}
	DynArray<AModel> green_shots(2);
	{
		glm::vec3 shot1_translation(0.3f, -0.7f, 40.0f);
		glm::vec3 shot2_translation(-0.3f, -0.7f, 40.0f);
		shot1_translation = glm::rotate(ties[1].rotation, shot1_translation);
		shot2_translation = glm::rotate(ties[1].rotation, shot2_translation);
		green_shots[0].init(ties[1].position + shot1_translation, ties[1].rotation, glm::vec3(0.3f, 0.3f, 7.5f), 200.0f);
		green_shots[1].init(ties[1].position + shot2_translation, ties[1].rotation, glm::vec3(0.3f, 0.3f, 7.5f), 200.0f);
	}
	
	DynArray<AModel> engine(1);
	{
		glm::vec3 engine_translation(0.0f, 0.0f, 0.0f);
		engine[0].init(engine_translation, glm::mat4(), glm::vec3(5.0f, 5.0f, 5.0f), 1.0f);
	}
	
	g_logger.log<LogLevel::eInfo> ( "Starting Main Loop" );

	std::chrono::time_point<std::chrono::high_resolution_clock> current = std::chrono::high_resolution_clock::now();
	std::chrono::time_point<std::chrono::high_resolution_clock> last = current;

	struct AnInstance {
		glm::mat4 mv2_matrix;
		glm::mat4 normal_matrix;
	};
	struct ShotInstance {
		glm::mat4 mv2_matrix;
		glm::vec4 umbracolor;
	};
	
	DynArray<AnInstance> x_instances(xwings.size());
	DynArray<AnInstance> tie_instances(ties.size());
	DynArray<AnInstance> gallofree_instances(gallofrees.size());
	DynArray<ShotInstance> shot_instances(red_shots.size() + green_shots.size());
	DynArray<ShotInstance> engine_instances(engine.size());
	
	g_logger.log<LogLevel::eInfo> ( "Starting Main Loop" );
	
	while ( instance->is_window_open() ) {
		
		using namespace std::chrono_literals;
		
		last = current;
		current = std::chrono::high_resolution_clock::now();
		std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds> ( current - last );
		g_state.current_timestamp = current.time_since_epoch().count();
		g_state.delta_time = ns.count();
		
		static std::chrono::nanoseconds frametime = std::chrono::duration_cast<std::chrono::nanoseconds>(1s) / 60;
		
		std::chrono::nanoseconds waittime = frametime - ns - std::chrono::duration_cast<std::chrono::nanoseconds>(1ms);
		std::this_thread::sleep_for(waittime);
		
		Timing timer(&g_logger, "Main-Loop");
		
		double delta = ns.count();
		delta /= 1000000000.0;
		
		u32 plus = utf8_to_utf32("+");
		
		double adjusted_delta = g_state.utf32_keystates[plus].pressed ? delta : delta / 1000.0;
		
		OSEvent event;
		while(window->eventqueue.pop(&event)) {
			switch(event.type) {
			case OSEventType::eButton: {
				bool ispressed = event.button.action != PressAction::eRelease;
				if(event.button.keycode == KeyCode::eF10 && event.button.action == PressAction::ePress) {
					window->maximized() = !window->maximized();
					window->update();
				}
				if(event.button.keycode != KeyCode::eUnknown) {
					KeyState& keystate = g_state.basic_keystates[(u32)event.button.keycode];
					keystate.pressed = ispressed;
					keystate.time_pressed = g_state.current_timestamp;
				}
				if(event.button.utf8[0]) {
					KeyState& keystate = g_state.utf32_keystates[utf8_to_utf32(event.button.utf8)];
					keystate.pressed = ispressed;
					keystate.time_pressed = g_state.current_timestamp;
				}
				KeyState& keystate = g_state.keystates[(u32)event.button.keycode];
				keystate.pressed = ispressed;
				keystate.time_pressed = g_state.current_timestamp;
			}break;
			case OSEventType::eMouse: {
				switch(event.mouse.action) {
				case MouseMoveAction::eMoved:
					if ( g_state.basic_keystates[(u32)KeyCode::eMouseLeft].pressed ) {
						g_state.camera.turn ( event.mouse.deltay / 1000.0, event.mouse.deltax / 1000.0 );
					}
					break;
				case MouseMoveAction::eEntered:
					break;
				case MouseMoveAction::eLeft:
					break;
				}
			}break;
			case OSEventType::eScroll: {
				g_state.camera.zoom ( -event.scroll.deltay );
			}break;
			case OSEventType::eChar: {
				printf("eChar\n");
			}break;
			case OSEventType::eWindow: {
				switch(event.window.action) {
				case WindowAction::eMoved:
					break;
				case WindowAction::eResized:
					g_state.camera.aspect = ((float)event.window.x) / ((float)event.window.y);
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
			}break;
			}
		}
		
		
		for(AModel& xwing : xwings){
			xwing.move(adjusted_delta);
		}
		for(AModel& tie : ties){
			tie.move(adjusted_delta);
		}
		for(AModel& gallofree : gallofrees){
			gallofree.move(adjusted_delta);
		}
		for(AModel& shot : red_shots){
			shot.move(adjusted_delta);
		}
		for(AModel& shot : green_shots){
			shot.move(adjusted_delta);
		}
		float camera_move_factor = glm::length(g_state.camera.view_vector) * delta * 0.25f;
		
		glm::vec3 move_vec(0.0f, 0.0f, 0.0f);
		
		if(g_state.basic_keystates[(u32)KeyCode::eA].pressed) move_vec.x -= camera_move_factor;
		if(g_state.basic_keystates[(u32)KeyCode::eD].pressed) move_vec.x += camera_move_factor;
		if(g_state.basic_keystates[(u32)KeyCode::eS].pressed) move_vec.z -= camera_move_factor;
		if(g_state.basic_keystates[(u32)KeyCode::eW].pressed) move_vec.z += camera_move_factor;
		
		if(g_state.basic_keystates[(u32)KeyCode::eSpace].pressed) move_vec.y += camera_move_factor;
		if(g_state.basic_keystates[(u32)KeyCode::eLCntrl].pressed) move_vec.y -= camera_move_factor;
		
		g_state.camera.move ( move_vec );
		
		//this should happen internally in a seperate thread
		//or outside in a seperate thread but probably internally is better
		instance->process_events();
		instancegroup->clear();

		//eye, center, up
		struct CameraStruct {
			glm::mat4 camera_matrixes;
			glm::vec3 camera_pos;
		} camera;
		camera.camera_matrixes = g_state.camera.v2s_mat();
		camera.camera_pos = g_state.camera.view_vector * 1.0f;
		
		instance->update_context_data ( camera_matrix, &camera );
		
		glm::mat4 inverse_camera_matrix = glm::inverse (camera.camera_matrixes);
		
		instance->update_context_data ( inverse_camera_matrix_context, &inverse_camera_matrix );
		
		glm::mat4 w2v_matrix = g_state.camera.w2v_mat();
		global_light.direction_amb = glm::normalize ( w2v_matrix * light_vector );
		global_light.direction_amb.w = 0.4f;
		global_light.color = glm::vec4 ( 0.5f, 0.5f, 0.5f, 0.2f );
		
		instance->update_context_data ( light_vector_context, &global_light );

		glm::mat4 m2s_matrixes[1] = {
			g_state.camera.v2s_mat() * g_state.camera.w2v_rot_mat()
		};
		instance->update_context_data ( skybox_context, m2s_matrixes );
		instancegroup->register_instances ( skybox_instance_base_id, cube, nullptr, 1 );
		
		x_instances.resize(xwings.size());
		
		for(u32 i = 0; i < xwings.size(); i++) {
			x_instances[i].mv2_matrix = w2v_matrix * xwings[i].m2w_mat();
			x_instances[i].normal_matrix = glm::transpose ( glm::inverse ( x_instances[i].mv2_matrix ) );
		}
		instancegroup->register_instances ( textured_instance_base_id, x_models, x_instances.data(), x_instances.size() );

		tie_instances.resize(ties.size());
		for(u32 i = 0; i < ties.size(); i++) {
			tie_instances[i].mv2_matrix = w2v_matrix * ties[i].m2w_mat();
			tie_instances[i].normal_matrix = glm::transpose ( glm::inverse ( tie_instances[i].mv2_matrix ) );
		}
		instancegroup->register_instances ( textured_instance_base_id, tie_models, tie_instances.data(), tie_instances.size() );

		//AnInstance gallofree_matrixes1[1] = {
		//	{ glm::scale ( glm::translate ( w2v_matrix, glm::vec3 ( -4.0f, 0.0f, 0.0f ) ), glm::vec3 ( 10.0f, 10.0f, 10.0f ) ), glm::mat4() }
		//};
		//gallofree_matrixes1[0].normal_matrix = glm::transpose ( glm::inverse ( gallofree_matrixes1[0].mv2_matrix ) );
		//instancegroup->register_instances ( textured_instance_base_id, gallofree_models, gallofree_matrixes1, 1 );

		gallofree_instances.resize(gallofrees.size());
		for(u32 i = 0; i < gallofrees.size(); i++) {
			gallofree_instances[i].mv2_matrix = w2v_matrix * gallofrees[i].m2w_mat();
			gallofree_instances[i].normal_matrix = glm::transpose ( glm::inverse ( gallofree_instances[i].mv2_matrix ) );
		}
		instancegroup->register_instances ( flat_instance_base_id, gallofree_models, gallofree_instances.data(), gallofree_instances.size() );

		shot_instances.resize(red_shots.size() + green_shots.size());
		{
			u32 i = 0;
			for(; i < red_shots.size(); i++) {
				shot_instances[i].mv2_matrix = w2v_matrix * red_shots[i].m2w_mat();
				shot_instances[i].umbracolor = glm::vec4(1.0, 0.1, 0.1, 0.33);
			}
			u32 j = 0;
			for(; j < green_shots.size(); j++) {
				shot_instances[i + j].mv2_matrix = w2v_matrix * green_shots[j].m2w_mat();
				shot_instances[i + j].umbracolor = glm::vec4(0.1, 1.0, 0.1, 0.33);
			}
		}
		instancegroup->register_instances ( shot_instance_base_id, dot_model, shot_instances.data(), shot_instances.size() );

		engine_instances.resize(engine.size());
		{
			for(u32 i = 0; i < engine_instances.size(); i++) {
				engine_instances[i].mv2_matrix = w2v_matrix * engine[i].m2w_mat();
				engine_instances[i].umbracolor = glm::vec4(1.0, 0.0, 0.0, 0.0);
			}
		}
		//instancegroup->register_instances ( engine_instance_base_id, dot_model, engine_instances.data(), engine_instances.size() );
		
		instancegroup->register_instances ( dirlight_instance_base_id, fullscreen_model, nullptr, 1 );
		instancegroup->register_instances ( single_instance_base_id, fullscreen_model, nullptr, 1 );
		
		instance->render_bundles ( {bundle} );
	}
	window->destroy();

	delete bundle;

	destroy_instance ( instance );
	return 0;
}
