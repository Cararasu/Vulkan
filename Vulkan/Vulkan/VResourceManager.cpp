#include "VResourceManager.h"
#include "VInstance.h"
#include "VImage.h"
#include <fstream>

VResourceManager::VResourceManager ( VInstance* instance ) : instance ( instance ) {
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


u64 VResourceManager::load_shader ( ShaderType type, String name, String filename ) {
	VShaderModule* module = v_get_shader ( name );
	if ( module ) return module->id;

	std::vector<char> shaderCode = readFile ( filename.cstr );

	if ( !shaderCode.size() ) {
		v_logger.log<LogLevel::eWarn> ( "Shader from File %s could not be loaded or is empty", filename.cstr );
		return 0;
	}

	vk::ShaderModuleCreateInfo createInfo ( vk::ShaderModuleCreateFlags(), shaderCode.size(), ( const u32* ) shaderCode.data() );

	vk::ShaderModule shadermodule;
	V_CHECKCALL ( instance->vk_device ().createShaderModule ( &createInfo, nullptr, &shadermodule ), printf ( "Creation of Shadermodule failed\n" ) );
	module = new VShaderModule ( type, name, shadermodule );
	shader_array.insert ( module );
	shader_string_id_map.insert ( std::make_pair ( name, module->id ) );
	return module->id;
}

VShaderModule* VResourceManager::v_get_shader ( StringReference ref ) {
	if ( ref.id ) {
		return shader_array.get ( ref.id );
	}
	auto it = shader_string_id_map.find ( ref.name );
	if ( it != shader_string_id_map.end() ) {
		return shader_array.get ( it->second );
	}
	return nullptr;
}
ShaderModule* VResourceManager::get_shader ( StringReference ref ) {
	return v_get_shader ( ref );
}

Image* VResourceManager::create_dependant_image ( Image* image, ImageFormat type, float scaling ) {
	VBaseImage* base_image = dynamic_cast<VBaseImage*> ( image );
	if ( !base_image ) return nullptr;
	return v_create_dependant_image ( base_image, type, scaling );
}
VImageWrapper* VResourceManager::v_create_dependant_image ( VBaseImage* base_image, ImageFormat type, float scaling ) {
	vk::Format format = transform_image_format ( type );

	vk::ImageUsageFlags usages;
	vk::ImageAspectFlags aspectFlags;
	switch ( type ) {
	case ImageFormat::eD16Unorm:
	case ImageFormat::eD32F: {
		usages |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
		aspectFlags |= vk::ImageAspectFlagBits::eDepth;
	}
	break;
	case ImageFormat::eD24Unorm_St8U:
	case ImageFormat::eD32F_St8Uint: {
		usages |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
		aspectFlags |= vk::ImageAspectFlagBits::eDepth;
		aspectFlags |= vk::ImageAspectFlagBits::eStencil;
	}
	break;
	case ImageFormat::eSt8Uint: {
		usages |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
		aspectFlags |= vk::ImageAspectFlagBits::eStencil;
	}
	break;
	default: {
		usages |= vk::ImageUsageFlagBits::eColorAttachment;
		aspectFlags |= vk::ImageAspectFlagBits::eColor;
	}
	break;
	}
	//TODO make eSampled and eInputAttachment dynamically
	VImageWrapper* v_wrapper = new VImageWrapper ( base_image->v_instance,
	        base_image->width,
			base_image->height,
			base_image->depth,
	        1,
	        base_image->mipmap_layers,
	        format,
	        vk::ImageTiling::eOptimal,
	        usages | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment,
	        aspectFlags,
	        vk::MemoryPropertyFlagBits::eDeviceLocal );
	images.insert ( v_wrapper );
	auto it = dependency_map.find ( base_image );
	if ( it == dependency_map.end() ) {
		it = dependency_map.insert ( it, std::make_pair ( base_image, DynArray<VImageWrapper*>() ) );
	}
	it->second.push_back ( v_wrapper );
	v_wrapper->dependent = true;
	v_wrapper->fraction = scaling;
	return v_wrapper;
}
void VResourceManager::v_delete_dependant_images(VBaseImage* image) {
	auto it = dependency_map.find(image);
	if(it != dependency_map.end()) {
		for(VImageWrapper* image : it->second) {
			delete image;
		}
		dependency_map.erase(it);
	}
}
void VResourceManager::delete_image ( Image* image ) {

}
