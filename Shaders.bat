@echo off
@echo Executing Pre Build commands ...
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/quad.vert -o ../workingdir/shader/quad.vert.sprv
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/quad.frag -o ../workingdir/shader/quad.frag.sprv
@
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/textured.vert -o ../workingdir/shader/textured.vert.sprv
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/textured.frag -o ../workingdir/shader/textured.frag.sprv
@
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/flat.vert -o ../workingdir/shader/flat.vert.sprv
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/flat.frag -o ../workingdir/shader/flat.frag.sprv
@
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/skybox.vert -o ../workingdir/shader/skybox.vert.sprv
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/skybox.frag -o ../workingdir/shader/skybox.frag.sprv
@
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/shot.vert -o ../workingdir/shader/shot.vert.sprv
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/shot.frag -o ../workingdir/shader/shot.frag.sprv
@
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/shotlight.vert -o ../workingdir/shader/shotlight.vert.sprv
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/shotlight.frag -o ../workingdir/shader/shotlight.frag.sprv
@
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/billboard.vert -o ../workingdir/shader/billboard.vert.sprv
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/billboard.frag -o ../workingdir/shader/billboard.frag.sprv
@
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/engine.frag -o ../workingdir/shader/engine.frag.sprv
@
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/passthrough.vert -o ../workingdir/shader/passthrough.vert.sprv
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/null.vert -o ../workingdir/shader/null.vert.sprv
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/lightless.frag -o ../workingdir/shader/lightless.frag.sprv
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/dirlight.frag -o ../workingdir/shader/dirlight.frag.sprv
@
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/brightness.frag -o ../workingdir/shader/brightness.frag.sprv
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/hbloom.frag -o ../workingdir/shader/hbloom.frag.sprv
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/vbloom.frag -o ../workingdir/shader/vbloom.frag.sprv
@
@$(VULKAN_PATH)/bin/glslc ../workingdir/shader/composition.frag -o ../workingdir/shader/composition.frag.sprv
@
@echo Done
make
