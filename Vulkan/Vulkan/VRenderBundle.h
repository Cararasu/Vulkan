#pragma once

#include "render/Model.h"
struct VRenderStage;
struct RenderStage;
struct Image;

struct VRenderBundle : public RenderBundle {
	InstanceGroup* igroup;
	ContextGroup* cgroup;
	const VRenderStage* rstage;
	Array<Image*> targets;
	
	VRenderBundle(InstanceGroup* igroup, ContextGroup* cgroup, const RenderStage* rstage, Array<Image*>& targets);
	virtual ~VRenderBundle();

	void v_dispatch();
	
};
