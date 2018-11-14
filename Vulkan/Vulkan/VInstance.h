#pragma once

#include "render/Instance.h"
#include "render/Window.h"
#include "VHeader.h"
#include <render/IdArray.h>

#define V_MAX_PGCQUEUE_COUNT (2)

void gatherExtLayer ( vk::PhysicalDevice device, std::vector<vk::LayerProperties>* layers, std::vector<vk::ExtensionProperties>* extensions );

struct VExtLayerStruct {
	std::vector<vk::LayerProperties> availableLayers;
	std::vector<vk::ExtensionProperties> availableExtensions;
	std::vector<String> neededLayers;
	std::vector<String> neededExtensions;

	bool activateLayer ( String name );
	bool activateExtension ( String name );
};

struct VDevice : public Device {
	vk::PhysicalDevice physical_device;
	
	VExtLayerStruct extLayers;
	std::vector<vk::QueueFamilyProperties> queueFamilyProps;
	vk::PhysicalDeviceProperties vkPhysDevProps;
	vk::PhysicalDeviceFeatures  vkPhysDevFeatures;

	virtual ~VDevice() {}
};

struct VInstance;

typedef std::function<void ( VInstance* instance, u64 frame_index ) > FrameFinishCallback;

struct VFrameCleanupStore {
	u64 frame_index;
	DynArray<FrameFinishCallback> finish_callbacks;
};

struct VMonitor : public Monitor {
	GLFWmonitor* monitor;

	VMonitor ( GLFWmonitor* );
	virtual ~VMonitor();

	virtual VideoMode current_mode();
};


struct VWindow;
struct VContextBase;
struct VModelBase;
struct VModel;
struct VModelInstanceBase;
struct VRenderStage;
struct VBuffer;
struct VInstanceGroup;
struct VResourceManager;

struct VSimpleTransferJob {
	VBuffer* source_buffer, *target_buffer;
	vk::BufferCopy sections;
};


struct VInstance : public Instance {
	InstanceOptions options;
	
	VExtLayerStruct extLayers;
	Map<GLFWmonitor*, VMonitor*> monitor_map;
	Map<GLFWwindow*, VWindow*> window_map;
	Set<VWindow*> windows;

	vk::Instance v_instance;
	VDevice* v_device;
	vk::Device m_device;
	vk::DebugReportCallbackEXT debugReportCallbackEXT;

	QueueWrapper queues;

	bool initialized = false;
	u64 frame_index = 1;

	VInstance();
	virtual ~VInstance();

	virtual bool initialize ( InstanceOptions options, Device* device = nullptr );

	virtual void process_events();
	virtual bool is_window_open();

	virtual void present_window ( Window* window );
	virtual void v_present_window ( VWindow* window );
	virtual void present_windows();

	virtual Window* create_window();
	virtual bool destroy_window ( Window* window );

	virtual Monitor* get_primary_monitor();

	VMonitor* get_primary_monitor_vulkan();
	
	VResourceManager* m_resource_manager = nullptr;
	virtual ResourceManager* resource_manager() override;
	
//------------ Resources

	IdPtrArray<DataGroupDef> datagroup_store;
	IdPtrArray<VContextBase> contextbase_store;
	IdPtrArray<VModelBase> modelbase_store;
	UIdPtrArray<VModel> model_store;
	IdPtrArray<VModelInstanceBase> modelinstancebase_store;
	IdPtrArray<VRenderStage> renderstage_store;
	DynArray<VInstanceGroup*> instancegroup_store;
	
	virtual const DataGroupDef* register_datagroupdef ( Array<DataValueDef> valuedefs, u32 size, u32 arraycount ) override;
	virtual const DataGroupDef* datagroupdef ( RId handle ) override;

	virtual const ContextBase* register_contextbase ( RId datagroup/*image-defs*/ ) override;
	virtual const ContextBase* register_contextbase ( const DataGroupDef* datagroup/*image-defs*/ ) override;
	virtual const ContextBase* contextbase ( RId handle ) override;

	virtual const ModelBase* register_modelbase ( RId vertexdatagroup ) override;
	virtual const ModelBase* register_modelbase ( const DataGroupDef* vertexdatagroup ) override;
	virtual const ModelBase* modelbase ( RId handle ) override;

	virtual const Model load_generic_model ( RId modelbase, void* vertices, u32 vertexcount, u16* indices, u32 indexcount ) override;
	virtual const Model load_generic_model ( const ModelBase* modelbase, void* vertices, u32 vertexcount, u16* indices, u32 indexcount ) override;

	virtual const Model load_generic_model ( RId modelbase, void* vertices, u32 vertexcount, u32* indices, u32 indexcount ) override;
	virtual const Model load_generic_model ( const ModelBase* modelbase, void* vertices, u32 vertexcount, u32* indices, u32 indexcount ) override;

	virtual const ModelInstanceBase* register_modelinstancebase ( Model model, RId datagroup = 0 ) override;
	virtual const ModelInstanceBase* register_modelinstancebase ( Model model, const DataGroupDef* datagroup = nullptr ) override;
	virtual const ModelInstanceBase* modelinstancebase ( RId handle ) override;

