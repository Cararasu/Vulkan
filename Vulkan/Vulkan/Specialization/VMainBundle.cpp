#include "VMainBundle.h"
#include "../VImage.h"
#include "../VModel.h"
#include "../VContext.h"
#include "../VImage.h"
#include "../VInstance.h"
#include <render/Specialization.h>
#include "../VShader.h"
#include "../VResourceManager.h"
#include "../VTransformEnums.h"

VMainBundle::VMainBundle ( VInstance* instance, InstanceGroup* igroup, ContextGroup* cgroup ) :
	v_instance ( instance ),
	v_igroup ( static_cast<VInstanceGroup*> ( igroup ) ),
	v_cgroup ( static_cast<VContextGroup*> ( cgroup ) ),
	v_bundleStates ( 2 ) {

}

VMainBundle::~VMainBundle() {
	v_destroy_pipeline_layouts();
}
void VMainBundle::v_destroy_pipeline_layouts() {
	v_destroy_renderpasses();
	v_destroy_pipelines();
	if ( v_object_pipeline_layout ) {
		v_instance->vk_device ().destroyPipelineLayout ( v_object_pipeline_layout );
		v_object_pipeline_layout = vk::PipelineLayout();
	}
}
void VMainBundle::v_destroy_pipelines() {
	if ( v_object_pipeline ) {
		v_instance->vk_device ().destroyPipeline ( v_object_pipeline );
		v_object_pipeline = vk::Pipeline();
	}
}
void VMainBundle::v_destroy_renderpasses() {
	if ( v_renderpass ) {
		v_instance->vk_device ().destroyRenderPass ( v_renderpass );
		v_renderpass = vk::RenderPass();
	}
}

void VMainBundle::set_rendertarget ( u32 index, Image* image ) {
	assert ( index < v_bundleStates.size );
	VBundleImageState& imagestate = v_bundleStates[index];
	imagestate.actual_image = static_cast<VBaseImage*> ( image );

	if ( imagestate.actual_image->v_format != imagestate.current_format ) {
		imagestate.current_format = imagestate.actual_image->v_format;
		v_destroy_renderpasses();
	}
	
}
void VMainBundle::v_check_rebuild_pipelines() {
	u32 width = 0, height = 0;
	for ( VBundleImageState& imagestate : v_bundleStates ) {
		if ( !imagestate.actual_image ) {
			printf ( "One or more Images not set for MainBundle\n" );
			return;
		}
		printf ( "Image_build_index %" PRId64 "\n", imagestate.actual_image->created_frame_index );
		if ( last_frame_index_pipeline_built <= imagestate.actual_image->created_frame_index ) {
			v_destroy_pipeline_layouts();
		}
		
		assert(!width || width == imagestate.actual_image->extent.width);
		assert(!height || height == imagestate.actual_image->extent.height);
		width = imagestate.actual_image->extent.width;
		height = imagestate.actual_image->extent.height;
		
	}
	if(viewport.extend.width != width || viewport.extend.height != height)
		
	viewport = Viewport<f32> ( 0.0f, 0.0f, width, height, 0.0f, 1.0f );
}

