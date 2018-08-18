
#include "Resources.h"

struct RenderPassInput {
	RId renderpass_id;
	u32 index;
	bool only_direct_pixel_access;
};
struct RenderPassOutput {
	ImageFormat format;
	//multisample
};

struct RenderPassDef {
	RId modelbase_id;
	Array<RenderPassInput> inputs;
	Array<RenderPassOutput> outputs;
	
	RenderPassOutput depth_stencil;
	bool depthtest, depthwrite, stenciltest;
	f32 depthbias;
	
};
struct RenderPass : public Resource {
	const RenderPassDef def;
	
	virtual RenderPass* cast_to_renderpass() {
		return this;
	}
};
//triangle/line/points mode
//scissors?
//viewports optimizations with geometry shader for multiple screens
//sample-shading???
