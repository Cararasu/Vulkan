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

template<typename T>
struct ProxyObject {
	T obj;
	T* proxyObj = nullptr;

	inline void update() {
		if ( proxyObj ) memcpy ( &obj, proxyObj, sizeof ( T ) );
		printf ( "Proxy_update\n" );
	}
};
//Job system: Fork -> Meet -> Join ...

//Stages
//	GameTick
//		GameLogic
//	FrameTick
//	GraphicsTick
//	UpdatePhase

//Stages
//	WorkStage
//		GraphicsTick(working on obj)
//		LogicTick(working on ProxyObjects)
//			FrameTick + opt. GameTick
//	SubmitStage
//		UpdateStage(proxyObj -> obj)
//		SubmitTick(data -> gpu)

//GameLogic -> Camera Query -> Graphics Queue -> other Queries -> update GPU Data

#include <stdint.h>

struct DefDrawData {
	glm::dvec3 pos, rot, scale;
};
struct CullingStrategy {
//CameraFlagList
//QuadTree(Box(Index of Obj))

	//virtual void update(ObjectStore<DrawObject>* objectStore) = 0;

	virtual void addToStrat ( DefDrawData& obj ) = 0;
	virtual void removeFromStrat ( DefDrawData& obj ) = 0;
};
struct RenderContext {
	//ObjectStore<DrawObject, u32> objStore;
	CullingStrategy* strategy;

	template<typename OBJDATA>
	u64 createObject();
	template<typename OBJDATA>
	void removeObject ( u64 id );
};

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
}*/

struct Orientation {
	glm::dvec3 position, rotation, scale;
};
struct BoundingBox {
	glm::dvec3 pos_size, neg_size;
};
struct SimpleObject {
	Orientation orientation;
	BoundingBox bounding_box;
};

/*
 * VGlobal
 * 		VGlobal -> creates -> VInstance
 * VInstance - Device, Renderpasses, Queues, Vulkan-Object-Definitions, ...
 * 		VInstance -> creates -> RenderEnvironment
 * RenderEnvironment - Buffers, Descriptorsets, Objects, ...
 * 		RenderEnvironment -> creates -> PerFrameStuff
 * PerFrameStuff - OnEnd-Functions(After the Frame is definetly over(Fence)) they are called and clean up
 * 		PerFrameStuff -> creates -> PerThreadStuff
 * PerThreadStuff - Instances, ...
 * */

/*
 * Data organisation
 *     ObjectStorage has the Objects with BoundingBoxes and the Orientation
 *         Organised by Type
 *         update calls update the proxyobject and then call individual update
 *         execute calls execute on every object
 *         create VertexInputAttributeDescription for per Object stuff
 *     VerticesIndicesStorage
 *         Organised maybe also by type but organised in different buffers, bufferid/offset/count
 *
 *         generate VertexInputAttributeDescriptions from this
 * */

#include <render/Instance.h>
#include <chrono>
#include <thread>

/*
 * ImageStore
 *
 * ResourceManager
 *     ResourceHandle loadImage()
 *     ResourceHandle loadModel() -> buffer with indices, vertices, uvs
 *     ResourceHandle load...
 *
 * setImage(ResourceHandle image, ResourceHandle default)
 *
 * Deferred Shading:
 *
 * Color rgb + specular? 16i VK_FORMAT_R16G16B16A16_SFLOAT/VK_FORMAT_R16G16B16A16_UNORM
 * depth 1 32f VK_FORMAT_R32_SFLOAT
 * normals 3 10f2i VK_FORMAT_A2B10G10R10_UNORM_PACK32
 * specular 1f?
 *
 */
