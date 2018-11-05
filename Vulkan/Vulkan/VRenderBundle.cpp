#include "VRenderBundle.h"
#include "VRenderer.h"

VRenderBundle::VRenderBundle(InstanceGroup* igroup, ContextGroup* cgroup, const RenderStage* rstage) :
	igroup(dynamic_cast<VInstanceGroup*>(igroup)), cgroup(dynamic_cast<VContextGroup*>(cgroup)), rstage(dynamic_cast<const VRenderStage*>(rstage)), rendertargets(this->rstage->image_defs.size) {
	
}

VRenderBundle::~VRenderBundle() {
	
}
void VRenderBundle::set_rendertarget(u32 index, Image* image) {
	rendertargets[index].external = true;
	rendertargets[index].baseimage = dynamic_cast<VBaseImage*>(image);
}

void VRenderBundle::v_dispatch() {
	printf("Dispatch\n");
	printf("%d instancetypes\n", igroup->base_to_id_map.size());
	printf("%d contexts\n", cgroup->context_map.size());
	printf("%d renderers\n", rstage->renderers.size);
}