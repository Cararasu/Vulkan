#pragma once

#include "Resources.h"

//Renderer - defines one GPU pipeline
//	ContextBase - id
//	ModelInstanceBase - id
//RenderStage - groups multiple renderers that can be done simultaniously
//	RenderStages
//RenderPass - groups sections that can be used in a deferred shading pass
//	RenderSections
//RenderInformation - list of renderpasses that are executed simultaneously
//	RenderPasses

//ShaderDef
//create_shader(opengl3, opengl4, vulkan, direct3d, ...)

struct ShaderBase {
	
};

struct Renderer : public IdHandle {
	const ModelInstanceBase* model_instance_base;
	const Array<const ContextBase*> contexts;
	//changeable stuff here like options?
	Renderer(const ModelInstanceBase* model_instance_base, const Array<const ContextBase*> contexts) : model_instance_base(model_instance_base), contexts(contexts){}
	virtual ~Renderer() {}
};
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
	
	//void set_output_image();
};

struct RenderInformation{
	
};
//triangle/line/points mode
//scissors?
//viewports optimizations with geometry shader for multiple screens
//sample-shading???
