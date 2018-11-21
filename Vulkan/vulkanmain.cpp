#include <stdio.h>

#include <chrono>
#include <thread>
#include <algorithm>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <render/Window.h>

//Job system: Fork -> Meet -> Join ...

//GameLogic -> Camera Query -> Graphics Queue -> other Queries -> update GPU Data



struct SimpleVertex {
	glm::vec3 pos;
	glm::vec3 uv;
	glm::vec3 normal;
};
#include <fstream>
#include <sstream>

void loadObjFile ( std::string file, std::vector<SimpleVertex>& s_vertices, std::vector<u32> indices ) {
	std::ifstream input ( file, std::ios::binary );
	std::string line;

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> texture_coords;
	
	
	
	while ( std::getline ( input, line ) ) {
		size_t word_start = line.find_first_not_of ( " \t" );
		size_t word_end = line.find_first_of ( " \t", word_start );
		if ( word_start == std::string::npos || word_end == std::string::npos ) {
			continue;
		} else if ( line[word_start] == '#' )
			continue;
		else if ( line[word_start] == 's' ) //ignore for now
			continue;
		else if ( line[word_start] == 'v' ) {
			glm::vec3 vec;
			if ( line[word_start + 1] == 't' ) {
				sscanf ( line.c_str() + word_start + 2, "%f %f %f", &vec.x, &vec.y, &vec.z );
				texture_coords.push_back ( vec );
			} else if ( line[word_start + 1] == 'n' ) {
				sscanf ( line.c_str() + word_start + 2, "%f %f %f", &vec.x, &vec.y, &vec.z );
				normals.push_back ( vec );
			} else {
				sscanf ( line.c_str() + word_start + 1, "%f %f %f", &vec.x, &vec.y, &vec.z );
				vertices.push_back ( vec );
			}
		} else if ( line[word_start] == 'f' ) {
			glm::uvec3 pos_index;
			glm::uvec3 texture_index;
			glm::uvec3 normal_index;
			sscanf ( line.c_str() + word_start + 1,
			         "%u/%u/%u %u/%u/%u %u/%u/%u",
			         &pos_index.x, &texture_index.x, &normal_index.x,
			         &pos_index.y, &texture_index.y, &normal_index.y,
			         &pos_index.z, &texture_index.z, &normal_index.z );
			printf("%u, %u, %u\n", pos_index.x, pos_index.y, pos_index.z);
		} else if ( line[word_start] == 'g' ) {

		} else {
			printf ( "%s\n", line.c_str() );
		}
	}
}


