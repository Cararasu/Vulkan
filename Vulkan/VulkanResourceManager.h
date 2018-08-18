#ifndef VULKANRESOURCEMANAGER_H
#define VULKANRESOURCEMANAGER_H

#include <VulkanHeader.h>
#include "render/Model.h"
#include "render/ResourceManager.h"

struct VulkanInstance;
struct VulkanBuffer;
struct VulkanResourceManager;
struct VulkanModelBase;
struct VulkanModel;
struct VulkanContextBase;
struct VulkanTransferController;


struct VulkanInstance;

struct VulkanResourceManager : public ResourceManager {
	VulkanInstance * v_instance;

	VulkanTransferController* transfer_control;

	VulkanResourceManager ( VulkanInstance * instance );
	~VulkanResourceManager();

	vk::ShaderModule load_shader_from_file ( const char* filename );

	IdPtrArray<DataGroupDef> datagroup_store;
	IdPtrArray<ContextBase> contextbase_store;
	IdPtrArray<VulkanModelBase> modelbase_store;
	IdPtrArray<VulkanModel> model_store;
	
	virtual RId register_datagroupdef ( DataGroupDef& datagroupdef );
	virtual const DataGroupDef* datagroupdef ( RId handle );

	virtual RId register_contextbase ( RId datagroup/*image-defs*/ );
	virtual const ContextBase contextbase ( RId handle );

	virtual RId register_modelbase ( RId vertexdatagroup );
	virtual const ModelBase modelbase ( RId handle );

	virtual const Model load_generic_model ( RId modelbase_id, u8* vertices, u32 vertexcount, u16* indices, u32 indexcount );
	virtual const Model load_generic_model ( RId modelbase_id, u8* vertices, u32 vertexcount, u32* indices, u32 indexcount );

	virtual RId register_modelinstancebase ( Model model, RId context = 0 );
	virtual const ModelInstanceBase modelinstancebase ( RId handle );

	virtual const ModelInstance create_instance ( RId modelinstancebase );
};

#endif // VULKANRESOURCEMANAGER_H
