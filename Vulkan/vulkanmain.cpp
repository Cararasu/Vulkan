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
	bool m1_pressed = false;
	bool m2_pressed = false;
	bool m3_pressed = false;
	
	bool w_pressed = false;
	bool a_pressed = false;
	bool s_pressed = false;
	bool d_pressed = false;
	bool space_pressed = false;
	bool l_cntrl_pressed = false;
	
	
	bool slowmotion = false;
	
	Camera camera;
} g_state;

int main ( int argc, char **argv ) {

	Instance* newinstance = create_instance ( "Vulkan" );

	newinstance->initialize ( InstanceOptions() );

	//preload-shaders
	newinstance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_quad_shader", "shader/quad.vert.sprv" );
	newinstance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_quad_shader", "shader/quad.frag.sprv" );

	newinstance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_textured_shader", "shader/textured.vert.sprv" );
	newinstance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_textured_shader", "shader/textured.frag.sprv" );

	newinstance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_flat_shader", "shader/flat.vert.sprv" );
	newinstance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_flat_shader", "shader/flat.frag.sprv" );

	newinstance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_skybox_shader", "shader/skybox.vert.sprv" );
	newinstance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_skybox_shader", "shader/skybox.frag.sprv" );

	newinstance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_shot_shader", "shader/shot.vert.sprv" );
	newinstance->resource_manager()->load_shader ( ShaderType::eGeometry, "geom_shot_shader", "shader/shot.geom.sprv" );
	newinstance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_shot_shader", "shader/shot.frag.sprv" );

	Monitor* primMonitor = newinstance->get_primary_monitor();

	if ( g_logger.level <= LogLevel::eDebug ) {
		g_logger.log<LogLevel::eDebug> ( "Monitors" );
		for ( Monitor* monitor : newinstance->monitors ) {
			if ( monitor == primMonitor )
				g_logger.log<LogLevel::eDebug> ( "\tPrimary Monitor | %s %dx%d", monitor->name, monitor->extend.x, monitor->extend.y );
			else
				g_logger.log<LogLevel::eDebug> ( "\t%s %dx%d", monitor->name, monitor->extend.x, monitor->extend.y );
		}
		g_logger.log<LogLevel::eDebug> ( "Devices" );
		for ( Device* device : newinstance->devices ) {
			g_logger.log<LogLevel::eDebug> ( "\t%s %" PRId32, device->name, device->rating );
		}
	}

	Model shot = newinstance->create_model ( shot_modelbase_id );
	{
		Array<glm::vec3> data_to_load = {
			glm::vec3 ( 0.0f, 0.0f, 0.0f )
		};
		Array<u16> indices = { 0 };
		newinstance->load_generic_model ( shot, data_to_load.data, data_to_load.size, indices.data, indices.size );
	}

	Model cube = newinstance->create_model ( simple_modelbase_id );
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
		newinstance->load_generic_model ( cube, data_to_load.data, data_to_load.size, indices.data, indices.size );
	}


	Image* skybox_teximage = newinstance->create_texture ( 4096, 4096, 0, 6, 1 );
	newinstance->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_PositiveZ.png", skybox_teximage, 0, 0 );
	newinstance->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_PositiveY.png", skybox_teximage, 1, 0 );
	newinstance->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_PositiveX.png", skybox_teximage, 2, 0 );
	newinstance->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_NegativeZ.png", skybox_teximage, 3, 0 );
	newinstance->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_NegativeY.png", skybox_teximage, 4, 0 );
	newinstance->load_image_to_texture ( "assets/SkyboxDark/GalaxyTex_NegativeX.png", skybox_teximage, 5, 0 );

	std::vector<SimpleVertex> vvv;
	std::vector<u32> iiii;
	std::tuple<u32, u32> vertex_data;
	std::tuple<u32, u32> index_data;

	Array<Model> x_models ( 6 );

	x_models[0] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Body.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( x_models[0], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	x_models[1] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Windows.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( x_models[1], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	x_models[2] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Wing_LB.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( x_models[2], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	x_models[3] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Wing_LT.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( x_models[3], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	x_models[4] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Wing_RB.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( x_models[4], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	x_models[5] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/X/XWing_Wing_RT.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( x_models[5], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	Array<Model> tie_models ( 6 );

	tie_models[0] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Tie/Tie_Fighter_Body.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( tie_models[0], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	tie_models[1] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Tie/Tie_Fighter_Windows.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( tie_models[1], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	tie_models[2] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Tie/Tie_Fighter_Arm_L.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( tie_models[2], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	tie_models[3] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Tie/Tie_Fighter_Arm_R.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( tie_models[3], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	tie_models[4] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Tie/Tie_Fighter_Wing_L.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( tie_models[4], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	tie_models[5] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Tie/Tie_Fighter_Wing_R.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( tie_models[5], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();

	Array<Model> gallofree_models ( 11 );

	gallofree_models[0] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/BottomEngine2.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( gallofree_models[0], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[1] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/BottomEngines1.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( gallofree_models[1], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[2] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/Cargo.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( gallofree_models[2], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[3] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/CockPit_Cockpit.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( gallofree_models[3], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[4] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/Greebles.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( gallofree_models[4], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[5] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/HullPlatesBottom.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( gallofree_models[5], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[6] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/HullPlatesTop.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( gallofree_models[6], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[7] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/SectionBottom.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( gallofree_models[7], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[8] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/SectionTop.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( gallofree_models[8], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[9] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/SectionTopInterior.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( gallofree_models[9], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();
	gallofree_models[10] = newinstance->create_model ( simple_modelbase_id );
	loadDataFile ( "assets/Gallofree/TopEngines.data", vvv, iiii, vertex_data, index_data );
	newinstance->load_generic_model ( gallofree_models[10], vvv.data(), vvv.size(), iiii.data(), iiii.size() );
	vvv.clear();
	iiii.clear();


	InstanceGroup* instancegroup = newinstance->create_instancegroup();//maybe list of modelinstancebases for optimization
	ContextGroup* contextgroup = newinstance->create_contextgroup();//maybe list of contextbases for optimization

	Context light_vector_context = newinstance->create_context ( lightvector_base_id );
	contextgroup->set_context ( light_vector_context );

	Context camera_matrix = newinstance->create_context ( camera_context_base_id );
	contextgroup->set_context ( camera_matrix );

	Context x_context = newinstance->create_context ( tex_simplemodel_context_base_id );
	Image* x_teximage = newinstance->load_image_to_texture ( "assets/X/XWing_Diffuse_01_1k.png", 4 );
	newinstance->update_context_image ( x_context, 0, x_teximage );
	newinstance->set_context ( x_models[0], x_context );
	newinstance->set_context ( x_models[1], x_context );
	newinstance->set_context ( x_models[2], x_context );
	newinstance->set_context ( x_models[3], x_context );
	newinstance->set_context ( x_models[4], x_context );
	newinstance->set_context ( x_models[5], x_context );

	Context skybox_context = newinstance->create_context ( skybox_context_base_id );
	newinstance->update_context_image ( skybox_context, 0, skybox_teximage );
	newinstance->set_context ( cube, skybox_context );

	Context tie_body_context = newinstance->create_context ( tex_simplemodel_context_base_id );
	Context tie_arm_context = newinstance->create_context ( tex_simplemodel_context_base_id );
	Context tie_wing_context = newinstance->create_context ( tex_simplemodel_context_base_id );
	Image* tie_body_teximage = newinstance->load_image_to_texture ( "assets/Tie/Tie_Fighter_Body_Diffuse_1k.png", 4 );
	Image* tie_arm_teximage = newinstance->load_image_to_texture ( "assets/Tie/Tie_Fighter_Arm_Diffuse_1k.png", 4 );
	Image* tie_wing_teximage = newinstance->load_image_to_texture ( "assets/Tie/Tie_Fighter_Wing_Diffuse_1k.png", 4 );
	newinstance->update_context_image ( tie_body_context, 0, tie_body_teximage );
	newinstance->update_context_image ( tie_arm_context, 0, tie_arm_teximage );
	newinstance->update_context_image ( tie_wing_context, 0, tie_wing_teximage );
	newinstance->set_context ( tie_models[0], tie_body_context );
	newinstance->set_context ( tie_models[1], tie_body_context );
	newinstance->set_context ( tie_models[2], tie_arm_context );
	newinstance->set_context ( tie_models[3], tie_arm_context );
	newinstance->set_context ( tie_models[4], tie_wing_context );
	newinstance->set_context ( tie_models[5], tie_wing_context );

	Context gallofree_context = newinstance->create_context ( tex_simplemodel_context_base_id );
	Context flat_gallofree_context = newinstance->create_context ( flat_simplemodel_context_base_id );
	Image* gallofree_teximage = newinstance->load_image_to_texture ( "assets/Gallofree/ScratchedMetal2.jpeg", 4 );

	newinstance->update_context_image ( gallofree_context, 0, gallofree_teximage );

	glm::vec4 color ( 0.75f, 0.75f, 0.75f, 1.0f );
	newinstance->update_context_data ( flat_gallofree_context, &color );

	newinstance->set_context ( gallofree_models[0], gallofree_context );
	newinstance->set_context ( gallofree_models[1], gallofree_context );
	newinstance->set_context ( gallofree_models[2], gallofree_context );
	newinstance->set_context ( gallofree_models[3], gallofree_context );
	newinstance->set_context ( gallofree_models[4], gallofree_context );
	newinstance->set_context ( gallofree_models[5], gallofree_context );
	newinstance->set_context ( gallofree_models[6], gallofree_context );
	newinstance->set_context ( gallofree_models[7], gallofree_context );
	newinstance->set_context ( gallofree_models[8], gallofree_context );
	newinstance->set_context ( gallofree_models[9], gallofree_context );
	newinstance->set_context ( gallofree_models[10], gallofree_context );

	newinstance->set_context ( gallofree_models[0], flat_gallofree_context );
	newinstance->set_context ( gallofree_models[1], flat_gallofree_context );
	newinstance->set_context ( gallofree_models[2], flat_gallofree_context );
	newinstance->set_context ( gallofree_models[3], flat_gallofree_context );
	newinstance->set_context ( gallofree_models[4], flat_gallofree_context );
	newinstance->set_context ( gallofree_models[5], flat_gallofree_context );
	newinstance->set_context ( gallofree_models[6], flat_gallofree_context );
	newinstance->set_context ( gallofree_models[7], flat_gallofree_context );
	newinstance->set_context ( gallofree_models[8], flat_gallofree_context );
	newinstance->set_context ( gallofree_models[9], flat_gallofree_context );
	newinstance->set_context ( gallofree_models[10], flat_gallofree_context );

	instancegroup->clear();

	g_state.camera.look_at = glm::vec3 ( 0.0f, 0.0f, 0.0f );
	g_state.camera.view_vector = glm::vec3 ( 0.0f, -40.0f, -40.0f );
	g_state.camera.up_vector = glm::vec3 ( 0.0f, 1.0f, 0.0f );
	g_state.camera.fov = 120.0f;
	g_state.camera.aspect = 1.0f;
	g_state.camera.near = 0.1f;
	g_state.camera.far = 100000.0f;


	Window* window = newinstance->create_window();

	window->on_resize = [] ( Window * window, float x, float y ) {
		g_state.camera.aspect = x / y;
	};
	window->on_mouse_moved = [] ( Window * window, double x, double y, double delta_x, double delta_y ) {
		if ( g_state.m1_pressed ) {
			g_state.camera.turn ( delta_y / 1000.0, delta_x / 1000.0 );
		}
	};
	window->on_scroll = [] ( Window * window, double delta_x, double delta_y ) {
		g_state.camera.zoom ( -delta_y );
	};
	window->on_mouse_press = [] ( Window * window, u32 button, PressAction pressed, u32 mods ) {
		if ( button == 0 ) {
			g_state.m1_pressed = pressed == PressAction::ePress;
		}
	};
	window->on_button_press = [] ( Window * window, u32 button, u32 keycode, PressAction pressed, u32 mods ) {
		switch ( button ) {
		case 65://A
			g_state.a_pressed = pressed != PressAction::eRelease;
			break;
		case 68://D
			g_state.d_pressed = pressed != PressAction::eRelease;
			break;
		case 83://S
			g_state.s_pressed = pressed != PressAction::eRelease;
			break;
		case 87://W
			g_state.w_pressed = pressed != PressAction::eRelease;
			break;
		case 32://Space
			g_state.space_pressed = pressed != PressAction::eRelease;
			break;
		case 341://left control
			g_state.l_cntrl_pressed = pressed != PressAction::eRelease;
			break;
		}
	};

	Extent2D<s32> window_size ( 1000, 600 );
	*window->position() = primMonitor->offset + ( ( primMonitor->extend / 2 ) - ( window_size / 2 ) );
	*window->size() = window_size;
	*window->visible() = true;

	window->update();
	//*window->position() = {100, 100};
	//*window->size() = {800, 800};
	*window->cursor_mode() = CursorMode::eInvisible;

	Image* windowimage = window->backed_image();

	RenderBundle* bundle = newinstance->create_main_bundle ( instancegroup, contextgroup );

	bundle->set_rendertarget ( 0, windowimage );
	bundle->set_rendertarget ( 1, newinstance->resource_manager()->create_dependant_image ( windowimage, ImageFormat::eD24Unorm_St8U, 1.0f ) );

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
		glm::vec4 spikecolor;
	};
	
	DynArray<AnInstance> x_instances(xwings.size());
	DynArray<AnInstance> tie_instances(ties.size());
	DynArray<AnInstance> gallofree_instances(gallofrees.size());
	DynArray<ShotInstance> shot_instances(red_shots.size() + green_shots.size());
	
	g_state.slowmotion = true;
	while ( newinstance->is_window_open() ) {
		
		using namespace std::chrono_literals;
		
		std::this_thread::sleep_for(10ms);
		
		last = current;
		current = std::chrono::high_resolution_clock::now();
		std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds> ( current - last );
		g_logger.log<LogLevel::eInfo> ( "%dns elapsed", ns );
		
		double delta = ns.count();
		delta /= 1000000000.0;
		
		double adjusted_delta = g_state.slowmotion ? delta / 1000.0 : delta;
		
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
		
		if(g_state.a_pressed) move_vec.x -= camera_move_factor;
		if(g_state.d_pressed) move_vec.x += camera_move_factor;
		if(g_state.s_pressed) move_vec.z -= camera_move_factor;
		if(g_state.w_pressed) move_vec.z += camera_move_factor;
		
		if(g_state.space_pressed) move_vec.y += camera_move_factor;
		if(g_state.l_cntrl_pressed) move_vec.y -= camera_move_factor;
		
		g_state.camera.move ( move_vec );
		
		//this should happen internally in a seperate thread
		//or outside in a seperate thread but probably internally is better
		newinstance->process_events();
		instancegroup->clear();

		//eye, center, up
		struct CameraStruct {
			glm::mat4 camera_matrixes;
			glm::vec3 camera_pos;
		} camera;
		camera.camera_matrixes = g_state.camera.v2s_mat();
		camera.camera_pos = g_state.camera.view_vector * 1.0f;
		
		newinstance->update_context_data ( camera_matrix, &camera );
		
		glm::mat4 w2v_matrix = g_state.camera.w2v_mat();
		global_light.direction_amb = glm::normalize ( w2v_matrix * light_vector );
		global_light.direction_amb.w = 0.4f;
		global_light.color = glm::vec4 ( 0.5f, 0.5f, 0.5f, 0.5f );
		
		newinstance->update_context_data ( light_vector_context, &global_light );

		glm::mat4 m2s_matrixes[1] = {
			g_state.camera.v2s_mat() * g_state.camera.w2v_rot_mat()
		};
		newinstance->update_context_data ( skybox_context, m2s_matrixes );
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
				shot_instances[i].umbracolor = glm::vec4(1.0, 0.0, 0.0, 0.0);
				shot_instances[i].spikecolor = glm::vec4(1.0, 1.0, 1.0, 0.0);
			}
			u32 j = 0;
			for(; j < green_shots.size(); j++) {
				shot_instances[i + j].mv2_matrix = w2v_matrix * green_shots[j].m2w_mat();
				shot_instances[i + j].umbracolor = glm::vec4(0.0, 1.0, 0.0, 0.0);
				shot_instances[i + j].spikecolor = glm::vec4(1.0, 1.0, 1.0, 0.0);
			}
		}
		instancegroup->register_instances ( shot_instance_base_id, shot, shot_instances.data(), shot_instances.size() );

		newinstance->render_bundles ( {bundle} );
	}
	window->destroy();

	delete bundle;
	destroy_instance ( newinstance );
	return 0;
}