void VMainBundle::v_rebuild_pipelines() {
	if ( !v_object_pipeline_layout ) {
		printf ( "Rebuild Pipeline Layouts %" PRId64 "\n", v_instance->frame_index );
		vk::DescriptorSetLayoutBinding bindings1[] = {
			vk::DescriptorSetLayoutBinding ( 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr ),
		};
		vk::DescriptorSetLayoutBinding bindings2[] = {
			vk::DescriptorSetLayoutBinding ( 0, vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr ),
			vk::DescriptorSetLayoutBinding ( 1, vk::DescriptorType::eSampledImage, 3, vk::ShaderStageFlagBits::eFragment, nullptr )
		};

		v_descriptor_set_layouts = {
			//v_instance->vk_device ().createDescriptorSetLayout ( vk::DescriptorSetLayoutCreateInfo ( vk::DescriptorSetLayoutCreateFlags(), 1, bindings1 ), nullptr ),
			//v_instance->vk_device ().createDescriptorSetLayout ( vk::DescriptorSetLayoutCreateInfo ( vk::DescriptorSetLayoutCreateFlags(), 2, bindings2 ), nullptr )
		};

		std::array<vk::PushConstantRange, 0> pushConstRanges = {};//{vk::PushConstantRange ( vk::ShaderStageFlagBits::eFragment, 0, sizeof ( ObjectPartData ) ) };

		vk::PipelineLayoutCreateInfo createInfo (
		    vk::PipelineLayoutCreateFlags(),
		    0, nullptr,//setLayouts
		    0, nullptr//pushConstRanges
		);
		if ( v_descriptor_set_layouts.size != 0 ) {
			createInfo.setLayoutCount = v_descriptor_set_layouts.size;
			createInfo.pSetLayouts = v_descriptor_set_layouts.data;
		}
		if ( pushConstRanges.size() != 0 ) {
			createInfo.pushConstantRangeCount = pushConstRanges.size();
			createInfo.pPushConstantRanges = pushConstRanges.data();
		}
		v_object_pipeline_layout = v_instance->vk_device ().createPipelineLayout ( createInfo, nullptr );
	}
	if ( !v_renderpass ) {
		printf ( "Rebuild Renderpasses %" PRId64 "\n", v_instance->frame_index );
		vk::AttachmentDescription attachments[2] = {
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            v_bundleStates[0].current_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eDontCare,//loadOp
			                            vk::AttachmentStoreOp::eStore,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eColorAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eColorAttachmentOptimal//finalLayout
			                          ),
			vk::AttachmentDescription ( vk::AttachmentDescriptionFlags(),
			                            v_bundleStates[1].current_format, vk::SampleCountFlagBits::e1,//format, samples
			                            vk::AttachmentLoadOp::eDontCare,//loadOp
			                            vk::AttachmentStoreOp::eDontCare,//storeOp
			                            vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			                            vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			                            vk::ImageLayout::eDepthStencilAttachmentOptimal,//initialLaylout
			                            vk::ImageLayout::eDepthStencilAttachmentOptimal//finalLayout
			                          )
		};

		vk::AttachmentReference colorAttachmentRefs[] = {
			vk::AttachmentReference ( 0, vk::ImageLayout::eColorAttachmentOptimal )
		};
		vk::AttachmentReference depthAttachmentRef ( 1, vk::ImageLayout::eDepthStencilAttachmentOptimal );

		vk::SubpassDescription subpasses[] = {
			vk::SubpassDescription (
			    vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics,
			    0, nullptr/*inputAttachments*/,
			    1, colorAttachmentRefs/*colorAttachments*/,
			    nullptr,/*resolveAttachments*/
			    &depthAttachmentRef,/*depthAttackment*/
			    0, nullptr/*preserveAttachments*/
			)
		};

		vk::RenderPassCreateInfo renderPassInfo ( vk::RenderPassCreateFlags(), 2, attachments, 1, subpasses, 0, nullptr/*dependencies*/ );

		v_renderpass = v_instance->vk_device ().createRenderPass ( renderPassInfo, nullptr );
	}
	if ( !v_object_pipeline ) {
		printf ( "Rebuild Pipelines %" PRId64 "\n", v_instance->frame_index );
		
		const ModelInstanceBase* modelinstancebase = v_instance->modelinstancebase(0);
		
		const DataGroupDef* vertex_datagroup = v_instance->datagroupdef(vertex_datagroup_def);
		const DataGroupDef* instance_datagroup = v_instance->datagroupdef(matrix_datagroup_def);
		
		
		std::array<vk::VertexInputBindingDescription, 2> vertexInputBindings = {
			vk::VertexInputBindingDescription ( 0, vertex_datagroup->size, vk::VertexInputRate::eVertex ),
			vk::VertexInputBindingDescription ( 1, instance_datagroup->size, vk::VertexInputRate::eInstance )
		};
		
		Array<vk::VertexInputAttributeDescription> vertexInputAttributes;
		u32 valuecount = 0;
		for(DataValueDef& valuedef : vertex_datagroup->valuedefs) {
			valuecount += to_v_format(valuedef.type).count * valuedef.arraycount;
			printf("Value: %s %d[%d]\n", to_string(valuedef.type), valuedef.offset, valuedef.arraycount);
		}
		vertexInputAttributes.resize(valuecount);
		u32 offset = 0;
		for(DataValueDef& valuedef : vertex_datagroup->valuedefs) {
			VFormatData formatdata = to_v_format(valuedef.type);
			u32 count = formatdata.count * valuedef.arraycount;
			for(int i = 0; i < count; i++) {
				vertexInputAttributes[offset + i] = vk::VertexInputAttributeDescription ( offset + i, 0, formatdata.format, valuedef.offset/* + value*/ );
			}
			offset += count;
		}

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo ( vk::PipelineVertexInputStateCreateFlags(),
				vertexInputBindings.size(), vertexInputBindings.data(),
				vertexInputAttributes.size, vertexInputAttributes.data );

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly ( vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList, VK_FALSE );

		vk::Viewport viewports[] = {
			vk::Viewport ( viewport.offset.x, viewport.offset.y, viewport.extend.width, viewport.extend.height, viewport.depth.min, viewport.depth.max )
		};

		vk::Rect2D scissors[] = {
			vk::Rect2D ( vk::Offset2D ( 0, 0 ), vk::Extent2D ( viewport.extend.width, viewport.extend.height ) ),
		};

		vk::PipelineViewportStateCreateInfo viewportState ( vk::PipelineViewportStateCreateFlags(), 1, viewports, 1, scissors );

		vk::PipelineRasterizationStateCreateInfo rasterizer ( vk::PipelineRasterizationStateCreateFlags(),
				VK_FALSE, VK_FALSE, //depthClampEnable, rasterizerDiscardEnable
				vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise,
				VK_FALSE, //depthBiasEnable
				0.0f, //depthBiasConstantFactor
				0.0f, //depthBiasClamp
				0.0f, //depthBiasSlopeFactor
				1.0f ); //lineWidth

		vk::PipelineMultisampleStateCreateInfo multisampling (
			vk::PipelineMultisampleStateCreateFlags(),
			vk::SampleCountFlagBits::e1,
			VK_FALSE,//sampleShadingEnable
			1.0f, nullptr, //minSampleShading, pSampleMask
			VK_FALSE, VK_FALSE //alphaToCoverageEnable, alphaToOneEnable
		);

		vk::PipelineDepthStencilStateCreateInfo depthStencil (
			vk::PipelineDepthStencilStateCreateFlags(),
			VK_TRUE, VK_TRUE, //depthTestEnable, depthWriteEnable
			vk::CompareOp::eLess, //depthCompareOp
			VK_FALSE, VK_FALSE, //depthBoundsTestEnable, stencilTestEnable
			{}, {}, //fron, back
			0.0f, 1.0f //minDepthBounds, maxDepthBounds
		);

		vk::PipelineColorBlendAttachmentState colorBlendAttachments[1] = {
			vk::PipelineColorBlendAttachmentState (
				VK_FALSE, //blendEnable
				vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcColorBlendFactor, dstColorBlendFactor
				vk::BlendOp::eAdd,//colorBlendOp
				vk::BlendFactor::eOne, vk::BlendFactor::eZero, //srcAlphaBlendFactor, dstAlphaBlendFactor
				vk::BlendOp::eAdd,//alphaBlendOp
				vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA//colorWriteMask
			)
		};

		vk::PipelineColorBlendStateCreateInfo colorBlending (
			vk::PipelineColorBlendStateCreateFlags(),
			VK_FALSE, vk::LogicOp::eCopy,//logicOpEnable, logicOp
			1, colorBlendAttachments, // attachments
			{0.0f, 0.0f, 0.0f, 0.0f} //blendConstants
		);
		
		VShaderModule* vmod = v_instance->m_resource_manager->v_get_shader(StringReference("vert_shader"));
		VShaderModule* fmod = v_instance->m_resource_manager->v_get_shader(StringReference("frag_shader"));

		vk::PipelineShaderStageCreateInfo shaderStages[2] = {
			vk::PipelineShaderStageCreateInfo (
				vk::PipelineShaderStageCreateFlags(),
				vk::ShaderStageFlagBits::eVertex, vmod->shadermodule,
				"main", nullptr//name, specialization
			),
			vk::PipelineShaderStageCreateInfo (
				vk::PipelineShaderStageCreateFlags(),
				vk::ShaderStageFlagBits::eFragment, fmod->shadermodule,
				"main", nullptr//name, specialization
			),
		};
		vk::GraphicsPipelineCreateInfo pipelineInfo (
			vk::PipelineCreateFlags(),
			2, shaderStages,
			&vertexInputInfo, &inputAssembly, nullptr, &viewportState, &rasterizer, &multisampling, &depthStencil, &colorBlending,
			nullptr,
			v_object_pipeline_layout,
			v_renderpass,
			0,
			vk::Pipeline(),
			-1
		);

		v_object_pipeline = v_instance->vk_device ().createGraphicsPipelines ( vk::PipelineCache(), {pipelineInfo}, nullptr ) [0];

	}
	last_frame_index_pipeline_built = v_instance->frame_index;
}
void VMainBundle::v_dispatch() {
	v_check_rebuild_pipelines();
	v_rebuild_pipelines();


	last_used = v_instance->frame_index;
}
