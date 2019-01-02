#include "VRenderBundle.h"
#include "VRenderer.h"

VRenderBundle::VRenderBundle ( InstanceGroup* arg_igroup, ContextGroup* arg_cgroup, Array<const RenderStage*>& arg_rstages, Array<ImageType>& arg_image_types, Array<ImageDependency>& arg_dependencies ) :
	igroup ( dynamic_cast<VInstanceGroup*> ( arg_igroup ) ), cgroup ( dynamic_cast<VContextGroup*> ( arg_cgroup ) ),
	rstages ( arg_rstages.size ), image_types ( arg_image_types ), dependencies ( arg_dependencies ) {
	for ( u32 i = 0; i < arg_rstages.size; i++ ) {
		this->rstages[i] = dynamic_cast<const VRenderStage*> ( arg_rstages[i] );
	}
}

VRenderBundle::~VRenderBundle() {

}
void VRenderBundle::v_dispatch() {
	
}
