#include "VRenderBundle.h"
#include "VRenderer.h"

VRenderBundle::VRenderBundle ( InstanceGroup* arg_igroup, ContextGroup* arg_cgroup, Array<const RenderStage*>& arg_rstages, Array<ImageType>& arg_image_types, Array<ImageDependency>& arg_dependencies ) :
	igroup ( dynamic_cast<VInstanceGroup*> ( arg_igroup ) ), cgroup ( dynamic_cast<VContextGroup*> ( arg_cgroup ) ),
	rstages ( arg_rstages.size ), image_types ( arg_image_types ), dependencies ( arg_dependencies ), rendertargets ( arg_image_types.size ) {
	for ( u32 i = 0; i < arg_rstages.size; i++ ) {
		printf ( "%p\n", arg_rstages[i] );
		this->rstages[i] = dynamic_cast<const VRenderStage*> ( arg_rstages[i] );
	}
}

VRenderBundle::~VRenderBundle() {

}
void VRenderBundle::set_rendertarget ( u32 index, Image* image ) {
	rendertargets[index].external = true;
	rendertargets[index].baseimage = dynamic_cast<VBaseImage*> ( image );
}

void VRenderBundle::v_dispatch() {
	printf ( "Dispatch\n" );
	printf ( "%" PRId64 " instancetypes\n", igroup->instance_to_data_map.size() );
	printf ( "%" PRId64 " contexts\n", cgroup->context_map.size() );
	printf ( "%" PRId64 " stages\n", rstages.size );
}
