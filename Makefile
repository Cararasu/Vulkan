.PHONY: clean All

All:
	@echo "----------Building project:[ Vulkan - Debug Windows ]----------"
	@cd "Vulkan" && "$(MAKE)" -f  "Vulkan.mk"
clean:
	@echo "----------Cleaning project:[ Vulkan - Debug Windows ]----------"
	@cd "Vulkan" && "$(MAKE)" -f  "Vulkan.mk" clean
