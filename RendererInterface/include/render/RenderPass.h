
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

struct RendererBase {
	//shader definitions and stuff
};
struct Renderer {
	ModelInstanceBase* model_instance_base;
	u32 context_base_count;
	ContextBase* context_bases;
	//and other changable stuff
};
struct RenderStage{
	u32 renderer_count;
	Renderer* renderers;
};

struct RenderPass {
	u32 render_stage_count;
	Renderer* render_stages;
};
struct RenderInformation{
	
};
//triangle/line/points mode
//scissors?
//viewports optimizations with geometry shader for multiple screens
//sample-shading???
