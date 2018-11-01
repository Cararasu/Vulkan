#include "VResourceManager.h"
#include "VInstance.h"
#include <fstream>

VResourceManager::VResourceManager(VInstance* instance) : instance(instance) {
}

VResourceManager::~VResourceManager() {
}

static std::vector<char> readFile ( String filename ) {
	std::ifstream file ( filename.cstr, std::ios::ate | std::ios::binary );

	if ( !file.is_open() ) {
		printf ( "Couldn't open File %s\n", filename.cstr );
		return std::vector<char>();
	}
	size_t fileSize = ( size_t ) file.tellg();
	std::vector<char> buffer ( fileSize );

	file.seekg ( 0 );
	file.read ( buffer.data(), fileSize );
	file.close();

	return buffer;
}


u64 VResourceManager::load_shader (ShaderType type, String name, String filename) {
	VShaderModule* module = v_get_shader(name);
	if(module) return module->id;
	
	std::vector<char> shaderCode = readFile ( filename.cstr );

	if ( !shaderCode.size() ) {
		v_logger.log<LogLevel::eWarn> ( "Shader from File %s could not be loaded or is empty", filename.cstr );
		return 0;
	}

	vk::ShaderModuleCreateInfo createInfo ( vk::ShaderModuleCreateFlags(), shaderCode.size(), ( const u32* ) shaderCode.data() );

	vk::ShaderModule shadermodule;
	V_CHECKCALL ( instance->vk_device ().createShaderModule ( &createInfo, nullptr, &shadermodule ), printf ( "Creation of Shadermodule failed\n" ) );
	module = new VShaderModule(type, name, shadermodule);
	shader_array.insert(module);
	shader_string_id_map.insert(std::make_pair(name, module->id));
	return module->id;
}

VShaderModule* VResourceManager::v_get_shader(StringReference ref) {
	if(ref.id) {
		return shader_array.get(ref.id);
	}
	auto it = shader_string_id_map.find(ref.name);
	if(it != shader_string_id_map.end()){
		return shader_array.get(it->second);
	}
	return nullptr;
}
ShaderModule* VResourceManager::get_shader(StringReference ref) {
	return v_get_shader(ref);
}