	virtual const RenderStage* create_renderstage ( 
	    const ModelInstanceBase* model_instance_base,
	    const Array<const ContextBase*> context_bases,
		StringReference vertex_shader,
		StringReference fragment_shader,
		StringReference geometry_shader,
		StringReference tess_cntrl_shader,
		StringReference tess_eval_shader,
		Array<RenderImageDef> input_image_defs ) override;
	virtual const RenderStage* renderstage ( RId handle ) override;

	virtual InstanceGroup* create_instancegroup() override;
	virtual ContextGroup* create_contextgroup() override;

	virtual RenderBundle* create_renderbundle ( InstanceGroup* igroup, ContextGroup* cgroup, Array<const RenderStage*>& rstages, Array<ImageType>& image_types, Array<ImageDependency>& dependencies );

	virtual void prepare_render ();
	virtual void prepare_render (Array<Window*> windows);
	
	virtual void render_bundles ( Array<RenderBundle*> bundles );


//------------ Specialized Functions
	virtual RenderBundle* create_main_bundle(InstanceGroup* igroup, ContextGroup* cgroup) override;


//------------ Transfer Data

	VBuffer* request_staging_buffer ( u64 size );
	void free_staging_buffer(VBuffer* buffer);

	//immediate transfer
	void transfer_data ( Array<VSimpleTransferJob>& jobs );
	//high priority/ per frame transfer
	vk::Semaphore schedule_transfer_data ( Array<VSimpleTransferJob>& jobs );
	//a fence for optional cpu-synchronization and cpu-side check for completion
	//maybe do it in a queue that has lower priority
	vk::Fence do_transfer_data_asynch ( Array<VSimpleTransferJob>& jobs );


//------------ Synchronization

	//wait until frame completion
	void wait_for_frame ( u64 frame_index );

	void schedule_on_frame_finish ( FrameFinishCallback callback );
	void schedule_on_current_frame_finish ( FrameFinishCallback callback );
	

//------------ Memory allocation

	u32 find_memory_type ( u32 typeFilter, vk::MemoryPropertyFlags properties );

	vk::DeviceMemory allocate_memory ( vk::MemoryRequirements memoryRequirement, vk::MemoryPropertyFlags properties );
	void allocate_gpu_memory ( vk::MemoryRequirements mem_req, GPUMemory* memory );

	RendResult free_gpu_memory ( GPUMemory memory );

//------------ Images

	vk::ImageView createImageView2D ( vk::Image image, u32 mipBase, u32 mipOffset, vk::Format format, vk::ImageAspectFlags aspectFlags );
	vk::ImageView createImageView2DArray ( vk::Image image, u32 mipBase, u32 mipOffset, u32 arrayOffset, u32 arraySize, vk::Format format, vk::ImageAspectFlags aspectFlags );
	void destroyImageView ( vk::ImageView imageview );

	vk::Format find_supported_image_format ( const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features );
	vk::Format find_depth_image_format();

//------------ Instance/Device/Queues

	inline QueueWrapper* queue_wrapper () {
		return &queues;
	}
	inline vk::Device vk_device () {
		return m_device;
	}
	inline vk::PhysicalDevice vk_physical_device () {
		return v_device->physical_device;
	}
	
//------------ Vulkan Resources

//------------------------ ResourceCaching

//------------------------ Semaphores

	inline vk::Semaphore create_semaphore () {
		//TODO caching
		return m_device.createSemaphore ( vk::SemaphoreCreateInfo(), nullptr );
	}
	inline RendResult destroy_semaphore ( vk::Semaphore sem ) {
		m_device.destroySemaphore ( sem, nullptr );
		return RendResult::eSuccess;
	}
	
//------------------------ CommandPools/CommandBuffers

	vk::CommandPool createTransferCommandPool ( vk::CommandPoolCreateFlags createFlags = vk::CommandPoolCreateFlags() );
	vk::CommandPool createGraphicsCommandPool ( vk::CommandPoolCreateFlags createFlags = vk::CommandPoolCreateFlags() );
	void destroyCommandPool ( vk::CommandPool commandPool );
	
	vk::CommandBuffer createCommandBuffer ( vk::CommandPool commandPool, vk::CommandBufferLevel bufferLevel );
	void delete_command_buffer ( vk::CommandPool commandPool, vk::CommandBuffer commandBuffer );
	
};


inline RendResult wrap_command ( std::function<RendResult ( vk::CommandBuffer ) > do_command, VInstance* v_instance, vk::CommandPool commandpool, vk::CommandBuffer* cmdbuffer ) {
	*cmdbuffer = v_instance->createCommandBuffer ( commandpool, vk::CommandBufferLevel::ePrimary );
	cmdbuffer->begin ( vk::CommandBufferBeginInfo ( vk::CommandBufferUsageFlagBits::eOneTimeSubmit ) );
	RendResult res = do_command ( *cmdbuffer );
	cmdbuffer->end();
	return res;
}
