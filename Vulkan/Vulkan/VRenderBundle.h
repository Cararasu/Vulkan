#pragma once

#include "VModel.h"
#include "VContext.h"

struct VRenderStage;
struct RenderStage;
struct Image;

struct VRenderImageWrapper {
	bool external = false;
	VBaseImage* baseimage = nullptr;
};

struct VRenderBundle : public RenderBundle {
	VInstanceGroup* igroup;
	VContextGroup* cgroup;
	const VRenderStage* rstage;
	
	Array<VRenderImageWrapper> rendertargets;
	
	VRenderBundle(InstanceGroup* igroup, ContextGroup* cgroup, const RenderStage* rstage);
	virtual ~VRenderBundle();

	virtual void set_rendertarget(u32 index, Image* image) override;
	
	void v_dispatch();
	
};
