#pragma once

#include "render/Instance.h"
#include "render/Window.h"
#include "VHeader.h"
#include <render/IdArray.h>
#include "VRenderer.h"

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

	std::vector<vk::ExtensionProperties> availableExtensions;
	std::vector<vk::LayerProperties> availableLayers;
	std::vector<vk::QueueFamilyProperties> queueFamilyProps;
	vk::PhysicalDeviceProperties vkPhysDevProps;
	vk::PhysicalDeviceFeatures  vkPhysDevFeatures;

	virtual ~VDevice() {}
};

struct VInstance;

typedef std::function<void (VInstance* instance, u64 frame_index)> FrameFinishCallback;

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
struct VWindowSection;
struct VTransferController;
struct VContextBase;
struct VModelBase;
struct VModel;
struct VModelInstanceBase;
struct VRenderer;
struct VRenderStage;
struct VRenderer;

struct VInstance : public Instance {
	VExtLayerStruct extLayers;
	Map<GLFWmonitor*, VMonitor*> monitor_map;
	Map<GLFWwindow*, VWindow*> window_map;
	Set<VWindow*> windows;
	Set<VWindowSection*> window_sections;

	vk::Instance m_instance;
	VDevice* v_device;
	vk::Device m_device;
	vk::DebugReportCallbackEXT debugReportCallbackEXT;

	QueueWrapper queues;

	bool initialized = false;
	u64 frame_index = 1;

	VInstance();
	virtual ~VInstance();

	virtual bool initialize ( Device* device = nullptr );

	virtual void process_events();
	virtual bool is_window_open();

	virtual void render_window ( Window* window );
	virtual void v_render_window ( VWindow* window );
	virtual void render_windows();

	virtual Window* create_window();
	virtual bool destroy_window ( Window* window );

	VMonitor* get_primary_monitor_vulkan();
	virtual Monitor* get_primary_monitor();

//------------ Resources
	VTransferController* transfer_control = nullptr;

	IdPtrArray<DataGroupDef> datagroup_store;
	IdPtrArray<VContextBase> contextbase_store;
	IdPtrArray<VModelBase> modelbase_store;
	UIdPtrArray<VModel> model_store;
	IdPtrArray<VModelInstanceBase> modelinstancebase_store;
	IdPtrArray<VRenderer> vulkanrenderer_store;
	IdPtrArray<VRenderStage> vulkanrenderstage_store;

	vk::ShaderModule load_shader_from_file ( String filename );

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

	virtual const Renderer* create_renderer (
	    const ModelInstanceBase* model_instance_base,
	    Array<const ContextBase*> context_bases/*,
	    StringReference vertex_shader,
	    StringReference fragment_shader,
	    StringReference geometry_shader,
	    StringReference tesselation_control_shader,
	    StringReference tesselation_evaluation_shader*/
	) override;
	virtual const Renderer* renderer ( RId handle ) override;

	virtual const RenderStage* create_renderstage ( Array<const Renderer*> renderer, Array<void*> dependencies, Array<void*> inputs, Array<void*> outputs, Array<void*> temporaries ) override;
	virtual const RenderStage* renderstage ( RId handle ) override;
	
	virtual InstanceGroup* create_instancegroup() override;
	virtual ContextGroup* create_contextgroup() override;

	virtual RenderBundle* create_renderbundle(InstanceGroup* igroup, ContextGroup* cgroup, const RenderStage* rstage, void* targets);
	
	virtual void render_bundles(Array<RenderBundle*> bundles);
//------------ Instance/Device/Physicaldevice
	
	//wait until frame completion
	void v_wait_for_frame(u64 frame_index);
	
	void v_on_frame_finish(FrameFinishCallback callback);
	void v_on_current_frame_finish(FrameFinishCallback callback);
	
	GPUMemory allocate_gpu_memory ( vk::MemoryRequirements mem_req, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended );
	RendResult free_gpu_memory ( GPUMemory memory );

	vk::ImageView createImageView2D ( vk::Image image, u32 mipBase, u32 mipOffset, vk::Format format, vk::ImageAspectFlags aspectFlags );
	vk::ImageView createImageView2DArray ( vk::Image image, u32 mipBase, u32 mipOffset, u32 arrayOffset, u32 arraySize, vk::Format format, vk::ImageAspectFlags aspectFlags );
	void destroyImageView ( vk::ImageView imageview );

	vk::DeviceMemory allocate_memory ( vk::MemoryRequirements memoryRequirement, vk::MemoryPropertyFlags properties );

	vk::CommandPool createTransferCommandPool ( vk::CommandPoolCreateFlags createFlags = vk::CommandPoolCreateFlags() );
	vk::CommandPool createGraphicsCommandPool ( vk::CommandPoolCreateFlags createFlags = vk::CommandPoolCreateFlags() );
	void destroyCommandPool ( vk::CommandPool commandPool );
	vk::CommandBuffer createCommandBuffer ( vk::CommandPool commandPool, vk::CommandBufferLevel bufferLevel );
	void delete_command_buffer ( vk::CommandPool commandPool, vk::CommandBuffer commandBuffer );
	void copy_data ( const void* srcData, vk::DeviceMemory dstMemory, vk::DeviceSize offset, vk::DeviceSize size );

	u32 find_memory_type ( u32 typeFilter, vk::MemoryPropertyFlags properties );
	vk::Format find_supported_format ( const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features );
	vk::Format find_depth_format();
};


inline QueueWrapper* vulkan_queue_wrapper ( VInstance* instance ) {
	return &instance->queues;
}

inline vk::Device vulkan_device ( const VInstance* instance ) {
	return instance->m_device;
}
inline vk::PhysicalDevice vulkan_physical_device ( const VInstance* instance ) {
	return instance->v_device->physical_device;
}
inline vk::Semaphore create_semaphore ( VInstance* instance ) {
	return instance->m_device.createSemaphore ( vk::SemaphoreCreateInfo(), nullptr );
}
inline RendResult destroy_semaphore ( const VInstance* const instance, vk::Semaphore sem ) {
	instance->m_device.destroySemaphore ( sem, nullptr );
	return RendResult::eSuccess;
}

inline bool delete_command_buffer ( vk::Format format ) {
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

inline RendResult wrap_command ( std::function<RendResult ( vk::CommandBuffer ) > do_command, VInstance* v_instance, vk::CommandPool commandpool, vk::CommandBuffer* cmdbuffer ) {
	*cmdbuffer = v_instance->createCommandBuffer ( commandpool, vk::CommandBufferLevel::ePrimary );
	cmdbuffer->begin ( vk::CommandBufferBeginInfo ( vk::CommandBufferUsageFlagBits::eOneTimeSubmit ) );
	RendResult res = do_command ( *cmdbuffer );
	cmdbuffer->end();
	return res;
}
