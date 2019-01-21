#include "VMainBundle.h"
#include "../VImage.h"
#include "../VModel.h"
#include "../VContext.h"
#include "../VImage.h"
#include "../VInstance.h"
#include <render/Specialization.h>
#include <render/Timing.h>
#include "../VShader.h"
#include "../VResourceManager.h"
#include "../VTransformEnums.h"
#include "../VWindow.h"
#include "VMainRenderStage.h"
#include "VBloomRenderStage.h"
#include "VFinalCompositionRenderStage.h"
#include "VShadowMapGeneration.h"
#include "VHDRRenderStage.h"


void gen_pipeline_layout ( VInstance* v_instance, SubPassInput* subpass_input, PipelineStruct* p_struct, PushConstUsed* pushconsts ) {
	if ( !p_struct->pipeline_layout ) {
		v_logger.log<LogLevel::eTrace> ( "Rebuild Pipeline Layouts" );

		DynArray<vk::DescriptorSetLayout> v_descriptor_set_layouts;
		//contexts with the input attachments
		if ( subpass_input->ds_layout ) v_descriptor_set_layouts.push_back ( subpass_input->ds_layout );
		//contexts from the contextgroup
		for ( ContextBaseId id : p_struct->contextBaseId ) {
			VContextBase& v_contextbase = v_instance->contextbase_map[id];
			if ( v_contextbase.descriptorset_layout ) v_descriptor_set_layouts.push_back ( v_contextbase.descriptorset_layout );
		}
		//contexts from the model
		for ( ContextBaseId id : p_struct->model_contextBaseId ) {
			VContextBase& v_contextbase = v_instance->contextbase_map[id];
			if ( v_contextbase.descriptorset_layout ) v_descriptor_set_layouts.push_back ( v_contextbase.descriptorset_layout );
		}

		vk::PipelineLayoutCreateInfo createInfo (
		    vk::PipelineLayoutCreateFlags(),
		    0, nullptr,//setLayouts
		    0, nullptr//pushConstRanges
		);
		if ( v_descriptor_set_layouts.size() != 0 ) {
			createInfo.setLayoutCount = v_descriptor_set_layouts.size();
			createInfo.pSetLayouts = v_descriptor_set_layouts.data();
		}
		std::array<vk::PushConstantRange, 1> pushConstRanges = {};
		if(pushconsts) {
			pushConstRanges[0].stageFlags = vk::ShaderStageFlagBits::eAllGraphics;
			pushConstRanges[0].offset = pushconsts->offset;
			pushConstRanges[0].size = pushconsts->size;
			createInfo.pushConstantRangeCount = pushConstRanges.size();
			createInfo.pPushConstantRanges = pushConstRanges.data();
		}
		p_struct->pipeline_layout = v_instance->vk_device ().createPipelineLayout ( createInfo, nullptr );
	}
}

