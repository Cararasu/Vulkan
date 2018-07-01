#ifndef VULKANRESOURCEMANAGER_H
#define VULKANRESOURCEMANAGER_H

#include <VulkanHeader.h>

struct VulkanInstance;
struct VulkanResourceManager {
	VulkanInstance* v_instance;

	VulkanResourceManager ( VulkanInstance* instance ) : v_instance ( instance ) {}
	~VulkanResourceManager() {}

	vk::ShaderModule load_shader_from_file(const char* filename);
};

#endif // VULKANRESOURCEMANAGER_H
