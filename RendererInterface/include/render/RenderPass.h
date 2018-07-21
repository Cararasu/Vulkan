
#include "Resource.h"

struct RenderPass {
	RId modelbase_id;


};

struct RenderPassInput {
	ResourceHandle renderpass_handle;
	u32 index;
	bool only_direct_pixel_access;
};
struct RenderPassOutput {
	ImageType type;
};

struct RenderPass {
	ResourceHandle modelbase_handle;
	ResourceHandle instancebase_handle;
	Array<RenderPassInput> inputs;
	Array<RenderPassOutput> outputs;
};
