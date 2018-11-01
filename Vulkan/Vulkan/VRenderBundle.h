#pragma once

#include "VModel.h"
#include "VContext.h"

struct VRenderStage;
struct RenderStage;
struct Image;

struct VRenderBundle : public RenderBundle {
	VInstanceGroup* igroup;
	VContextGroup* cgroup;
	const VRenderStage* rstage;
	
	VRenderBundle(InstanceGroup* igroup, ContextGroup* cgroup, const RenderStage* rstage);
	virtual ~VRenderBundle();

	void v_dispatch();
	
};
