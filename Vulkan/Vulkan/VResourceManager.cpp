#include "VResourceManager.h"
#include "VInstance.h"
#include "VImage.h"
#include "VSampler.h"
#include "VTransformEnums.h"
#include <fstream>

VResourceManager::VResourceManager ( VInstance* instance ) : v_instance ( instance ) {
}

VResourceManager::~VResourceManager() {
	for ( VShaderModule* shadermodule : shader_array ) {
		v_instance->vk_device ().destroyShaderModule ( shadermodule->shadermodule );
		delete shadermodule;
	}
	dependency_map.clear();
	for ( VBaseImage* image : v_images ) {
		if ( image ) v_delete_image(image);
	}
	v_images.clear();
	for ( VSampler* sampler : v_samplers ) {
		if ( sampler ) delete sampler;
	}
	v_samplers.clear();
}

static std::vector<char> readFile ( String filename ) {
	std::ifstream file ( filename.cstr, std::ios::ate | std::ios::binary );

	if ( !file.is_open() ) {
		v_logger.log<LogLevel::eError> ( "Couldn't open File %s", filename.cstr );
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
	V_CHECKCALL ( v_instance->vk_device ().createShaderModule ( &createInfo, nullptr, &shadermodule ), v_logger.log<LogLevel::eError> ( "Creation of Shadermodule failed" ) );
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
	for ( auto& it : shader_string_id_map ) {
		//TODO HACK
		if ( it.first == ref.name ) {
			return shader_array.get ( it.second );
		}
	}
	return nullptr;
}
ShaderModule* VResourceManager::get_shader ( StringReference ref ) {
	return v_get_shader ( ref );
}

Image* VResourceManager::create_texture ( u32 width, u32 height, u32 depth, u32 array_layers, u32 mipmap_layers, ImageFormat format ) {
	
	vk::ImageUsageFlags usages = vk::ImageUsageFlags() | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
	vk::ImageAspectFlags aspectFlags;
	switch ( format ) {
	case ImageFormat::eD16Unorm:
	case ImageFormat::eD32F: {
		usages |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
		aspectFlags |= vk::ImageAspectFlagBits::eDepth;
	}
	break;
	case ImageFormat::eD24Unorm_St8U:
	case ImageFormat::eD32F_St8U: {
		usages |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
		aspectFlags |= vk::ImageAspectFlagBits::eDepth;
		aspectFlags |= vk::ImageAspectFlagBits::eStencil;
	}
	break;
	case ImageFormat::eSt8U: {
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
	
	return v_images.insert ( new VBaseImage ( v_instance, width, height, depth, array_layers, mipmap_layers, transform_image_format(format), vk::ImageTiling::eOptimal,
	                         usages, aspectFlags, vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal ) );
}
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
Image* VResourceManager::load_image_to_texture ( std::string file, Image* image, u32 array_layer, u32 mipmap_layer ) {
	VBaseImage* v_image = static_cast<VBaseImage*> ( image );
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load ( file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if ( !pixels ) {
		v_logger.log<LogLevel::eWarn> ( "failed to load texture image from file %s", file.c_str() );
		throw std::runtime_error ( "failed to load texture image!" );
	}
	VThinBuffer buffer = v_instance->request_staging_buffer ( imageSize );
	memcpy ( buffer.mapped_ptr, pixels, imageSize );
	vk::CommandBuffer cmdbuffer = v_instance->request_transfer_command_buffer();
	v_instance->free_transfer_command_buffer ( cmdbuffer );
	vk::CommandBufferBeginInfo begininfo ( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

	cmdbuffer.begin ( begininfo );
	v_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, cmdbuffer );
	vk::BufferImageCopy bufferimagecopy ( 0, texWidth, texHeight, {vk::ImageAspectFlagBits::eColor, mipmap_layer, array_layer, 1}, {0, 0, 0}, {texWidth, texHeight, 1} );
	cmdbuffer.copyBufferToImage ( buffer.buffer, v_image->image, vk::ImageLayout::eTransferDstOptimal, 1, &bufferimagecopy );

	cmdbuffer.end();

	vk::SubmitInfo submitinfos[1] = {
		vk::SubmitInfo (
		    0, nullptr, //waitSem
		    nullptr,
		    1, &cmdbuffer,//commandbuffers
		    0, nullptr//signalSem
		)
	};
	vk::Fence fence = v_instance->request_fence();
	v_instance->queue_wrapper()->transfer_queue.submit ( 1, submitinfos, fence );
	//TODO make asynch
	if ( fence ) {
		v_instance->vk_device().waitForFences ( {fence}, true, std::numeric_limits<u64>::max() );
		v_instance->vk_device().resetFences ( {fence} );
		v_instance->free_data.fences.push_back ( fence );
	}
	stbi_image_free ( pixels );
	return image;
}
Image* VResourceManager::load_image_to_texture ( std::string file, u32 mipmap_layers ) {
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load ( file.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if ( !pixels ) {
		throw std::runtime_error ( "failed to load texture image!" );
	}
	VBaseImage* v_image = v_images.insert ( new VBaseImage ( v_instance, texWidth, texHeight, 0, 1, mipmap_layers, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
	                                        vk::ImageUsageFlags() | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc, vk::ImageAspectFlagBits::eColor,
	                                        vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal
	                                                       ) );

	VThinBuffer buffer = v_instance->request_staging_buffer ( imageSize );
	vk::CommandBuffer cmdbuffer = v_instance->request_transfer_command_buffer();
	v_instance->free_transfer_command_buffer ( cmdbuffer );

	memcpy ( buffer.mapped_ptr, pixels, imageSize );
	vk::CommandBufferBeginInfo begininfo = {
		vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr
	};

	cmdbuffer.begin ( begininfo );
	v_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, cmdbuffer );
	vk::BufferImageCopy bufferimagecopy ( 0, texWidth, texHeight, {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, {0, 0, 0}, {texWidth, texHeight, 1} );
	cmdbuffer.copyBufferToImage ( buffer.buffer, v_image->image, vk::ImageLayout::eTransferDstOptimal, 1, &bufferimagecopy );
	//v_image->generate_mipmaps(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, cmdbuffer);
	cmdbuffer.end();

	vk::SubmitInfo submitinfos[1] = {
		vk::SubmitInfo (
		    0, nullptr, //waitSem
		    nullptr,
		    1, &cmdbuffer,//commandbuffers
		    0, nullptr//signalSem
		)
	};
	vk::Fence fence = v_instance->request_fence();
	v_instance->queue_wrapper()->transfer_queue.submit ( 1, submitinfos, fence );
	//TODO make asynch
	if ( fence ) {
		v_instance->vk_device().waitForFences ( {fence}, true, std::numeric_limits<u64>::max() );
		v_instance->vk_device().resetFences ( {fence} );
		v_instance->free_data.fences.push_back ( fence );
	}
	stbi_image_free ( pixels );
	return v_image;
}
Image* VResourceManager::create_dependant_image ( Image* image, ImageFormat type, u32 mipmap_layers, float scaling ) {
	VBaseImage* base_image = dynamic_cast<VBaseImage*> ( image );
	if ( !base_image ) return nullptr;
	return v_create_dependant_image ( base_image, type, mipmap_layers, scaling );
}
VBaseImage* VResourceManager::v_create_dependant_image ( VBaseImage* base_image, ImageFormat type, u32 mipmap_layers, float scaling ) {
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
	case ImageFormat::eD32F_St8U: {
		usages |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
		aspectFlags |= vk::ImageAspectFlagBits::eDepth;
		aspectFlags |= vk::ImageAspectFlagBits::eStencil;
	}
	break;
	case ImageFormat::eSt8U: {
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
	VBaseImage* v_wrapper = v_images.insert ( new VBaseImage ( base_image->v_instance,
	                        base_image->width,
	                        base_image->height,
	                        base_image->depth,
	                        1,
	                        mipmap_layers,
	                        format,
	                        vk::ImageTiling::eOptimal,
	                        usages | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment,
	                        aspectFlags,
							base_image->id,
							scaling,
	                        vk::MemoryPropertyFlagBits::eDeviceLocal,
							vk::MemoryPropertyFlags() ) );
	auto it = dependency_map.find ( base_image );
	if ( it == dependency_map.end() ) {
		it = dependency_map.insert ( it, std::make_pair ( base_image, DynArray<VBaseImage*>() ) );
	}
	it->second.push_back ( v_wrapper );
	return v_wrapper;
}
void VResourceManager::v_delete_dependant_images ( VBaseImage* image ) {
	auto it = dependency_map.find ( image );
	if ( it != dependency_map.end() ) {
		for ( VBaseImage* img : it->second ) {
			v_delete_image(img);
		}
		it->second.clear();
		it = dependency_map.erase ( it );
	}
}
void VResourceManager::delete_image ( Image* image ) {
	v_delete_image ( static_cast<VBaseImage*> ( image ) );
}
void VResourceManager::v_delete_image ( VBaseImage* image ) {
	if(image->dependant_image) {
		auto it = dependency_map.find ( v_images[image->dependant_image] );
		if ( it != dependency_map.end() ) {
			DynArray<VBaseImage*>& images = it->second;
			bool deleted = false;
			for ( auto imit = images.begin(); imit != images.end(); ) {
				if(image == *imit) {
					imit = images.erase(imit);
					deleted = true;
					continue;
				}
				imit++;
			}
			assert(deleted);
		}
	}
	delete v_images.remove ( image->id );
}
Sampler* VResourceManager::create_sampler ( FilterType magnification, FilterType minification, FilterType mipmapping, EdgeHandling u, EdgeHandling v, EdgeHandling w, float lodbias, Range<float> lodrange, float anismax, DepthComparison comp ) {
	VSampler* sampler = new VSampler ( v_instance, magnification, minification, mipmapping, u, v, w, lodbias, lodrange, anismax, comp );
	v_samplers.insert ( sampler );
	return sampler;
}
