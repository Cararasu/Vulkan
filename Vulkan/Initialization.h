
#include <render/Instance.h>
#include "GameState.h"

void register_shaders (Instance* instance);
RenderBundle* setup_renderbundle (Instance* instance, Window* window, InstanceGroup* instancegroup, ContextGroup* contextgroup);
void setup_camerapoints (GameState* gamestate);