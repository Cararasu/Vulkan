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

struct RenderImageDef {
	ImageType type;
	ImageUsage usage;
};
struct StageImage {
	u32 stage_index, image_index;
};
struct ImageDependency {
	u32 imageindex;
	StageImage source_image;
	StageImage target_image;
};

enum class RenderStageType {
	eRenderer,
	ePresenter,
	eComposer,
};

struct RenderStage : public IdHandle {
	RenderStageType type;
	
	virtual ~RenderStage() {}
	//inputs
	//outouts
	//Dependencies		Renderer*/index -> Renderer*/index
};

struct RenderInformation{
	
};
//triangle/line/points mode
//scissors?
//viewports optimizations with geometry shader for multiple screens
//sample-shading???