void destroy_pipeline ( VInstance* v_instance, PipelineStruct* p_struct ) {
	for(u32 i = 0; i < p_struct->pipelines.size; i++ ){
		if ( p_struct->pipelines[i] ) {
			v_instance->vk_device ().destroyPipeline ( p_struct->pipelines[i] );
			p_struct->pipelines[i] = vk::Pipeline();
		}
	}
}
void destroy_pipeline_layout ( VInstance* v_instance, PipelineStruct* p_struct ) {
	if ( p_struct->pipeline_layout ) {
		v_instance->vk_device ().destroyPipelineLayout ( p_struct->pipeline_layout );
		p_struct->pipeline_layout = vk::PipelineLayout();
	}
}
void update_instancegroup ( VInstance* v_instance, VInstanceGroup* igroup, vk::CommandBuffer buffer ) {
	if ( igroup->last_updated_frame_index < v_instance->frame_index ) {
		igroup->last_updated_frame_index = v_instance->frame_index;
		void* instance_data = igroup->buffer_storeage.allocate_transfer_buffer();
		for ( auto it = igroup->instance_to_data_map.begin(); it != igroup->instance_to_data_map.end(); it++ ) {
			const InstanceBase* instancebase = v_instance->instancebase ( it->first );
			for ( InstanceBlock& block : it->second ) {
				if ( block.data ) memcpy ( instance_data + block.offset, block.data, instancebase->instance_datagroup.size * block.count );
			}
		}
		igroup->buffer_storeage.transfer_data ( buffer );
	}
}
void update_contexts ( VInstance* v_instance, VContextGroup* cgroup, vk::CommandBuffer buffer ) {
	if ( v_instance->last_contexts_updated_frame_index < v_instance->frame_index ) {
		VUpdateableBufferStorage* bufferstorage = v_instance->context_bufferstorage;
		bufferstorage->fetch_transferbuffers();
		for ( auto it = v_instance->v_context_map.begin(); it != v_instance->v_context_map.end(); it++ ) {
			for ( VContext* v_context : it->second ) {
				if ( v_context->data ) {
					VThinBuffer staging = bufferstorage->get_buffer_pair ( v_context->buffer_chunk.index ).second;
					memcpy ( staging.mapped_ptr + v_context->buffer_chunk.offset, v_context->data, v_context->buffer_chunk.size );
				}
			}
		}
		for ( std::pair<VBuffer*, VThinBuffer> p : bufferstorage->buffers ) {
			vk::BufferCopy buffercopy ( 0, 0, p.first->size );
			//printf("VThinBuffer transfer 0x%x - 0x%x\n", p.second.buffer, p.first->buffer);
			buffer.copyBuffer(p.second.buffer, p.first->buffer, 1, &buffercopy);
		}
		bufferstorage->free_transferbuffers ( v_instance->frame_index );
	}
	
	DynArray<vk::ImageMemoryBarrier> barriers;
	barriers.reserve(32);
	vk::PipelineStageFlags sourceStage, destinationStage;
	
	Map<ModelBaseId, IdPtrArray<VModel>> v_model_map;
	for(auto& p : v_instance->v_model_map) {
		for(VModel* v_model : p.second) {
			for(VContext* v_context : v_model->v_contexts) {
				if(v_context) {
					for ( VImageUseRef& vimageuseref : v_context->images ) {
						if ( vimageuseref ) {
							VImageUse* imageuse = vimageuseref.deref();
							barriers.push_back(vimageuseref.image->transition_layout_impl ( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, 
								imageuse->mipmaps, imageuse->layers, &sourceStage, &destinationStage ));
						}
					}
				}
			}
		}
	}
	for ( auto it = cgroup->context_map.begin(); it != cgroup->context_map.end(); it++ ) {
		VContext* v_context = it->second;
		v_context->update_if_needed();
		for ( VImageUseRef& vimageuseref : v_context->images ) {
			if ( vimageuseref ) {
				VImageUse* imageuse = vimageuseref.deref();
				barriers.push_back(vimageuseref.image->transition_layout_impl ( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, 
					imageuse->mipmaps, imageuse->layers, &sourceStage, &destinationStage ));
			}
		}
	}
	buffer.pipelineBarrier (
		sourceStage, destinationStage,
		vk::DependencyFlags(),
		{},//memoryBarriers
		{},//bufferBarriers
		barriers//imageBarriers
	);
}
void render_pipeline ( VInstance* v_instance, VInstanceGroup* igroup, VContextGroup* cgroup, PipelineStruct* p_struct, SubPassInput* renderpass_struct, vk::CommandBuffer cmdbuffer, u32 pipeline_index ) {

	DynArray<InstanceBlock>& instanceblocks = igroup->instance_to_data_map[p_struct->instancebase_id];

	u32 descriptor_offset = 0;

	if ( renderpass_struct->ds_set ) {
		cmdbuffer.bindDescriptorSets ( vk::PipelineBindPoint::eGraphics, p_struct->pipeline_layout, descriptor_offset, 1, &renderpass_struct->ds_set, 0, nullptr );
		descriptor_offset += 1;
	}

	DynArray<vk::DescriptorSet> descriptorSets;
	for ( ContextBaseId id : p_struct->contextBaseId ) {
		VContext* context_ptr = cgroup->context_map[id];
		assert ( context_ptr );
		context_ptr->update_if_needed();
		descriptorSets.push_back ( context_ptr->descriptor_set );
	}
	if ( descriptorSets.size() ) {
		cmdbuffer.bindDescriptorSets ( vk::PipelineBindPoint::eGraphics, p_struct->pipeline_layout, descriptor_offset, descriptorSets, {} );
		descriptor_offset += descriptorSets.size();
	}
	cmdbuffer.bindPipeline ( vk::PipelineBindPoint::eGraphics, p_struct->pipelines[pipeline_index] );
	IdPtrArray<VModel>& models = v_instance->v_model_map[p_struct->modelbase_id];
	const ModelBase* modelbase_ptr = v_instance->modelbase ( p_struct->modelbase_id );
	for ( InstanceBlock& instanceblock : instanceblocks ) {
		if ( instanceblock.modelbase_id != p_struct->modelbase_id ) continue;
		if ( !instanceblock.count ) continue;
		VModel* v_model = models[instanceblock.model_id];

		//v_logger.log<LogLevel::eDebug> ( "Instance: 0x%x ModelBase: 0x%x Model-Index: 0x%x Offset: 0x%x Count: %d", instanceblock.base_id, instanceblock.modelbase_id, instanceblock.model_id, instanceblock.offset, instanceblock.count );
		//v_logger.log<LogLevel::eDebug> ( "Vertices: %d", v_model->indexcount );

		//TODO improve this as it is terribly bad
		//maybe sort the contexts? and then binary search?
		DynArray<vk::DescriptorSet> model_descriptorSets;
		for ( ContextBaseId id : p_struct->model_contextBaseId ) {
			bool found = false;
			for ( u32 i = 0; i < modelbase_ptr->contextbase_ids.size; i++ ) {
				if ( modelbase_ptr->contextbase_ids[i] == id ) {
					assert ( v_model->v_contexts[i] );
					v_model->v_contexts[i]->update_if_needed();
					model_descriptorSets.push_back ( v_model->v_contexts[i]->descriptor_set );
					found = true;
					break;
				}
			}
			assert ( found );
		}
		if ( model_descriptorSets.size() ) {
			cmdbuffer.bindDescriptorSets ( vk::PipelineBindPoint::eGraphics, p_struct->pipeline_layout, descriptor_offset, model_descriptorSets, {} );
		}

		cmdbuffer.bindIndexBuffer ( v_model->indexbuffer.buffer, 0, v_model->index_is_2byte ? vk::IndexType::eUint16 : vk::IndexType::eUint32 );

		if ( instanceblock.data )
			cmdbuffer.bindVertexBuffers ( 0, {v_model->vertexbuffer.buffer, igroup->buffer_storeage.buffer.buffer}, {0, instanceblock.offset} );
		else
			cmdbuffer.bindVertexBuffers ( 0, {v_model->vertexbuffer.buffer}, {0} );
		
		cmdbuffer.drawIndexed ( v_model->indexcount, instanceblock.count, 0, 0, 0 );
	}
}


