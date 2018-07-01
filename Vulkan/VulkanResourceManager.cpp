#include "VulkanResourceManager.h"

#include "VulkanInstance.h"
#include <fstream>

static std::vector<char> readFile(const char* filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		printf("Couldn't open File %s\n", filename);
		return std::vector<char>();
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}
vk::ShaderModule VulkanResourceManager::load_shader_from_file(const char* filename){
	
	std::vector<char> shaderCode = readFile(filename);
	
	vk::ShaderModuleCreateInfo createInfo(vk::ShaderModuleCreateFlags(), shaderCode.size(), (const u32*)shaderCode.data());
	
	vk::ShaderModule shadermodule;
	V_CHECKCALL (vulkan_device(v_instance).createShaderModule(&createInfo, nullptr, &shadermodule), printf ("Creation of Shadermodule failed\n"));
	return shadermodule;
}