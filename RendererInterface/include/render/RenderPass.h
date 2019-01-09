#pragma once

#include "Resources.h"

//RenderStage - ...
//	RenderStages
//RenderPass - groups sections that can be used in a deferred shading pass
//	RenderSections
//RenderInformation - list of renderpasses that are executed simultaneously
//	RenderPasses

//ShaderDef
//create_shader(opengl3, opengl4, vulkan, direct3d, ...)

enum class ImageUsage {
	eContextRead = 0x1,
	eRead = 0x2,
	eWrite = 0x3,
	eReadWrite = 0x4,
};

struct StageImage {
	u32 stage_index, image_index;
};
struct ImageDependency {
	u32 imageindex;
	StageImage source_image;
	StageImage target_image;
};

struct Window;

enum class RenderStageType {
	eRendering,
	eDownSample,
	eCopyToScreen,
};
struct RenderStage {

	const RenderStageType type;

	RenderStage ( RenderStageType type ) : type ( type ) {}
	virtual ~RenderStage () {}

	virtual void set_renderimage ( u32 index, Image* image, u32 miplayer = 0, u32 arraylayer = 0 ) = 0;
	virtual void set_renderwindow ( u32 index, Window* window ) = 0;
	virtual void set_contextgroup ( ContextGroup* contextgroup ) = 0;
};
struct RenderBundle {

	virtual ~RenderBundle() {}
	
	virtual void add_dependency( u32 src_index, u32 dst_index ) = 0;
	virtual void remove_dependency( u32 src_index, u32 dst_index ) = 0;

	virtual void set_window_dependency( Window* window ) = 0;
	virtual void clear_window_dependency( ) = 0;
	
	virtual void set_renderstage ( u32 index, RenderStage* renderstage ) = 0;
	virtual RenderStage* get_renderstage ( u32 index ) = 0;
	virtual RenderStage* remove_renderstage ( u32 index ) = 0;
};