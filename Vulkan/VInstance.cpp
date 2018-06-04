#include "VInstance.h"

#include <vector>
#include <fstream>
#include "VGlobal.h"

VInstance::VInstance(vk::PhysicalDevice physicalDevice, uint32_t deviceId) :physicalDevice(physicalDevice), deviceId(deviceId){
}

VInstance::~VInstance() {
	
	deletePipelineModuleLayout(&pipeline_module_builders.standard, pipeline_module_layouts.standard);
	
	device.destroyShaderModule (global.shadermodule.standardShaderVert);
	device.destroyShaderModule (global.shadermodule.standardShaderFrag);

	device.destroy (nullptr);
}

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
vk::ShaderModule VInstance::loadShaderFromFile(const char* filename){
	
	std::vector<char> shaderCode = readFile(filename);
	
	vk::ShaderModuleCreateInfo createInfo(vk::ShaderModuleCreateFlags(), shaderCode.size(), (const uint32_t*)shaderCode.data());
	
	vk::ShaderModule shadermodule;
	V_CHECKCALL (device.createShaderModule(&createInfo, nullptr, &shadermodule), printf ("Creation of Shadermodule failed\n"));
	return shadermodule;
}