VScaleDownRenderStage::VScaleDownRenderStage ( VInstance* v_instance ) : VRenderStage ( RenderStageType::eDownSample ), v_instance ( v_instance ) {
	v_bundlestates.resize ( 1 );
}
VScaleDownRenderStage::~VScaleDownRenderStage() {

}
void VScaleDownRenderStage::set_renderimage ( u32 index, Image* image, u32 miplayer, u32 arraylayer ) {
	assert ( index < v_bundlestates.size );
	VBundleImageState& imagestate = v_bundlestates[index];
	imagestate.actual_image = static_cast<VBaseImage*> ( image );
	imagestate.miplayer = miplayer;
	imagestate.arraylayer = arraylayer;
	if ( imagestate.actual_image->v_format != imagestate.current_format ) {
		imagestate.current_format = imagestate.actual_image->v_format;
	}
}
void VScaleDownRenderStage::set_renderwindow ( u32 index, Window* window ) {
	assert ( false );
}
void VScaleDownRenderStage::v_dispatch ( vk::CommandBuffer buffer, u32 index ) {
	VBundleImageState& imagestate = v_bundlestates[0];
	if ( imagestate.actual_image->mipmap_layers > 1 ) {
		imagestate.actual_image->generate_mipmaps ( vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, buffer );
	}
}
VCopyImageRenderStage::VCopyImageRenderStage ( VInstance* v_instance ) : VRenderStage ( RenderStageType::eCopyToScreen ), v_instance ( v_instance ) {
	v_bundlestates.resize ( 2 );
}
VCopyImageRenderStage::~VCopyImageRenderStage() {

}
void VCopyImageRenderStage::set_renderimage ( u32 index, Image* image, u32 miplayer, u32 arraylayer ) {
	assert ( index < v_bundlestates.size );
	VBundleImageState& imagestate = v_bundlestates[index];
	imagestate.actual_image = static_cast<VBaseImage*> ( image );
	imagestate.miplayer = miplayer;
	imagestate.arraylayer = arraylayer;
	if ( imagestate.actual_image->v_format != imagestate.current_format ) {
		imagestate.current_format = imagestate.actual_image->v_format;
	}
}
void VCopyImageRenderStage::set_renderwindow ( u32 index, Window* window ) {
	v_window = static_cast<VWindow*> ( window );
}
void VCopyImageRenderStage::v_dispatch ( vk::CommandBuffer buffer, u32 index ) {

	VBundleImageState& srcimagestate = v_bundlestates[0];
	VBundleImageState& dstimagestate = v_bundlestates[1];
	vk::Extent3D src_extent = srcimagestate.actual_image->extent,
	             dst_extent = dstimagestate.actual_image->extent;

	srcimagestate.actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal, buffer );
	dstimagestate.actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, buffer );
	vk::ImageBlit imageBlit ( vk::ImageSubresourceLayers ( vk::ImageAspectFlagBits::eColor, 0, 0, 1 ), {
		vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( src_extent.width, src_extent.height, src_extent.depth )
	}, vk::ImageSubresourceLayers ( vk::ImageAspectFlagBits::eColor, 0, 0, 1 ), {
		vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( dst_extent.width, dst_extent.height, dst_extent.depth )
	} );
	buffer.blitImage (
	    srcimagestate.actual_image->image, vk::ImageLayout::eTransferSrcOptimal,
	    dstimagestate.actual_image->image, vk::ImageLayout::eTransferDstOptimal,
	    1, &imageBlit, vk::Filter::eLinear
	);
}
VCopyToScreenRenderStage::VCopyToScreenRenderStage ( VInstance* v_instance ) : VRenderStage ( RenderStageType::eCopyToScreen ), v_instance ( v_instance ) {
	v_bundlestates.resize ( 1 );
}
VCopyToScreenRenderStage::~VCopyToScreenRenderStage() {

}
void VCopyToScreenRenderStage::set_renderimage ( u32 index, Image* image, u32 miplayer, u32 arraylayer ) {
	assert ( index < v_bundlestates.size );
	VBundleImageState& imagestate = v_bundlestates[index];
	imagestate.actual_image = static_cast<VBaseImage*> ( image );
	imagestate.miplayer = miplayer;
	imagestate.arraylayer = arraylayer;
	if ( imagestate.actual_image->v_format != imagestate.current_format ) {
		imagestate.current_format = imagestate.actual_image->v_format;
	}
}
void VCopyToScreenRenderStage::set_renderwindow ( u32 index, Window* window ) {
	v_window = static_cast<VWindow*> ( window );
}
void VCopyToScreenRenderStage::v_dispatch ( vk::CommandBuffer buffer, u32 index ) {

	VBundleImageState& imagestate = v_bundlestates[0];
	VBaseImage* window_image = v_window->present_images[v_window->present_image_index];
	vk::Extent3D window_extent = window_image->extent,
	             image_extent = imagestate.actual_image->extent;

	imagestate.actual_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal, buffer );
	window_image->transition_layout ( vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, buffer );
	vk::ImageBlit imageBlit ( vk::ImageSubresourceLayers ( vk::ImageAspectFlagBits::eColor, 0, 0, 1 ), {
		vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( image_extent.width, image_extent.height, image_extent.depth )
	}, vk::ImageSubresourceLayers ( vk::ImageAspectFlagBits::eColor, 0, 0, 1 ), {
		vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( window_extent.width, window_extent.height, window_extent.depth )
	} );
	buffer.blitImage (
	    imagestate.actual_image->image, vk::ImageLayout::eTransferSrcOptimal,
	    window_image->image, vk::ImageLayout::eTransferDstOptimal,
	    1, &imageBlit, vk::Filter::eLinear
	);
	window_image->transition_layout ( vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR, buffer );
}
VMainBundle::VMainBundle ( VInstance* instance ) :
	stages ( 11 ),
	dependencies(),
	window_dependency ( nullptr ),
	v_instance ( instance ),
	commandpool(),
	v_per_frame_data ( MAX_PRESENTIMAGE_COUNT ),
	last_used ( 0 ) {

	if ( !commandpool ) {
		vk::CommandPoolCreateInfo createInfo ( vk::CommandPoolCreateFlagBits::eResetCommandBuffer, v_instance->queues.graphics_queue_id );
		v_instance->vk_device ().createCommandPool ( &createInfo, nullptr, &commandpool );
	}
	stages[0] = new VShadowMapGeneration ( instance );
	stages[1] = new VShadowMapGeneration ( instance );
	stages[2] = new VShadowMapGeneration ( instance );
	stages[3] = new VMainRenderStage ( instance );
	stages[4] = new VBrightnessRenderStage ( instance );
	stages[5] = new VScaleDownRenderStage ( instance );
	stages[6] = new VBloomRenderStage ( instance );
	stages[7] = new HBloomRenderStage ( instance );
	stages[8] = new VFinalCompositionRenderStage ( instance );
	stages[9] = new VHDRRenderStage ( instance );
	stages[10] = new VCopyToScreenRenderStage ( instance );
}
VMainBundle::~VMainBundle() {
	v_instance->vk_device ().destroyCommandPool ( commandpool );
	commandpool = vk::CommandPool();
	for ( VRenderStage* renderstage : stages ) {
		delete renderstage;
	}
}