/*
u32 loadDataFile ( std::string file, OpaqueObjectDispatcher* dispatcher ) {

	std::ifstream input ( file, std::ios::binary );

	std::string str;
	std::getline ( input, str, '\0' );
	u32 vertexCount = 0;
	input.read ( reinterpret_cast<char*> ( &vertexCount ), sizeof ( u32 ) );

	std::vector<Vertex> vs;
	vs.resize ( vertexCount );
	for ( size_t i = 0; i < vertexCount; i++ ) {
		Vertex v;
		input.read ( reinterpret_cast<char*> ( &v.pos[0] ), sizeof ( float ) );
		input.read ( reinterpret_cast<char*> ( &v.uv[0] ), sizeof ( float ) );
		input.read ( reinterpret_cast<char*> ( &v.normal[0] ), sizeof ( float ) );

		input.read ( reinterpret_cast<char*> ( &v.pos[1] ), sizeof ( float ) );
		input.read ( reinterpret_cast<char*> ( &v.uv[1] ), sizeof ( float ) );
		input.read ( reinterpret_cast<char*> ( &v.normal[1] ), sizeof ( float ) );

		input.read ( reinterpret_cast<char*> ( &v.pos[2] ), sizeof ( float ) );
		input.read ( reinterpret_cast<char*> ( &v.uv[2] ), sizeof ( float ) );
		input.read ( reinterpret_cast<char*> ( &v.normal[2] ), sizeof ( float ) );

		vs[i] = v;
	}
	u32 indexCount;
	input.read ( reinterpret_cast<char*> ( &indexCount ), sizeof ( u32 ) );

	std::vector<u32> is;
	is.resize ( indexCount );

	input.read ( reinterpret_cast<char*> ( is.data() ), sizeof ( u32 ) *indexCount );

	ObjectPartData data;
	data.diffuseTexId = 0;
	return g_thread_data.dispatcher.add_object ( vs, is, data );
}

LoadedObjectData<Vertex, u32> loadDataFile ( std::string file, ObjectVertexData<Vertex, u32>& vertex_data ) {

	std::ifstream input ( file, std::ios::binary );

	std::string str;
	std::getline ( input, str, '\0' );
	u32 vertexCount = 0;
	LoadedObjectData<Vertex, u32> objData;
	objData.vertex_offset = vertex_data.vertices.size();
	objData.index_offset = vertex_data.indices.size();
	input.read ( reinterpret_cast<char*> ( &vertexCount ), sizeof ( u32 ) );


	vertex_data.vertices.resize ( vertex_data.vertices.size() + vertexCount );
	for ( size_t i = 0; i < vertexCount; i++ ) {
		Vertex v;
		input.read ( reinterpret_cast<char*> ( &v.pos[0] ), sizeof ( float ) );
		input.read ( reinterpret_cast<char*> ( &v.uv[0] ), sizeof ( float ) );
		input.read ( reinterpret_cast<char*> ( &v.normal[0] ), sizeof ( float ) );

		input.read ( reinterpret_cast<char*> ( &v.pos[1] ), sizeof ( float ) );
		input.read ( reinterpret_cast<char*> ( &v.uv[1] ), sizeof ( float ) );
		input.read ( reinterpret_cast<char*> ( &v.normal[1] ), sizeof ( float ) );

		input.read ( reinterpret_cast<char*> ( &v.pos[2] ), sizeof ( float ) );
		input.read ( reinterpret_cast<char*> ( &v.uv[2] ), sizeof ( float ) );
		input.read ( reinterpret_cast<char*> ( &v.normal[2] ), sizeof ( float ) );

		vertex_data.vertices[i] = v;
	}
	u32 indexCount;
	input.read ( reinterpret_cast<char*> ( &indexCount ), sizeof ( u32 ) );
	objData.index_size = indexCount;
	std::vector<u32> is;
	vertex_data.indices.resize ( vertex_data.indices.size() + indexCount );

	input.read ( reinterpret_cast<char*> ( &vertex_data.indices[objData.index_offset] ), sizeof ( u32 ) *indexCount );
	return objData;
}*/

struct Camera {
	glm::vec3 look_at;
	glm::vec3 view_vector;
	glm::vec3 up_vector;
	float fov, aspect, near, far;

	void turn ( float pitch, float yaw, float roll = 0.0f ) {
		view_vector = glm::rotate ( glm::rotate ( glm::mat4 ( 1.0f ), yaw, up_vector ), pitch, glm::cross ( up_vector, view_vector ) ) * glm::vec4 ( view_vector, 0.0f );
		up_vector = glm::rotate ( glm::mat4 ( 1.0f ), roll, view_vector ) * glm::vec4 ( up_vector, 0.0f );
	}
	void move ( float forward, float sidewards ) {
		glm::vec3 sideward_vec = glm::cross ( view_vector, up_vector );
		glm::vec3 forward_vec = glm::cross ( up_vector, sideward_vec );
		glm::vec3 change_vec = glm::normalize ( forward_vec ) * forward + glm::normalize ( sideward_vec ) * sidewards;
		look_at += change_vec;
	}
	void move_forward ( float forward, float sidewards, float upwards ) {
		look_at += view_vector * forward;
	}
	void zoom ( float zoom ) {
		view_vector *= std::pow ( 1.1f, zoom );
	}
	glm::mat4 v2s_mat() {
		return glm::perspective ( fov, aspect, near, far );
	}
	glm::mat4 w2v_mat() {
		return glm::lookAt ( look_at - view_vector, look_at, up_vector );
	}
};

