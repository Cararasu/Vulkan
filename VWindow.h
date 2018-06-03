#ifndef VWINDOW_H
#define VWINDOW_H

#include <vulkan/vulkan.h>
#include "VHeader.h"
#include "VDevice.h"
#include "PipelineModule.h"

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
enum class WindowState{
	eUninitialized,
	eInitialized,
	eFramePrepared,
	eFramePresented,
	eNotVisible,
	eResized
};
struct VWindow{
	VInstance* instance;
	GLFWwindow* window;
	VPGCQueue* pgcQueue;
	vk::PresentModeKHR chosenPresentationMode;
	vk::SurfaceCapabilitiesKHR capabilities;
	vk::Extent2D swapChainExtend;
	vk::SurfaceKHR surface;
	vk::SwapchainKHR swapChain;
	vk::Semaphore imageAvailableGuardSem;
	
	WindowState windowState = WindowState::eUninitialized;
	
	vk::ImageView depthImageView;
	ImageWrapper *depthImage = nullptr;
	
	vk::SurfaceFormatKHR presentSwapFormat;
	uint32_t presentImageIndex;
	
	PipelineModule standardmodule;
	
	std::vector<WindowPerPresentImageData> perPresentImageDatas;
	
	~VWindow();
	
	void initializeWindow(VInstance* instance);
	
	void createSwapchain();
	
	void setupFrame();
	
	void showNextFrame(uint32_t waitSemaphoreCount = 0, const vk::Semaphore* pWaitSemaphores = nullptr);
	
	void handleResize(uint32_t width, uint32_t height);
	
	bool isOpen(){
		return !glfwWindowShouldClose(window);
	}
	
	
	vk::CommandPool getCurrentGraphicsCommandPool(){
		return perPresentImageDatas[presentImageIndex].graphicQCommandPool;
	}
};

#endif // VWINDOW_H
