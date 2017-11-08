#ifndef VWINDOW_H
#define VWINDOW_H

#include <vulkan/vulkan.h>
#include "VHeader.h"
#include "VDevice.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define V_MAX_PRESENTIMAGE_COUNT (3)

struct WindowPerPresentImageData{
	vk::Image presentImage;
	vk::ImageView presentImageView;
	vk::Framebuffer framebuffer;
	bool firstShow = true;
	
	vk::Fence fence;
	vk::CommandPool graphicQCommandPool;
};

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
	
	std::vector<WindowPerPresentImageData> perPresentImageDatas;
	
	~VWindow();
	
	void initializeWindow();
	
	void showNextImage(uint32_t waitSemaphoreCount = 0, const vk::Semaphore* pWaitSemaphores = nullptr);
	
	bool isOpen(){
		return !glfwWindowShouldClose(window);
	}
	
	
	vk::CommandPool getCurrentGraphicsCommandPool(){
		return perPresentImageDatas[presentImageIndex].graphicQCommandPool;
	}
};

#endif // VWINDOW_H
