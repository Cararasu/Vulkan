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
	Array<const VRenderStage*> rstages;
	Array<ImageType> image_types;
	Array<ImageDependency> dependencies;
	
	Array<VRenderImageWrapper> rendertargets;
	
	VRenderBundle(InstanceGroup* igroup, ContextGroup* cgroup, Array<const RenderStage*>& rstages, Array<ImageType>& image_types, Array<ImageDependency>& dependencies);
	virtual ~VRenderBundle();

	virtual void set_rendertarget(u32 index, Image* image) override;
	
	void v_dispatch();
	
};
