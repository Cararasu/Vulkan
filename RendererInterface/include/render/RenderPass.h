
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
	ModelInstanceBase* model_instance_base;
	Array<ContextBase*> context_bases;
	//shader definitions and stuff
};
struct Renderer {
	//changeable stuff here?
};
struct RenderStage{
	Array<Renderer*> renderers;
};

struct RenderPass {
	Array<RenderStage*> render_stages;
};
struct RenderInformation{
	
};
//triangle/line/points mode
//scissors?
//viewports optimizations with geometry shader for multiple screens
//sample-shading???
