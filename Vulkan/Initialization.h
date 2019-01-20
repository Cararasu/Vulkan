
#include <render/Instance.h>
#include "GameState.h"

struct World;

void register_shaders (Instance* instance);
RenderBundle* setup_renderbundle (Instance* instance, Window* window, World* world, Array<Context>& shadowmap_cameras);
void setup_camerapoints (GameState* gamestate);