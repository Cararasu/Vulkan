#pragma once

#include "render/Instance.h"
#include "render/Window.h"
#include "VHeader.h"
#include <render/IdArray.h>
#include "VContext.h"
#include "VMemoryAllocator.h"

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
struct VModel;
struct VContext;
struct VRenderStage;
struct VBuffer;
struct VInstanceGroup;
struct VResourceManager;

struct VSimpleTransferJob {
	VThinBuffer source_buffer;
	VBuffer *target_buffer;
	vk::BufferCopy sections;
};

struct PerFrameData  {
	u64 frame_index;
	DynArray<VDividableBufferStore*> staging_buffer_stores;
	DynArray<vk::Fence> fences;
	DynArray<vk::CommandBuffer> command_buffers;
	AlwaysGrowAllocator frame_allocator;
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
	u64 last_completed_frame_index = 0;
	
	AlwaysGrowAllocator local_allocator;
	PerFrameData free_data;
	PerFrameData* current_free_data = nullptr;
	std::queue<PerFrameData*> per_frame_queue;
	DynArray<PerFrameData*> per_frame_datas;

	VInstance();
	virtual ~VInstance();

	virtual bool initialize ( InstanceOptions options, Device* device = nullptr ) override;

	virtual void process_events() override;
	virtual void process_events(double timeout) override;
	virtual bool is_window_open() override;

	virtual Window* create_window() override;
	virtual bool destroy_window ( Window* window ) override;

	virtual Monitor* get_primary_monitor() override;

	VMonitor* get_primary_monitor_vulkan();

	VResourceManager* m_resource_manager = nullptr;
	virtual ResourceManager* resource_manager() override;

//------------ Resources

	virtual void set_context ( Model& model, Context& context ) override;

	virtual void load_generic_model ( Model& model, void* vertices, u32 vertexcount, u16* indices, u32 indexcount ) override;
	virtual void load_generic_model ( Model& model, void* vertices, u32 vertexcount, u32* indices, u32 indexcount ) override;

	virtual void unload_model ( ModelId modelbase_id ) override;

	virtual void update_context_data ( Context& context, void* data ) override;
	virtual void update_context_image_sampler ( Context& context, u32 index, u32 array_index, ImageUseRef imageuse, Sampler* sampler ) override;

	Map<ContextBaseId, VContextBase> contextbase_map;
	virtual void contextbase_registered ( ContextBaseId id ) override;
	virtual void modelbase_registered ( ModelBaseId id ) override;
	virtual void instancebase_registered ( InstanceBaseId id ) override;

	VUpdateableBufferStorage* context_bufferstorage = nullptr;
	Map<ContextBaseId, IdPtrArray<VContext>> v_context_map;
	u64 last_contexts_updated_frame_index = 0;
	virtual Context create_context ( ContextBaseId id ) override;

	Map<ModelBaseId, IdPtrArray<VModel>> v_model_map;
	virtual Model create_model ( ModelBaseId id ) override;

	IdPtrArray<VRenderStage> renderstage_store;
	DynArray<VInstanceGroup*> instancegroup_store;

	virtual InstanceGroup* create_instancegroup() override;
	virtual ContextGroup* create_contextgroup() override;

	virtual RenderBundle* create_renderbundle ( InstanceGroup* igroup, ContextGroup* cgroup, Array<const RenderStage*>& rstages, Array<ImageUsage>& image_types, Array<ImageDependency>& dependencies ) override;

	virtual void render_bundles ( Array<RenderBundle*> bundles ) override;

//------------ Specialized Functions
	virtual RenderBundle* create_main_bundle ( ) override;

//------------ Transfer Data
//TODO move to seperate Transfer-Object which wraps all this in one logical unit
	VDividableBufferStore* current_staging_buffer_store = nullptr;

	VThinBuffer request_staging_buffer ( u64 size );

	vk::Fence request_fence();
	void free_fence ( vk::Fence fence );

	vk::CommandPool transfer_commandpool;
	
	vk::CommandBuffer request_transfer_command_buffer();
	void free_transfer_command_buffer ( vk::CommandBuffer buffer );

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
	void prepare_frame ();

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
		vk::Semaphore semaphore;
		vk::SemaphoreCreateInfo create_info;
		V_CHECKCALL(m_device.createSemaphore ( &create_info, nullptr, &semaphore ), printf("Could not create Semaphore\n"));
		return semaphore;
	}
	inline RendResult destroy_semaphore ( vk::Semaphore sem ) {
		m_device.destroySemaphore ( sem, nullptr );
		return RendResult::Success;
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
	vk::CommandBufferBeginInfo begin_info( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );
	cmdbuffer->begin ( &begin_info );
	RendResult res = do_command ( *cmdbuffer );
	cmdbuffer->end();
	return res;
}
