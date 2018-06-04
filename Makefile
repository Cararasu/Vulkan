.PHONY: clean All

All:
	@echo "----------Building project:[ MetaBuilder - Debug ]----------"
	@cd "MetaBuilder" && "$(MAKE)" -f  "MetaBuilder.mk"
	@echo "----------Building project:[ Vulkan - Debug Windows ]----------"
	@cd "Vulkan" && "$(MAKE)" -f  "Vulkan.mk"
clean:
	@echo "----------Cleaning project:[ MetaBuilder - Debug ]----------"
	@cd "MetaBuilder" && "$(MAKE)" -f  "MetaBuilder.mk"  clean
	@echo "----------Cleaning project:[ Vulkan - Debug Windows ]----------"
	@cd "Vulkan" && "$(MAKE)" -f  "Vulkan.mk" clean
