#ifndef VWINDOW_H
#define VWINDOW_H

#include <vulkan/vulkan.h>
#include "VHeader.h"
#include "VDevice.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define V_MAX_PRESENTIMAGE_COUNT (3)

struct VWindow{
	GLFWwindow* window;
	VPGCQueue* pgcQueue;
	VkExtent2D windowExtend;
	VkExtent2D swapChainExtend;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	bool needPresentGuard;
	VkSemaphore imageAvailableGuardSem;
	
	VkSurfaceFormatKHR presentSwapFormat;
	uint32_t presentImageIndex;
	std::vector<VkImageView> presentImages;
	
	void initializeWindow();
	
	void showNextImage(uint32_t waitSemaphoreCount = 0, const VkSemaphore* pWaitSemaphores = nullptr);
};

#endif // VWINDOW_H
