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

struct RendererBase {
	RId id;
	ModelInstanceBase* model_instance_base;
	Array<ContextBase*> context_bases;
	//GBuffer input
	//	id of previous generated image
	//Array outputdef
	//	Transform Feedback Opengl
	//Image outputdef
	//	Normal Rendering
	//shader definitions and stuff
};
struct RenderStageBase{
	RId id;
	Array<RendererBase*> renderers;
	//Dependencies
	//	Renderer*/index -> Renderer*/index
};
struct Renderer {
	RendererBase* renderer_base;
	//changeable stuff here like options?
};
struct RenderStage {
	RenderStageBase render_stage_base;
	Array<Renderer*> renderers;
	//Dependencies
	//	Renderer*/index -> Renderer*/index
};

struct RenderInformation{
	
};
//triangle/line/points mode
//scissors?
//viewports optimizations with geometry shader for multiple screens
//sample-shading???
