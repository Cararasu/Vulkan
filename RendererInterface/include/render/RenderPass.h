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
};
struct RenderStage : public IdHandle {
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
