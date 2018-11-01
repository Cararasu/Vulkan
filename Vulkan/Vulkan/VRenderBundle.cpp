#include "VRenderBundle.h"
#include "VRenderer.h"

VRenderBundle::VRenderBundle(InstanceGroup* igroup, ContextGroup* cgroup, const RenderStage* rstage) :
	igroup(dynamic_cast<VInstanceGroup*>(igroup)), cgroup(dynamic_cast<VContextGroup*>(cgroup)), rstage(dynamic_cast<const VRenderStage*>(rstage)) {
	
}

VRenderBundle::~VRenderBundle() {
	
}

void VRenderBundle::v_dispatch() {
	printf("Dispatch\n");
	printf("%d instancetypes\n", igroup->base_to_id_map.size());
	printf("%d contexts\n", cgroup->context_map.size());
	printf("%d renderers\n", rstage->renderers.size);
}