struct GameState {
	bool m1_pressed = false;
	bool m2_pressed = false;
	bool m3_pressed = false;

	Camera camera;
} g_state;


#include <render/Logger.h>
#include <render/Instance.h>
#include <render/Specialization.h>
#include <chrono>
#include <thread>

Logger g_logger ( "main" );

int main ( int argc, char **argv ) {

	std::vector<SimpleVertex> vvv;
	std::vector<u32> iiii;
	loadObjFile ( "assets/X/XWing.obj", vvv, iiii );
	return 0;

	Instance* newinstance = create_instance ( "Vulkan" );

	newinstance->initialize ( InstanceOptions() );

	//preload-shaders
	newinstance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_quad_shader", "shader/quad.vert.sprv" );
	newinstance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_quad_shader", "shader/quad.frag.sprv" );

	newinstance->resource_manager()->load_shader ( ShaderType::eVertex, "vert_shader", "shader/tri.vert.sprv" );
	newinstance->resource_manager()->load_shader ( ShaderType::eFragment, "frag_shader", "shader/tri.frag.sprv" );

	Monitor* primMonitor = newinstance->get_primary_monitor();

	printf ( "Monitors\n" );
	for ( Monitor* monitor : newinstance->monitors ) {
		if ( monitor == primMonitor )
			printf ( "\tPrimary Monitor | %s %dx%d\n", monitor->name, monitor->extend.x, monitor->extend.y );
		else
			printf ( "\t%s %dx%d\n", monitor->name, monitor->extend.x, monitor->extend.y );
	}
	printf ( "Devices\n" );
	for ( Device* device : newinstance->devices ) {
		printf ( "\t%s %" PRId32 "\n", device->name, device->rating );
	}

	//create model from modelbase
	Array<SimpleVertex> data_to_load = {
		{glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3 ( 1.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },
		{glm::vec3 ( 1.0f, -1.0f, 1.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },
		{glm::vec3 ( -1.0f, 1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },
		{glm::vec3 ( -1.0f, -1.0f, 1.0f ), glm::vec3 ( 0.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, 1.0f ) },

		{glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3 ( 1.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ) },
		{glm::vec3 ( -1.0f, 1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ) },
		{glm::vec3 ( 1.0f, 1.0f, -1.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ) },
		{glm::vec3 ( -1.0f, 1.0f, -1.0f ), glm::vec3 ( 0.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ) },

		{glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3 ( 1.0f, 1.0f, 0.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ) },
		{glm::vec3 ( 1.0f, 1.0f, -1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ) },
		{glm::vec3 ( 1.0f, -1.0f, 1.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ) },
		{glm::vec3 ( 1.0f, -1.0f, -1.0f ), glm::vec3 ( 0.0f, 0.0f, 0.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ) },

		{glm::vec3 ( 1.0f, 1.0f, -1.0f ), glm::vec3 ( 1.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ) },
		{glm::vec3 ( -1.0f, 1.0f, -1.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ) },
		{glm::vec3 ( 1.0f, -1.0f, -1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ) },
		{glm::vec3 ( -1.0f, -1.0f, -1.0f ), glm::vec3 ( 0.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, 0.0f, -1.0f ) },

		{glm::vec3 ( 1.0f, -1.0f, 1.0f ), glm::vec3 ( 1.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, -1.0f, 0.0f ) },
		{glm::vec3 ( 1.0f, -1.0f, -1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ), glm::vec3 ( 0.0f, -1.0f, 0.0f ) },
		{glm::vec3 ( -1.0f, -1.0f, 1.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, -1.0f, 0.0f ) },
		{glm::vec3 ( -1.0f, -1.0f, -1.0f ), glm::vec3 ( 0.0f, 0.0f, 0.0f ), glm::vec3 ( 0.0f, -1.0f, 0.0f ) },

		{glm::vec3 ( -1.0f, 1.0f, 1.0f ), glm::vec3 ( 1.0f, 1.0f, 0.0f ), glm::vec3 ( -1.0f, 0.0f, 0.0f ) },
		{glm::vec3 ( -1.0f, -1.0f, 1.0f ), glm::vec3 ( 0.0f, 1.0f, 0.0f ), glm::vec3 ( -1.0f, 0.0f, 0.0f ) },
		{glm::vec3 ( -1.0f, 1.0f, -1.0f ), glm::vec3 ( 1.0f, 0.0f, 0.0f ), glm::vec3 ( -1.0f, 0.0f, 0.0f ) },
		{glm::vec3 ( -1.0f, -1.0f, -1.0f ), glm::vec3 ( 0.0f, 0.0f, 0.0f ), glm::vec3 ( -1.0f, 0.0f, 0.0f ) },
	};

	Array<u16> indices = {
		0, 1, 2, 2, 1, 3,
		4, 5, 6, 6, 5, 7,
		8, 9, 10, 10, 9, 11,
		12, 13, 14, 14, 13, 15,
		16, 17, 18, 18, 17, 19,
		20, 21, 22, 22, 21, 23,
	};

	Model model = newinstance->create_model ( simplemodel_base_id );
	newinstance->load_generic_model ( model, data_to_load.data, data_to_load.size, indices.data, indices.size );

	//create instance from model
	//const ModelInstanceBase* mod_instance = newinstance->register_modelinstancebase ( model, matrix_def );

	InstanceGroup* instancegroup = newinstance->create_instancegroup();//maybe list of modelinstancebases for optimization
	ContextGroup* contextgroup = newinstance->create_contextgroup();//maybe list of contextbases for optimization

	Context light_vector = newinstance->create_context ( lightvector_base_id );
	contextgroup->set_context ( light_vector );

	Context camera_matrix = newinstance->create_context ( camera_context_base_id );
	contextgroup->set_context ( camera_matrix );

	Context simplemodel_context = newinstance->create_context ( simplemodel_context_base_id );
	Image* teximage = newinstance->load_image_to_texture ( "assets/X/XWing_Diffuse_01_1k.png", 4 );
	newinstance->update_context_image ( simplemodel_context, 0, teximage );

	newinstance->set_context ( model, simplemodel_context );



	instancegroup->clear();

	g_state.camera.look_at = glm::vec3 ( 0.0f, 0.0f, 0.0f );
	g_state.camera.view_vector = glm::vec3 ( 0.0f, -20.0f, -20.0f );
	g_state.camera.up_vector = glm::vec3 ( 0.0f, 0.0f, 1.0f );
	g_state.camera.fov = 100.0f;
	g_state.camera.aspect = 1.0f;
	g_state.camera.near = 0.1f;
	g_state.camera.far = 1000.0f;


	Window* window = newinstance->create_window();

	bool m_pressed = false;

	window->on_resize = [] ( Window * window, float x, float y ) {
		g_state.camera.aspect = y / x;
	};
	window->on_mouse_moved = [&m_pressed] ( Window * window, double x, double y, double delta_x, double delta_y ) {
		if ( g_state.m1_pressed ) {
			g_state.camera.turn ( delta_y / 1000.0, delta_x / 1000.0 );
		}
	};
	window->on_scroll = [] ( Window * window, double delta_x, double delta_y ) {
		g_state.camera.zoom ( -delta_y );
	};
	window->on_mouse_press = [&m_pressed] ( Window * window, u32 button, PressAction pressed, u32 mods ) {
		if ( button == 0 ) {
			g_state.m1_pressed = pressed == PressAction::ePress;
		}
	};
	window->on_button_press = [&m_pressed] ( Window * window, u32 button, u32 keycode, PressAction pressed, u32 mods ) {
		if ( pressed != PressAction::eRelease ) {
			switch ( button ) {
			case 65://A
				g_state.camera.move ( 0.0f, 0.1f );
				break;
			case 68://D
				g_state.camera.move ( 0.0f, -0.1f );
				break;
			case 83://S
				g_state.camera.move ( -0.1f, 0.0f );
				break;
			case 87://W
				g_state.camera.move ( 0.1f, 0.0f );
				break;
			}
		}
	};

	Extent2D<s32> window_size ( 1000, 600 );
	*window->position() = primMonitor->offset + ( ( primMonitor->extend / 2 ) - ( window_size / 2 ) );
	*window->size() = window_size;
	*window->visible() = true;

	//newinstance->create_window_section ( WindowSectionType::eUI )
	//window->root_section (  );
	window->update();
	//*window->position() = {100, 100};
	//*window->size() = {800, 800};
	*window->cursor_mode() = CursorMode::eInvisible;

	Image* windowimage = window->backed_image();

	RenderBundle* bundle = newinstance->create_main_bundle ( instancegroup, contextgroup );

	bundle->set_rendertarget ( 0, windowimage );
	bundle->set_rendertarget ( 1, newinstance->resource_manager()->create_dependant_image ( windowimage, ImageFormat::eD24Unorm_St8U, 1.0f ) );

	g_logger.log<LogLevel::eInfo> ( "Starting Main Loop\n" );
	while ( newinstance->is_window_open() ) {
		using namespace std::chrono_literals;
		std::this_thread::sleep_for ( 10ms );
		//this should happen internally in a seperate thread
		//or outside in a seperate thread but probably internally is better
		newinstance->process_events();
		instancegroup->clear();

		//eye, center, up
		glm::mat4 camera_matrixes[1] = {
			g_state.camera.v2s_mat()
		};
		glm::mat4 w2v_matrix = g_state.camera.w2v_mat();
		glm::vec4 lightvector = glm::normalize ( glm::vec4 ( 0.0f, 0.0f, 1.0f, 0.0f ) );

		newinstance->update_context_data ( camera_matrix, camera_matrixes );
		newinstance->update_context_data ( light_vector, &lightvector );

		glm::mat4 matrixes[4] = {
			glm::scale ( glm::translate ( w2v_matrix, glm::vec3 ( 0.0f, 0.0f, 0.0f ) ), glm::vec3 ( 1.0f, 1.0f, 1.0f ) ),
			glm::mat4(),
			glm::scale ( glm::translate ( w2v_matrix, glm::vec3 ( 0.0f, 3.0f, 0.0f ) ), glm::vec3 ( 1.0f, 1.0f, 1.0f ) ),
			glm::mat4()
		};
		matrixes[1] = glm::transpose ( glm::inverse ( matrixes[0] ) );
		matrixes[3] = glm::transpose ( glm::inverse ( matrixes[2] ) );

		instancegroup->register_instances ( simplemodel_instance_base_id, model, matrixes, 2 );

		newinstance->render_bundles ( {bundle} );
		//newinstance->present_windows();
	}
	window->destroy();

	delete bundle;
	destroy_instance ( newinstance );
	return 0;

	/*
		std::vector<u32> TiePartIds;
		TiePartIds.push_back ( loadDataFile ( "assets/Tie/Tie_Fighter_Body.data", &g_thread_data.dispatcher ) );
		g_thread_data.dispatcher.parts.back().data.diffuseTexId = 0;
		TiePartIds.push_back ( loadDataFile ( "assets/Tie/Tie_Fighter_Arm_L.data", &g_thread_data.dispatcher ) );
		g_thread_data.dispatcher.parts.back().data.diffuseTexId = 1;
		TiePartIds.push_back ( loadDataFile ( "assets/Tie/Tie_Fighter_Arm_R.data", &g_thread_data.dispatcher ) );
		g_thread_data.dispatcher.parts.back().data.diffuseTexId = 1;
		TiePartIds.push_back ( loadDataFile ( "assets/Tie/Tie_Fighter_Wing_L.data", &g_thread_data.dispatcher ) );
		g_thread_data.dispatcher.parts.back().data.diffuseTexId = 2;
		TiePartIds.push_back ( loadDataFile ( "assets/Tie/Tie_Fighter_Wing_R.data", &g_thread_data.dispatcher ) );
		g_thread_data.dispatcher.parts.back().data.diffuseTexId = 2;
		TiePartIds.push_back ( loadDataFile ( "assets/Tie_Fighter_Windows.data", &g_thread_data.dispatcher ) );
		g_thread_data.dispatcher.parts.back().data.diffuseTexId = 2;

		std::vector<u32> XPartIds;
		XPartIds.push_back ( loadDataFile ( "assets/X/XWing_Body.data", &g_thread_data.dispatcher ) );
		g_thread_data.dispatcher.parts.back().data.diffuseTexId = 3;
		XPartIds.push_back ( loadDataFile ( "assets/X/XWing_Windows.data", &g_thread_data.dispatcher ) );
		g_thread_data.dispatcher.parts.back().data.diffuseTexId = 3;
		XPartIds.push_back ( loadDataFile ( "assets/X/XWing_Wing_LB.data", &g_thread_data.dispatcher ) );
		g_thread_data.dispatcher.parts.back().data.diffuseTexId = 3;
		XPartIds.push_back ( loadDataFile ( "assets/X/XWing_Wing_LT.data", &g_thread_data.dispatcher ) );
		g_thread_data.dispatcher.parts.back().data.diffuseTexId = 3;
		XPartIds.push_back ( loadDataFile ( "assets/X/XWing_Wing_RB.data", &g_thread_data.dispatcher ) );
		g_thread_data.dispatcher.parts.back().data.diffuseTexId = 3;
		XPartIds.push_back ( loadDataFile ( "assets/X/XWing_Wing_RT.data", &g_thread_data.dispatcher ) );
		g_thread_data.dispatcher.parts.back().data.diffuseTexId = 3;



		PerspectiveViewPort<float> viewport;

		viewport.m_viewvector = {0.0f, 0.2f, -1.4f};
		viewport.m_focalpoint = {0.0f, 0.0f, 0.0f};
		viewport.m_upvector = {0.0f, 1.0f, 0.0f};
		viewport.m_distance = 2.0f;
		viewport.m_near = 0.01f;
		viewport.m_far = 100.0f;
		viewport.m_aspect = 1.0f;

		loadImage ( instance, "assets/Tie/Tie_Fighter_Body_Diffuse.png", imageWrapper, 0, transferCommandPool, instance->tqueue->transferQueue );
		loadImage ( instance, "assets/Tie/Tie_Fighter_Arm_Diffuse.png", imageWrapper, 1, transferCommandPool, instance->tqueue->transferQueue );
		loadImage ( instance, "assets/Tie/Tie_Fighter_Wing_Diffuse.png", imageWrapper, 2, transferCommandPool, instance->tqueue->transferQueue );
		loadImage ( instance, "assets/X/XWing_Diffuse.png", imageWrapper, 3, transferCommandPool, instance->tqueue->transferQueue );

		//@TODO Memory Barrier for graphics queue
		//instance->tqueue->waitForFinish();
		//vWindow->pgcQueue->waitForFinish();

		vk::Sampler sampler;
		{
			vk::SamplerCreateInfo samplerInfo (
			    vk::SamplerCreateFlags(), vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
			    vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
			    0.0f,
			    VK_TRUE, 16, //anisotrophic
			    VK_FALSE, vk::CompareOp::eAlways, //compare
			    0.0f, 12.0f, //lod
			    vk::BorderColor::eFloatOpaqueBlack, VK_FALSE
			);
		}
		*/

	return 0;
}