int main ( int argc, char **argv ) {

	Instance* newinstance = initialize_instance ( "Vulkan" );

	newinstance->initialize();

	Monitor* primMonitor = newinstance->get_primary_monitor();

	printf ( "Monitors\n" );
	for ( Monitor* monitor : newinstance->get_monitors() ) {
		if ( monitor == primMonitor )
			printf ( "\tPrimary Monitor | %s %dx%d\n", monitor->name, monitor->extend.x, monitor->extend.y );
		else
			printf ( "\t%s %dx%d\n", monitor->name, monitor->extend.x, monitor->extend.y );
	}
	printf ( "Devices\n" );
	for ( Device* device : newinstance->get_devices() ) {
		printf ( "\t%s %" PRId32 "\n", device->name, device->rating );
	}

	DataGroupDef groupdef;
	ContextBase contextbase;
	ModelBase modelbase;

	//create datagroups
	groupdef = {1, { {ValueType::eF32Vec3, 1, 0}, {ValueType::eF32Vec3, 1, 3 * 4}, {ValueType::eF32Vec3, 1, 6 * 4} }, 9 * 4, 1};
	RId vertex_id = newinstance->register_datagroupdef ( groupdef );

	groupdef = {2, { {ValueType::eF32Mat4, 1, 0} }, 16 * 4, 1};
	RId matrix_id = newinstance->register_datagroupdef ( groupdef );

	//create contextbase from datagroups
	RId w2smatrix_id = newinstance->register_contextbase ( matrix_id );
	RId m2wmatrix_id = newinstance->register_contextbase ( matrix_id );

	//create modelbase from datagroup and contextbases
	RId simplemodel_id = newinstance->register_modelbase ( vertex_id );
	//create model from modelbase
	Array<glm::vec3> data_to_load = {
		glm::vec3 ( 0.5f, 0.5f, 0.5f ), glm::vec3(), glm::vec3(),
		glm::vec3 ( -0.5f, 0.5f, 0.5f ), glm::vec3(), glm::vec3(),
		glm::vec3 ( 0.5f, -0.5f, 0.5f ), glm::vec3(), glm::vec3(),
		glm::vec3 ( -0.5f, -0.5f, 0.5f ), glm::vec3(), glm::vec3(),
		glm::vec3 ( 0.5f, 0.5f, -0.5f ), glm::vec3(), glm::vec3(),
		glm::vec3 ( -0.5f, 0.5f, -0.5f ), glm::vec3(), glm::vec3(),
		glm::vec3 ( 0.5f, -0.5f, -0.5f ), glm::vec3(), glm::vec3(),
		glm::vec3 (- 0.5f, -0.5f, -0.5f ), glm::vec3(), glm::vec3(),
	};

	Array<u16> indices = {0, 1, 2};

	const Model model = newinstance->load_generic_model ( simplemodel_id, (u8*)data_to_load.data(), 24, indices.data(), 3 );

	//create instance from model
	RId mod_instance = newinstance->register_modelinstancebase ( model, matrix_id );

	const ModelInstance instance = newinstance->create_instance ( mod_instance );
	//ModelInstance* mod_instance = model->create_instance ();

	//VulkanStagingMemory
	//	list of buffers that grows if the data is not enough
	//	u8* create_transfer_job(size, dstbuffer);

	//get context from model and is updated once?
	//Context* context = &mod_instance->contexts[0];
	//glm::mat4 matrix = glm::mat4();
	//set_value_m4 ( context->blocks[0].dataptr, &(newinstance->datagroup_store[matrix_id]), (u8*)&matrix, 0, 0, 0 );

	//create renderer from model and a list of rendercontexts
	//Renderer* simplerenderer = newinstance->create_renderer();

	//create world from a list of contextbases
	//World* simpleworld = newinstance->create_world();

	//RenderContext* simplecontext = simpleworld->add_rendercontext();
	//create contexts from world
	//add renderers to world
	//simpleworld->add_renderer ( simplerenderer );

	//loop
	//update contexts of world
	//update instances
	//let it render


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
	*window->cursor_mode() = CursorMode::eNormal;

	printf ( "Start Main Loop\n" );
	do {
		using namespace std::chrono_literals;
		std::this_thread::sleep_for ( 20ms );
		newinstance->process_events();
	} while ( newinstance->is_window_open() );
	WindowSection* section = window->root_section();
	window->root_section ( nullptr );

	window->destroy();
	if ( section )
		delete section;

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
