
#include "VHeader.h"
#include <vector>
#include <fstream>
#include "VBuilders.h"

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
VkShaderModule loadShaderFromFile(const char* filename){
	
	std::vector<char> shaderCode = readFile(filename);
	
	return ShaderBuilder().setShaderCode(shaderCode).build();
}