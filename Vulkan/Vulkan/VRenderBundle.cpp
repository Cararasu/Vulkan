#include "VRenderBundle.h"
#include "VRenderer.h"

VRenderBundle::VRenderBundle(InstanceGroup* igroup, ContextGroup* cgroup, const RenderStage* rstage, Array<Image*>& targets) :
	igroup(igroup), cgroup(cgroup), rstage(dynamic_cast<const VRenderStage*>(rstage)), targets(targets){
	
}

VRenderBundle::~VRenderBundle() {
	
}

void VRenderBundle::v_dispatch() {
	printf("Dispatch\n");
}