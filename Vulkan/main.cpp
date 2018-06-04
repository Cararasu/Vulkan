#include <stdio.h>

#include <chrono>
#include <thread>
#include <algorithm>
#include <vector>
#include <fstream>
#include "VGlobal.h"
#include "VHeader.h"
#include "VWindow.h"
#include "ViewPort.h"
#include "DataWrapper.h"
#include "RenderEnvironment.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#include <fstream>

#include <render/Window.h>

template<typename T>
struct ProxyObject{
	T obj;
	T* proxyObj = nullptr;
	
	inline void update(){
		if(proxyObj) memcpy(&obj, proxyObj, sizeof(T));
		printf("Proxy_update\n");
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

#include "DrawDispatcher.h"

struct DefDrawData{
	glm::dvec3 pos, rot, scale;
};
struct CullingStrategy {
//CameraFlagList
//QuadTree(Box(Index of Obj))

	//virtual void update(ObjectStore<DrawObject>* objectStore) = 0;
	
	virtual void addToStrat(DefDrawData& obj) = 0;
	virtual void removeFromStrat(DefDrawData& obj) = 0;
};
struct RenderContext {
	//ObjectStore<DrawObject, uint32_t> objStore;
	CullingStrategy* strategy;
	
	template<typename OBJDATA>
	uint64_t createObject();
	template<typename OBJDATA>
	void removeObject(uint64_t id);
};


uint32_t loadDataFile (std::string file, OpaqueObjectDispatcher* dispatcher) {

	std::ifstream input (file, std::ios::binary);

	std::string str;
	std::getline (input, str, '\0');
	uint32_t vertexCount = 0;
	input.read (reinterpret_cast<char*> (&vertexCount), sizeof (uint32_t));

	std::vector<Vertex> vs;
	vs.resize (vertexCount);
	for (size_t i = 0; i < vertexCount; i++) {
		Vertex v;
		input.read (reinterpret_cast<char*> (&v.pos[0]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.uv[0]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.normal[0]), sizeof (float));

		input.read (reinterpret_cast<char*> (&v.pos[1]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.uv[1]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.normal[1]), sizeof (float));

		input.read (reinterpret_cast<char*> (&v.pos[2]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.uv[2]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.normal[2]), sizeof (float));

		vs[i] = v;
	}
	uint32_t indexCount;
	input.read (reinterpret_cast<char*> (&indexCount), sizeof (uint32_t));

	std::vector<uint32_t> is;
	is.resize (indexCount);

	input.read (reinterpret_cast<char*> (is.data()), sizeof (uint32_t) *indexCount);
	
	ObjectPartData data;
	data.diffuseTexId = 0;
	return g_thread_data.dispatcher.add_object(vs, is, data);
}

LoadedObjectData<Vertex, uint32_t> loadDataFile (std::string file, ObjectVertexData<Vertex, uint32_t>& vertex_data) {

	std::ifstream input (file, std::ios::binary);

	std::string str;
	std::getline (input, str, '\0');
	uint32_t vertexCount = 0;
	LoadedObjectData<Vertex, uint32_t> objData;
	objData.vertex_offset = vertex_data.vertices.size();
	objData.index_offset = vertex_data.indices.size();
	input.read (reinterpret_cast<char*> (&vertexCount), sizeof (uint32_t));
	

	vertex_data.vertices.resize (vertex_data.vertices.size() + vertexCount);
	for (size_t i = 0; i < vertexCount; i++) {
		Vertex v;
		input.read (reinterpret_cast<char*> (&v.pos[0]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.uv[0]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.normal[0]), sizeof (float));

		input.read (reinterpret_cast<char*> (&v.pos[1]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.uv[1]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.normal[1]), sizeof (float));

		input.read (reinterpret_cast<char*> (&v.pos[2]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.uv[2]), sizeof (float));
		input.read (reinterpret_cast<char*> (&v.normal[2]), sizeof (float));

		vertex_data.vertices[i] = v;
	}
	uint32_t indexCount;
	input.read (reinterpret_cast<char*> (&indexCount), sizeof (uint32_t));
	objData.index_size = indexCount;
	std::vector<uint32_t> is;
	vertex_data.indices.resize (vertex_data.indices.size() + indexCount);

	input.read (reinterpret_cast<char*> (&vertex_data.indices[objData.index_offset]), sizeof (uint32_t) *indexCount);
	return objData;
}


void loadImage (VInstance* instance, std::string file, ImageWrapper * imageWrapper, uint32_t index, vk::CommandPool commandPool, vk::Queue queue) {
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load (file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error ("failed to load texture image!");
	}
	vk::Extent3D imageExtent = vk::Extent3D (texWidth, texHeight, 1);
	instance->transferData (pixels, imageWrapper->image, vk::Offset3D (0, 0, 0), imageExtent, index, imageSize,
	              vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, commandPool, queue);

	stbi_image_free (pixels);
}

struct Orientation{
	glm::dvec3 position, rotation, scale;
};
struct BoundingBox{
	glm::dvec3 pos_size, neg_size;
};
struct SimpleObject{
	Orientation orientation;
	BoundingBox bounding_box;
};

#define MAX_PART_IDS (8)

struct WWW{
	ProxyObject<SimpleObject> proxy_obj;
	uint32_t objectId;
	size_t partcount;
	std::array<uint32_t, MAX_PART_IDS> parts;
	void update(ThreadRenderEnvironment* renderEnv){//gets called after ProxyObject update in the update Phase
		proxy_obj.update();
		printf("Update Obj pos(%f,%f,%f)\n", proxy_obj.obj.orientation.position.x, proxy_obj.obj.orientation.position.y, proxy_obj.obj.orientation.position.z);
	}
	void execute(ThreadRenderEnvironment* renderEnv){//gets called during the execute Phase
		Instance instance;
		instance.m2wMatrix = glm::translate(proxy_obj.obj.orientation.position);
		printf("Dispatch Obj pos(%f,%f,%f)\n", proxy_obj.obj.orientation.position.x, proxy_obj.obj.orientation.position.y, proxy_obj.obj.orientation.position.z);
		for(size_t i = 0; i < partcount; i++){
			renderEnv->dispatcher.push_instance(parts[i], instance);
			printf("%d, ", parts[i]);
		}
		printf("\n");
	}
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

int main (int argc, char **argv) {
	
	std::vector<uint32_t> TiePartIds;
	TiePartIds.push_back(loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Body.data", &g_thread_data.dispatcher));
	g_thread_data.dispatcher.parts.back().data.diffuseTexId = 0;
	TiePartIds.push_back(loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Arm_L.data", &g_thread_data.dispatcher));
	g_thread_data.dispatcher.parts.back().data.diffuseTexId = 1;
	TiePartIds.push_back(loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Arm_R.data", &g_thread_data.dispatcher));
	g_thread_data.dispatcher.parts.back().data.diffuseTexId = 1;
	TiePartIds.push_back(loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Wing_L.data", &g_thread_data.dispatcher));
	g_thread_data.dispatcher.parts.back().data.diffuseTexId = 2;
	TiePartIds.push_back(loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Wing_R.data", &g_thread_data.dispatcher));
	g_thread_data.dispatcher.parts.back().data.diffuseTexId = 2;
	TiePartIds.push_back(loadDataFile ("../workingdir/assets/Tie_Fighter_Windows.data", &g_thread_data.dispatcher));
	g_thread_data.dispatcher.parts.back().data.diffuseTexId = 2;
	
	std::vector<uint32_t> XPartIds;
	XPartIds.push_back(loadDataFile ("../workingdir/assets/X/XWing_Body.data", &g_thread_data.dispatcher));
	g_thread_data.dispatcher.parts.back().data.diffuseTexId = 3;
	XPartIds.push_back(loadDataFile ("../workingdir/assets/X/XWing_Windows.data", &g_thread_data.dispatcher));
	g_thread_data.dispatcher.parts.back().data.diffuseTexId = 3;
	XPartIds.push_back(loadDataFile ("../workingdir/assets/X/XWing_Wing_LB.data", &g_thread_data.dispatcher));
	g_thread_data.dispatcher.parts.back().data.diffuseTexId = 3;
	XPartIds.push_back(loadDataFile ("../workingdir/assets/X/XWing_Wing_LT.data", &g_thread_data.dispatcher));
	g_thread_data.dispatcher.parts.back().data.diffuseTexId = 3;
	XPartIds.push_back(loadDataFile ("../workingdir/assets/X/XWing_Wing_RB.data", &g_thread_data.dispatcher));
	g_thread_data.dispatcher.parts.back().data.diffuseTexId = 3;
	XPartIds.push_back(loadDataFile ("../workingdir/assets/X/XWing_Wing_RT.data", &g_thread_data.dispatcher));
	g_thread_data.dispatcher.parts.back().data.diffuseTexId = 3;
	
	

	PerspectiveViewPort<float> viewport;

	viewport.m_viewvector = {0.0f, 0.2f, -1.4f};
	viewport.m_focalpoint = {0.0f, 0.0f, 0.0f};
	viewport.m_upvector = {0.0f, 1.0f, 0.0f};
	viewport.m_distance = 2.0f;
	viewport.m_near = 0.01f;
	viewport.m_far = 100.0f;
	viewport.m_aspect = 1.0f;

	printf ("hello world\n");

	glfwInit();

	global.preInitialize();

	printf ("Instance Extensions:\n");
	for (vk::ExtensionProperties& extProp : global.extLayers.availableExtensions) {
		printf ("\t%s\n", extProp.extensionName);
	}
	printf ("Instance Layers:\n");
	for (vk::LayerProperties& layerProp : global.extLayers.availableLayers) {
		printf ("\t%s\n", layerProp.layerName);
		printf ("\t\tDesc: %s\n", layerProp.description);
	}

	if (!global.extLayers.activateLayer ("VK_LAYER_LUNARG_standard_validation")) {
		printf ("Layer VK_LAYER_LUNARG_standard_validation not available\n");
	}
	if (!global.extLayers.activateLayer ("VK_LAYER_LUNARG_swapchain")) {
		printf ("Layer VK_LAYER_LUNARG_swapchain not available\n");
	}

	/*if(!global.instExtLayers.activateLayer("VK_LAYER_RENDERDOC_Capture")){
		printf("Extension VK_LAYER_RENDERDOC_Capture not available\n");
	}
	if(!global.instExtLayers.activateLayer("VK_LAYER_LUNARG_api_dump")){
		printf("Extension VK_LAYER_LUNARG_api_dump not available\n");
	}*/
	uint32_t instanceExtCount;
	const char** glfwReqInstanceExt = glfwGetRequiredInstanceExtensions (&instanceExtCount);
	for (size_t i = 0; i < instanceExtCount; i++) {
		if (!global.extLayers.activateExtension (glfwReqInstanceExt[i])) {
			printf ("Extension %s not available\n", glfwReqInstanceExt[i]);
		}else{
			printf ("Activate Extension %s\n", glfwReqInstanceExt[i]);
		}
	}
	if (!global.extLayers.activateExtension (VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
		printf ("Extension %s not available\n", VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	printf ("Instance Extensions available:\n");
	for (vk::ExtensionProperties& prop : global.extLayers.availableExtensions) {
		printf ("\t%s\n", prop.extensionName);
	}
	printf ("Instance Layers available:\n");
	for (vk::LayerProperties& prop : global.extLayers.availableLayers) {
		printf ("\t%s\n", prop.layerName);
	}

	global.initializeInstance ("Blabla", "WUWU Engine");

	
	{
		LoadedObjectWrapper<Vertex, uint32_t> obj_wrap;
		obj_wrap.object_def.push_back(loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Body.data", obj_wrap.vertex_data));
		obj_wrap.object_def.back().diffuseTexId = 0;
		obj_wrap.object_def.push_back(loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Arm_L.data", obj_wrap.vertex_data));
		obj_wrap.object_def.back().diffuseTexId = 1;
		obj_wrap.object_def.push_back(loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Arm_R.data", obj_wrap.vertex_data));
		obj_wrap.object_def.back().diffuseTexId = 1;
		obj_wrap.object_def.push_back(loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Wing_L.data", obj_wrap.vertex_data));
		obj_wrap.object_def.back().diffuseTexId = 2;
		obj_wrap.object_def.push_back(loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Wing_R.data", obj_wrap.vertex_data));
		obj_wrap.object_def.back().diffuseTexId = 2;
		obj_wrap.object_def.push_back(loadDataFile ("../workingdir/assets/Tie/Tie_Fighter_Windows.data", obj_wrap.vertex_data));
		obj_wrap.object_def.back().diffuseTexId = 2;
		
		obj_wrap.object_def.push_back(loadDataFile ("../workingdir/assets/X/XWing_Body.data", obj_wrap.vertex_data));
		obj_wrap.object_def.back().diffuseTexId = 3;
		obj_wrap.object_def.push_back(loadDataFile ("../workingdir/assets/X/XWing_Windows.data", obj_wrap.vertex_data));
		obj_wrap.object_def.back().diffuseTexId = 3;
		obj_wrap.object_def.push_back(loadDataFile ("../workingdir/assets/X/XWing_Wing_LB.data", obj_wrap.vertex_data));
		obj_wrap.object_def.back().diffuseTexId = 3;
		obj_wrap.object_def.push_back(loadDataFile ("../workingdir/assets/X/XWing_Wing_LT.data", obj_wrap.vertex_data));
		obj_wrap.object_def.back().diffuseTexId = 3;
		obj_wrap.object_def.push_back(loadDataFile ("../workingdir/assets/X/XWing_Wing_RB.data", obj_wrap.vertex_data));
		obj_wrap.object_def.back().diffuseTexId = 3;
		obj_wrap.object_def.push_back(loadDataFile ("../workingdir/assets/X/XWing_Wing_RT.data", obj_wrap.vertex_data));
		obj_wrap.object_def.back().diffuseTexId = 3;

		g_render_environment.objects.push_back(obj_wrap);
	}
	//global.devExtLayers.activateExtension(VK_NV_GLSL_SHADER_EXTENSION_NAME);
	VInstance* instance = global.createInstance();
	printf ("Device Extensions available:\n");
	for (vk::ExtensionProperties& prop : instance->extLayers.availableExtensions) {
		printf ("\t%s\n", prop.extensionName);
	}
	printf ("Device Layers available:\n");
	for (vk::LayerProperties& prop : instance->extLayers.availableLayers) {
		printf ("\t%s\n", prop.layerName);
	}

	if (!instance->extLayers.activateExtension (VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
		printf ("Extension %s not available\n", VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}
	if (!instance->extLayers.activateExtension (VK_NV_GLSL_SHADER_EXTENSION_NAME)) {
		printf ("Extension %s not available\n", VK_NV_GLSL_SHADER_EXTENSION_NAME);
	}
	global.initializeDevice(instance);
	instance->pipeline_module_builders.standard.instance = instance;
	g_thread_data.init(&g_render_environment, instance);

	VWindow* vWindow = new VWindow();

	instance->pipeline_module_layouts.standard = createPipelineModuleLayout(&instance->pipeline_module_builders.standard);

	vWindow->initializeWindow(instance);


	vk::DescriptorPool descriptorSetPool = createStandardDescriptorSetPool(instance);

	std::vector<vk::DescriptorSet> descriptorSets = instance->createDescriptorSets (descriptorSetPool, &instance->pipeline_module_layouts.standard.descriptorSetLayouts);

	g_thread_data.dispatcher.set_descriptor_set(descriptorSets[1]);

	vk::CommandPool transferCommandPool = instance->createTransferCommandPool (vk::CommandPoolCreateFlagBits::eTransient);

	g_thread_data.dispatcher.upload_data(transferCommandPool, instance->tqueue->transferQueue);

	VkExtent3D imageExtent = {4096, 4096, 1};
	vk::Format imageFormat = instance->findSupportedFormat ({vk::Format::eR8G8B8A8Unorm}, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eSampledImage);
	ImageWrapper* imageWrapper = new ImageWrapper (instance, imageExtent, 12, 4, imageFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlags (vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc),
	        vk::ImageAspectFlagBits::eColor, vk::MemoryPropertyFlags (vk::MemoryPropertyFlagBits::eDeviceLocal));
	{
		imageWrapper->transitionImageLayout (vk::ImageLayout::eTransferDstOptimal, vWindow->getCurrentGraphicsCommandPool(), vWindow->pgcQueue->graphicsQueue);

		loadImage (instance, "../workingdir/assets/Tie/Tie_Fighter_Body_Diffuse.png", imageWrapper, 0, transferCommandPool, instance->tqueue->transferQueue);
		loadImage (instance, "../workingdir/assets/Tie/Tie_Fighter_Arm_Diffuse.png", imageWrapper, 1, transferCommandPool, instance->tqueue->transferQueue);
		loadImage (instance, "../workingdir/assets/Tie/Tie_Fighter_Wing_Diffuse.png", imageWrapper, 2, transferCommandPool, instance->tqueue->transferQueue);
		loadImage (instance, "../workingdir/assets/X/XWing_Diffuse.png", imageWrapper, 3, transferCommandPool, instance->tqueue->transferQueue);
		
		imageWrapper->generateMipmaps(0, vk::ImageLayout::eShaderReadOnlyOptimal, vWindow->getCurrentGraphicsCommandPool(), vWindow->pgcQueue->graphicsQueue);
	}

	//TODO Memory Barrier for graphics queue
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

		instance->device.createSampler (&samplerInfo, nullptr, &sampler);
	}

	g_thread_data.dispatcher.set_image_array(imageWrapper, sampler);

	uint32_t MAX_COMMAND_COUNT = 100;

	BufferWrapper *uniformBuffer = new BufferWrapper (instance, sizeof (Camera),
	        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal);


	vk::DescriptorBufferInfo bufferInfo (uniformBuffer->buffer, offsetof (Camera, w2sMatrix), sizeof (glm::mat4));
	instance->device.updateDescriptorSets ({
		vk::WriteDescriptorSet (descriptorSets[0],
								0, 0, //dstBinding, dstArrayElement
								1, //descriptorCount
								vk::DescriptorType::eUniformBuffer, //descriptorType
								nullptr, &bufferInfo, nullptr), //pImageInfo, pBufferInfo, pTexelBufferView
	}, {});

	vk::Semaphore drawFinishedSemaphore = instance->createSemaphore();

	ObjectStore<WWW> store;
	SimpleObject obj1;
	SimpleObject obj2;
	SimpleObject obj3;
	WWW ww1;
	WWW ww2;
	WWW ww3;
	ww1.proxy_obj.proxyObj = &obj1;
	ww1.partcount = XPartIds.size();
	for(size_t i = 0; i < XPartIds.size(); i++)
		ww1.parts[i] = XPartIds[i];
	
	ww2.proxy_obj.proxyObj = &obj2;
	ww2.partcount = TiePartIds.size();
	for(size_t i = 0; i < TiePartIds.size(); i++)
		ww2.parts[i] = TiePartIds[i];
	
	ww3.proxy_obj.proxyObj = &obj3;
	ww3.partcount = TiePartIds.size();
	for(size_t i = 0; i < TiePartIds.size(); i++)
		ww3.parts[i] = TiePartIds[i];
	
	obj1.orientation.position = glm::vec3(0.0f, 0.0f, 0.0f);
	obj2.orientation.position = glm::vec3(1.0f, 1.0f, 1.0f);
	obj3.orientation.position = glm::vec3(-3.0f, 0.0f, 0.0f);
	
	ObjId id1 = store.insert(ww1);
	printf("Id: %d Index: %d\n", id1.id, id1.index);
	ObjId id2 = store.insert(ww2);
	printf("Id: %d Index: %d\n", id2.id, id2.index);
	ObjId id3 = store.insert(ww3);
	printf("Id: %d Index: %d\n", id3.id, id3.index);
	
	
	while (vWindow->isOpen()) {
		
		obj1.orientation.position.x += 0.01f;
		vWindow->setupFrame();
		printf ("--------------- FrameBoundary ---------------\n");
		printf ("PresetImageId: %d\n", vWindow->presentImageIndex);

		viewport.m_viewvector = glm::rotate (viewport.m_viewvector, 0.005f, viewport.m_upvector);
		
		g_thread_data.reset();
		//do stuff serialized
		//to do this threaded we need to extract the update and execute into another class or scope maybe into ThreadRenderEnvironment itself
		store.update(&g_thread_data);
		store.execute(&g_thread_data);
		
		//this should be per thread/per frame
		vk::CommandBuffer commandBuffer = instance->createCommandBuffer (vWindow->getCurrentGraphicsCommandPool(), vk::CommandBufferLevel::ePrimary);
		vk::CommandBufferBeginInfo beginInfo (vk::CommandBufferUsageFlags (vk::CommandBufferUsageFlagBits::eOneTimeSubmit), nullptr);
		
		{
			commandBuffer.begin (&beginInfo);
			
			
			//if staginbuffer not big enough to store all global values
			//	-> error
			//if staginbuffer big enough to store all global values and not big enough to hold instancedata
			
			uint32_t stagingOffset = 0;

			uint32_t uniformOffset = stagingOffset;
			( (Camera*) (stagingBuffer->data + stagingOffset)) [0].w2sMatrix = viewport.createWorldToScreenSpaceMatrix();
			stagingOffset += sizeof (Camera);

			copyBuffer (stagingBuffer, uniformBuffer, uniformOffset, 0, sizeof (Camera),
						vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite,
						commandBuffer);
			
			stagingOffset = g_thread_data.dispatcher.setup(stagingBuffer, stagingOffset, commandBuffer);
			
			commandBuffer.pipelineBarrier (
				vk::PipelineStageFlags (vk::PipelineStageFlagBits::eTransfer), vk::PipelineStageFlags (vk::PipelineStageFlagBits::eVertexInput),
				vk::DependencyFlags(),
			{}/*memoryBarrier*/, {
				vk::BufferMemoryBarrier (
					vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eVertexAttributeRead,
					instance->transfQId, instance->graphQId,
					g_thread_data.dispatcher.instanceBuffer->buffer, 0, g_thread_data.dispatcher.instanceBuffer->memory.size
				)
			}/*bufferBarrier*/, {}/*imageBarrier*/);
			commandBuffer.pipelineBarrier (
			    vk::PipelineStageFlags (vk::PipelineStageFlagBits::eTransfer), vk::PipelineStageFlags (vk::PipelineStageFlagBits::eVertexShader),
			    vk::DependencyFlags(),
			{}/*memoryBarrier*/, {
				vk::BufferMemoryBarrier (
				    vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eUniformRead,
				    instance->transfQId, instance->graphQId,
				    uniformBuffer->buffer, 0, uniformBuffer->memory.size
				)
			}/*bufferBarrier*/, {}/*imageBarrier*/);
			vk::ClearValue clearColors[2] = {
				vk::ClearValue (vk::ClearColorValue (std::array<float, 4> ({0.0f, 0.0f, 0.5f, 1.0f}))),
				vk::ClearValue (vk::ClearDepthStencilValue (1.0f, 0)),
			};
			commandBuffer.beginRenderPass (
			    vk::RenderPassBeginInfo (
					vWindow->standardmodule.renderPass,
			        vWindow->perPresentImageDatas[vWindow->presentImageIndex].framebuffer,
			        vk::Rect2D (vk::Offset2D (0, 0), vWindow->swapChainExtend),
			        2, clearColors
			    ),
			    vk::SubpassContents::eInline
			);
			
			commandBuffer.bindPipeline (vk::PipelineBindPoint::eGraphics, vWindow->standardmodule.pipeline);
			commandBuffer.bindDescriptorSets (vk::PipelineBindPoint::eGraphics, instance->pipeline_module_layouts.standard.pipelineLayout, 0, descriptorSets[0], {});
			
			g_thread_data.dispatcher.dispatch(commandBuffer);

			commandBuffer.endRenderPass();

			V_CHECKCALL_MAYBE (commandBuffer.end(), printf ("Recording of CommandBuffer failed\n"));
			
		}


		vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

		vk::Semaphore waitSemaphores[] = {vWindow->imageAvailableGuardSem};
		vk::Semaphore signalSemaphores[] = {drawFinishedSemaphore};
		vk::SubmitInfo submitInfo;
		if(vWindow->windowState != WindowState::eNotVisible){//otherwise we wait for a guard semaphore, for acquiring the present image
			submitInfo = vk::SubmitInfo (1, waitSemaphores, waitStages, 1, &commandBuffer, 1, signalSemaphores);

			vWindow->pgcQueue->submitGraphics (1, &submitInfo);
			
			vWindow->showNextFrame (1, signalSemaphores);
		}
		
		printf ("---------------   EndFrame    ---------------\n");
		glfwPollEvents();

		std::this_thread::sleep_for (std::chrono::nanoseconds (10000000));
	}

	instance->device.waitIdle();

	V_CHECKCALL_MAYBE (vWindow->pgcQueue->presentQueue.waitIdle(), printf ("Failed to wait for Present-Queue\n"));
	V_CHECKCALL_MAYBE (instance->device.waitIdle(), printf ("Failed to wait for Device\n"));

	instance->device.destroyCommandPool (transferCommandPool);

	instance->destroySemaphore (drawFinishedSemaphore);

	instance->device.destroySampler(sampler);

	g_thread_data.finit(instance);
	
	delete vWindow;
	if (stagingBuffer)
		delete stagingBuffer;

	delete imageWrapper;
	
	delete uniformBuffer;

	instance->device.destroyDescriptorPool (descriptorSetPool, nullptr);

	glfwTerminate();
	global.terminate();

	return 0;
}
