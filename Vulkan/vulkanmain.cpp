#include <stdio.h>

#include <chrono>
#include <thread>
#include <algorithm>
#include <vector>
#include <fstream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#include <fstream>

#include <render/Window.h>

//Job system: Fork -> Meet -> Join ...

//GameLogic -> Camera Query -> Graphics Queue -> other Queries -> update GPU Data

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
}


void loadImage ( VInstance* instance, std::string file, ImageWrapper * imageWrapper, u32 index, vk::CommandPool commandPool, vk::Queue queue ) {
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load ( file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if ( !pixels ) {
		throw std::runtime_error ( "failed to load texture image!" );
	}
	vk::Extent3D imageExtent = vk::Extent3D ( texWidth, texHeight, 1 );
	instance->transferData ( pixels, imageWrapper->image, vk::Offset3D ( 0, 0, 0 ), imageExtent, index, imageSize,
	                         vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, commandPool, queue );

	stbi_image_free ( pixels );
clTabCtrl}*/


#include <render/Logger.h>
#include <render/Instance.h>
#include <render/Specialization.h>
#include <chrono>
#include <thread>

Logger g_logger ( "main" );

int main ( int argc, char **argv ) {

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
	
	struct SimpleVertex {
		glm::vec3 pos;
		glm::vec3 uv;
		glm::vec3 normal;
	};
	//create model from modelbase
	Array<SimpleVertex> data_to_load = {
		{glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f)},
		{glm::vec3 ( -1.0f, 1.0f, 1.0f ), glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f)},
		{glm::vec3 ( 1.0f, -1.0f, 1.0f ), glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f)},
		{glm::vec3 ( -1.0f, -1.0f, 1.0f ), glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f)},
		
		{glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3(), glm::vec3(0.0f, 1.0f, 0.0f)},
		{glm::vec3 ( -1.0f, 1.0f, 1.0f ), glm::vec3(), glm::vec3(0.0f, 1.0f, 0.0f)},
		{glm::vec3 ( 1.0f, 1.0f, -1.0f ), glm::vec3(), glm::vec3(0.0f, 1.0f, 0.0f)},
		{glm::vec3 ( -1.0f, 1.0f, -1.0f ), glm::vec3(), glm::vec3(0.0f, 1.0f, 0.0f)},
		
		{glm::vec3 ( 1.0f, 1.0f, 1.0f ), glm::vec3(), glm::vec3(1.0f, 0.0f, 0.0f)},
		{glm::vec3 ( 1.0f, 1.0f, -1.0f ), glm::vec3(), glm::vec3(1.0f, 0.0f, 0.0f)},
		{glm::vec3 ( 1.0f, -1.0f, 1.0f ), glm::vec3(), glm::vec3(1.0f, 0.0f, 0.0f)},
		{glm::vec3 ( 1.0f, -1.0f, -1.0f ), glm::vec3(), glm::vec3(1.0f, 0.0f, 0.0f)},
		
		{glm::vec3 ( 1.0f, 1.0f, -1.0f ), glm::vec3(), glm::vec3(0.0f, 0.0f, -1.0f)},
		{glm::vec3 ( 1.0f, -1.0f, -1.0f ), glm::vec3(), glm::vec3(0.0f, 0.0f, -1.0f)},
		{glm::vec3 ( -1.0f, 1.0f, -1.0f ), glm::vec3(), glm::vec3(0.0f, 0.0f, -1.0f)},
		{glm::vec3 ( -1.0f, -1.0f, -1.0f ), glm::vec3(), glm::vec3(0.0f, 0.0f, -1.0f)},
		
		{glm::vec3 ( 1.0f, -1.0f, 1.0f ), glm::vec3(), glm::vec3(0.0f, -1.0f, 0.0f)},
		{glm::vec3 ( 1.0f, -1.0f, -1.0f ), glm::vec3(), glm::vec3(0.0f, -1.0f, 0.0f)},
		{glm::vec3 ( -1.0f, -1.0f, 1.0f ), glm::vec3(), glm::vec3(0.0f, -1.0f, 0.0f)},
		{glm::vec3 ( -1.0f, -1.0f, -1.0f ), glm::vec3(), glm::vec3(0.0f, -1.0f, 0.0f)},
		
		{glm::vec3 ( -1.0f, 1.0f, 1.0f ), glm::vec3(), glm::vec3(-1.0f, 0.0f, 0.0f)},
		{glm::vec3 ( -1.0f, -1.0f, 1.0f ), glm::vec3(), glm::vec3(-1.0f, 0.0f, 0.0f)},
		{glm::vec3 ( -1.0f, 1.0f, -1.0f ), glm::vec3(), glm::vec3(-1.0f, 0.0f, 0.0f)},
		{glm::vec3 ( -1.0f, -1.0f, -1.0f ), glm::vec3(), glm::vec3(-1.0f, 0.0f, 0.0f)},
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

	Context light_vector = newinstance->create_context(lightvector_base_id);
	contextgroup->set_context(light_vector);

	Context camera_matrix = newinstance->create_context(camera_context_base_id);
	contextgroup->set_context(camera_matrix);

	instancegroup->clear();

	Window* window = newinstance->create_window();

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
	/*{
		Array<const RenderStage*> stages ( {renderstage} );
		Array<ImageType> imagetypes = {ImageType::eColor, ImageType::eDepth};
		Array<ImageDependency> dependencies = { {0, {0, 0}, {renderstage->id, 0}}, {0, {0, 1}, {renderstage->id, 1}} };
		bundle = newinstance->create_renderbundle ( instancegroup, contextgroup, stages, imagetypes, dependencies );
	}*/
	bundle->set_rendertarget ( 0, windowimage );
	bundle->set_rendertarget ( 1, newinstance->resource_manager()->create_dependant_image ( windowimage, ImageFormat::eD24Unorm_St8U, 1.0f ) );

	glm::mat4 w2s_matrix ( 1.0f );

	printf ( "Starting Main Loop\n" );
	while ( newinstance->is_window_open() ) {
		using namespace std::chrono_literals;
		std::this_thread::sleep_for ( 20ms );
		//this should happen internally in a seperate thread
		//or outside in a seperate thread but probably internally is better
		newinstance->process_events();
		instancegroup->clear();
		
		//eye, center, up
		glm::mat4 camera_matrixes[1] = {
			glm::perspective(100.0f, 1.0f, 0.1f, 20.0f)
		};
		glm::mat4 w2v_matrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//camera_matrixes[2] = glm::transpose(glm::inverse(camera_matrixes[0] * camera_matrixes[1]));
		glm::vec4 lightvector = glm::normalize(glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
		
		newinstance->update_context_data(camera_matrix, camera_matrixes);
		newinstance->update_context_data(light_vector, &lightvector);
		
		glm::mat4 matrixes[4] = {
			glm::scale(glm::translate(w2v_matrix, glm::vec3(0.0f, 0.0f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f)),
			glm::mat4(),
			glm::scale(glm::translate(w2v_matrix, glm::vec3(0.0f, 3.0f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f)),
			glm::mat4()
		};
		matrixes[1] = glm::transpose(glm::inverse(matrixes[0]));
		matrixes[3] = glm::transpose(glm::inverse(matrixes[2]));

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
