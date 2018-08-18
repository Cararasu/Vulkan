
#include "VulkanResourceManager.h"
#include "VulkanTransferOperator.h"
#include "VulkanInstance.h"
#include <fstream>
#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "VulkanModel.h"


VulkanResourceManager::VulkanResourceManager ( VulkanInstance * instance ) : v_instance ( instance ), transfer_control (new VulkanTransferController(instance)) {
	
}

static std::vector<char> readFile ( const char* filename ) {
	std::ifstream file ( filename, std::ios::ate | std::ios::binary );

	if ( !file.is_open() ) {
		printf ( "Couldn't open File %s\n", filename );
		return std::vector<char>();
	}

	size_t fileSize = ( size_t ) file.tellg();
	std::vector<char> buffer ( fileSize );

	file.seekg ( 0 );
	file.read ( buffer.data(), fileSize );
	file.close();

	return buffer;
}
VulkanResourceManager::~VulkanResourceManager() {
	for(auto ele : datagroup_store) {
		delete ele;
	}
	for(auto ele : contextbase_store) {
		delete ele;
	}
	for(auto ele : modelbase_store) {
		delete ele;
	}
	for(auto ele : model_store) {
		delete ele;
	}
	delete transfer_control;
}
RId VulkanResourceManager::register_datagroupdef ( DataGroupDef& datagroupdef ) {
	return datagroup_store.insert ( new DataGroupDef(datagroupdef) );
}
const DataGroupDef* VulkanResourceManager::datagroupdef ( RId id ) {
	return datagroup_store[id];
}

RId VulkanResourceManager::register_contextbase ( RId datagroup_id ) {
	ContextBase* contextbase = new ContextBase ( );
	contextbase->datagroups = datagroup_id;
	return contextbase_store.insert ( contextbase );
}
const ContextBase VulkanResourceManager::contextbase ( RId id ) {
	const ContextBase* v_contextbase = contextbase_store.get(id);
	return *v_contextbase;
}

RId VulkanResourceManager::register_modelbase ( RId vertexdatagroup ) {
	return modelbase_store.insert ( new VulkanModelBase(vertexdatagroup) );
}
const ModelBase VulkanResourceManager::modelbase ( RId id ) {
	const VulkanModelBase* v_modelbase = modelbase_store[id];
	return {v_modelbase->id, v_modelbase->datagroup};
}

RId register_modelinstancebase ( RId model, RId context ) {

	return 0;
}
const ModelInstanceBase modelinstancebase ( RId handle ) {

	return {0};
}

vk::ShaderModule VulkanResourceManager::load_shader_from_file ( const char* filename ) {

	std::vector<char> shaderCode = readFile ( filename );

	vk::ShaderModuleCreateInfo createInfo ( vk::ShaderModuleCreateFlags(), shaderCode.size(), ( const u32* ) shaderCode.data() );

	vk::ShaderModule shadermodule;
	V_CHECKCALL ( vulkan_device ( v_instance ).createShaderModule ( &createInfo, nullptr, &shadermodule ), printf ( "Creation of Shadermodule failed\n" ) );
	return shadermodule;
}
const Model VulkanResourceManager::load_generic_model ( RId modelbase, u8* vertices, u32 vertexcount, u16* indices, u32 indexcount ) {
	const VulkanModelBase* v_modelbase = modelbase_store[modelbase];
	const DataGroupDef* datagroupdef = datagroup_store[v_modelbase->datagroup];

	VulkanModel* newmodel = new VulkanModel(v_instance, modelbase);
	newmodel->modelbase = modelbase;
	newmodel->index_is_2byte = true;
	newmodel->vertexoffset = 0;
	newmodel->vertexcount = vertexcount;
	newmodel->indexoffset = 0;
	newmodel->indexcount = indexcount;
	newmodel->vertexbuffer.init(vertexcount * datagroupdef->size,
	        vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
	        vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal);
	newmodel->indexbuffer.init( indexcount * sizeof ( u16 ),
	        vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
	        vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );

	this->transfer_control->transfer_data ( vertices, vertexcount * datagroupdef->size, &newmodel->vertexbuffer, 0 );
	this->transfer_control->transfer_data ( ( u8* ) indices, indexcount * sizeof ( u16 ), &newmodel->indexbuffer, 0 );
	return {model_store.insert ( newmodel ), modelbase};
}
const Model VulkanResourceManager::load_generic_model ( RId modelbase, u8* vertices, u32 vertexcount, u32* indices, u32 indexcount ) {
	const VulkanModelBase* v_modelbase = modelbase_store[modelbase];
	const DataGroupDef* datagroupdef = datagroup_store[v_modelbase->datagroup];
	bool fitsin2bytes = vertexcount < 0x10000;

	VulkanModel* newmodel = new VulkanModel(v_instance, modelbase);
	newmodel->modelbase = modelbase;
	newmodel->index_is_2byte = false;
	newmodel->vertexoffset = 0;
	newmodel->vertexcount = vertexcount;
	newmodel->indexoffset = 0;
	newmodel->indexcount = indexcount;
	newmodel->vertexbuffer.init(vertexcount * datagroupdef->size,
	        vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
	        vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal);
	newmodel->indexbuffer.init( indexcount * sizeof ( u32 ),
	        vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
	        vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal );

	this->transfer_control->transfer_data ( vertices, vertexcount * datagroupdef->size, &newmodel->vertexbuffer, 0 );
	this->transfer_control->transfer_data ( ( u8* ) indices, indexcount * sizeof ( u32 ), &newmodel->indexbuffer, 0 );
	return {model_store.insert ( newmodel ), modelbase};
}
RId VulkanResourceManager::register_modelinstancebase ( Model model, RId context ) {
	const VulkanModel* v_model = model_store[model.id];
	const VulkanModelBase* v_modelbase = modelbase_store[v_model->modelbase];
	const ContextBase* v_contextbase = contextbase_store[context];
	
}
const ModelInstanceBase VulkanResourceManager::modelinstancebase ( RId handle ) {

}
const ModelInstance VulkanResourceManager::create_instance ( RId modelinstancebase ) {

}
