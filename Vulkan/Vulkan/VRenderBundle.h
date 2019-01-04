#pragma once

#include "VModel.h"
#include "VContext.h"
#include <render/RenderPass.h>

struct VRenderStage;
struct RenderStage;
struct Image;
struct VBaseImage;
struct InstanceGroup;
struct ContextGroup;

struct VRenderImageWrapper {
	bool external = false;
	VBaseImage* baseimage = nullptr;
};

struct VRenderBundle : public RenderBundle {
	VInstanceGroup* igroup;
	VContextGroup* cgroup;
	Array<const VRenderStage*> rstages;
	Array<ImageUsage> image_types;
	Array<ImageDependency> dependencies;

	VRenderBundle ( InstanceGroup* igroup, ContextGroup* cgroup, Array<const RenderStage*>& rstages, Array<ImageUsage>& image_types, Array<ImageDependency>& dependencies );
	virtual ~VRenderBundle();

	void v_dispatch();

};