void VMainBundle::add_dependency ( u32 src_index, u32 dst_index ) {
	dependencies.push_back ( std::make_pair ( src_index, dst_index ) );
}
void VMainBundle::remove_dependency ( u32 src_index, u32 dst_index ) {
	for ( auto it = dependencies.begin(); it != dependencies.end(); it++ ) {
		if ( it->first == src_index && it->second == dst_index ) it = dependencies.erase ( it );
	}
}
void VMainBundle::set_window_dependency ( Window* window ) {
	window_dependency = static_cast<VWindow*> ( window );
}
void VMainBundle::clear_window_dependency( ) {
	window_dependency = nullptr;
}

void VMainBundle::set_renderstage ( u32 index, RenderStage* renderstage ) {
	//TODO
}
RenderStage* VMainBundle::get_renderstage ( u32 index ) {
	return stages[index];
}
RenderStage* VMainBundle::remove_renderstage ( u32 index ) {
	//TODO
	return nullptr;
}
void VMainBundle::v_dispatch ( ) {

	v_logger.log<LogLevel::eWarn> ( "--------------- FrameBoundary %d ---------------", v_instance->frame_index );

	u32 index = 0;
	if ( window_dependency ) {
		window_dependency->rendering_mutex.lock();
		window_dependency->prepare_frame();
		index = window_dependency->present_image_index;
		v_instance->prepare_frame();
	}

	QueueWrapper* queue_wrapper = &v_instance->queues;

	vk::PipelineStageFlags waitDstStageMask ( vk::PipelineStageFlagBits::eBottomOfPipe );

	PerFrameRenderObj& data = v_per_frame_data[index];
	if ( data.command.should_reset ) {
		if ( data.command.buffer ) {
			data.command.buffer.reset ( vk::CommandBufferResetFlags() );
		} else {
			data.command.buffer = v_instance->createCommandBuffer ( commandpool, vk::CommandBufferLevel::ePrimary );
		}
		vk::CommandBufferBeginInfo begininfo = {
			vk::CommandBufferUsageFlags(), nullptr
		};
		data.command.buffer.begin ( begininfo );

		for ( VRenderStage* renderstage : stages ) {
			renderstage->v_dispatch ( data.command.buffer, index );
		}

		data.command.buffer.end();
		//data.command.should_reset = false;
	}

	vk::SubmitInfo submitinfos[1] = { vk::SubmitInfo (
	                                      1, &window_dependency->image_available_guard_sem, //waitSem
	                                      &waitDstStageMask,
	                                      1, &data.command.buffer,//commandbuffers
	                                      1, &window_dependency->current_framelocal_data()->present_ready_sem//signalSem
	                                  )
	                                };

	queue_wrapper->graphics_queue.submit ( 1, submitinfos, window_dependency->current_framelocal_data()->image_presented_fence );

	if ( !queue_wrapper->combined_graphics_present_queue ) {
		//@TODO synchronize
		assert ( false );
	}
	//present image
	vk::Result results;
	vk::PresentInfoKHR presentInfo (
	    1, &window_dependency->current_framelocal_data()->present_ready_sem,
	    //active_sems.size(), active_sems.data(),
	    1, &window_dependency->swap_chain,
	    &index, &results );
	queue_wrapper->present_queue.presentKHR ( &presentInfo );
	last_used = v_instance->frame_index;

	v_instance->frame_index++;
	window_dependency->rendering_mutex.unlock();
	v_logger.log<LogLevel::eWarn> ( "---------------     End Frame    ---------------" );
}
