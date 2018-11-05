#include "VRenderer.h"

VRenderStage::VRenderStage(Array<Renderer*>& renderers, Array<RenderImageDef> image_defs) : renderers(renderers.size), image_defs(image_defs){
	for(u32 i = 0; i < renderers.size; i++) {
		this->renderers[i] = dynamic_cast<VRenderer*>(renderers[i]);
	}
}
VRenderStage::VRenderStage(Array<Renderer*>&& renderers, Array<RenderImageDef> image_defs) : renderers(renderers.size), image_defs(image_defs){
	for(u32 i = 0; i < renderers.size; i++) {
		this->renderers[i] = dynamic_cast<VRenderer*>(renderers[i]);
	}
}