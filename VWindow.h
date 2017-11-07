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
	vk::Extent2D windowExtend;
	vk::Extent2D swapChainExtend;
	vk::SurfaceKHR surface = vk::SurfaceKHR();
	vk::SwapchainKHR swapChain = vk::SwapchainKHR();
	vk::Semaphore imageAvailableGuardSem = vk::Semaphore();
	
	vk::SurfaceFormatKHR presentSwapFormat;
	uint32_t presentImageIndex;
	std::vector<vk::ImageView> presentImages;
	std::vector<vk::Framebuffer> framebuffers;
	
	std::vector<vk::CommandPool> tranferQCommandPools;
	std::vector<vk::CommandPool> graphicQCommandPools;
	
	~VWindow();
	
	void initializeWindow();
	
	void showNextImage(uint32_t waitSemaphoreCount = 0, const vk::Semaphore* pWaitSemaphores = nullptr);
	
	bool isOpen(){
		return !glfwWindowShouldClose(window);
	}
};

#endif // VWINDOW_H
