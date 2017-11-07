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
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	VkSemaphore imageAvailableGuardSem = VK_NULL_HANDLE;
	
	VkSurfaceFormatKHR presentSwapFormat;
	uint32_t presentImageIndex;
	std::vector<VkImageView> presentImages;
	
	~VWindow();
	
	void initializeWindow();
	
	void showNextImage(uint32_t waitSemaphoreCount = 0, const vk::Semaphore* pWaitSemaphores = nullptr);
	
	bool isOpen(){
		return !glfwWindowShouldClose(window);
	}
};

#endif // VWINDOW_H
