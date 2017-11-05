#include "VWindow.h"
#include "VGlobal.h"
#include <limits>
#include <algorithm>


void VWindow::initializeWindow(){
	
	pgcQueue = vGlobal.deviceWrapper.requestPGCQueue();
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(640, 640, "Vulkan Test", NULL, NULL);
	VCHECKCALL(glfwCreateWindowSurface(vGlobal.vkinstance, window, NULL, &surface), {
		printf("Creation of Surface failed");
	});
	
	imageAvailableGuardSem = createSemaphore(vGlobal.deviceWrapper.device);
	
	{
		{
			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(vGlobal.physicalDevice, surface, &formatCount, nullptr);
			if (formatCount != 0) {
				VkSurfaceFormatKHR formats[formatCount];
				vkGetPhysicalDeviceSurfaceFormatsKHR(vGlobal.physicalDevice, surface, &formatCount, formats);
				
				presentSwapFormat = formats[0];
				
				if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
					presentSwapFormat = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
				}
				for (size_t i = 0; i < formatCount; i++) {
					if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
						presentSwapFormat = formats[i];
						break;
					}
				}
			}
		}
		VkPresentModeKHR chosenPresentationMode = VK_PRESENT_MODE_FIFO_KHR;//can be turned into global flags of what is supported
		{
			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(vGlobal.physicalDevice, surface, &presentModeCount, nullptr);
			if (presentModeCount != 0) {
				VkPresentModeKHR presentModes[presentModeCount];
				vkGetPhysicalDeviceSurfacePresentModesKHR(vGlobal.physicalDevice, surface, &presentModeCount, presentModes);
				chosenPresentationMode = VK_PRESENT_MODE_FIFO_KHR;
				for (size_t i = 0; i < presentModeCount; i++) {
					if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
						chosenPresentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
						break;
					}else if(presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR){
						chosenPresentationMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
					}
				}
			}
		}
		VkSurfaceCapabilitiesKHR capabilities;//can be saved globally for each physicaldevice
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vGlobal.physicalDevice, surface, &capabilities);
		{
			int width, height;
			glfwGetWindowSize(window, &width , &height);
			VkExtent2D actualExtent = {width, height};
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
				swapChainExtend = capabilities.currentExtent;
			} else {

				actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
				
				swapChainExtend = actualExtent;
			}
		}
		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
			imageCount = std::max<uint32_t>(capabilities.maxImageCount, V_MAX_PRESENTIMAGE_COUNT);
		}
		VkSwapchainCreateInfoKHR swapchainCreateInfo;
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.pNext = nullptr;
		swapchainCreateInfo.flags = 0;
		swapchainCreateInfo.surface = surface;
		swapchainCreateInfo.minImageCount = imageCount;
		swapchainCreateInfo.imageFormat = presentSwapFormat.format;
		swapchainCreateInfo.imageColorSpace = presentSwapFormat.colorSpace;
		swapchainCreateInfo.imageExtent = swapChainExtend;
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if (pgcQueue->combinedPGQ) {
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCreateInfo.queueFamilyIndexCount = 1; // Optional
			uint32_t queueFamilyIndices[] = {pgcQueue->presentQId};
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices; // Optional
		} else {
			swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			uint32_t queueFamilyIndices[] = {pgcQueue->presentQId, pgcQueue->graphicsQId};
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		swapchainCreateInfo.preTransform = capabilities.currentTransform;//VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR??
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//awesome blending with other windows of os
		swapchainCreateInfo.presentMode = chosenPresentationMode;
		swapchainCreateInfo.clipped = VK_TRUE;//clip pixels that are behind other windows
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
		
		VkBool32 supported;
		VCHECKCALL(vkGetPhysicalDeviceSurfaceSupportKHR(vGlobal.physicalDevice, pgcQueue->presentQId, surface, &supported), {
			printf("Surface Not supported");
		});
		printf("SUPPORTED %d\n", supported);
		VCHECKCALL(vkCreateSwapchainKHR(vGlobal.deviceWrapper.device, &swapchainCreateInfo, nullptr, &swapChain), {
			printf("Creation of Swapchain failed\n");
		});
	}
	uint32_t swapImageCount;
	vkGetSwapchainImagesKHR(vGlobal.deviceWrapper.device, swapChain, &swapImageCount, nullptr);
	printf("SwapImageCount %d\n", swapImageCount);
	VkImage swapChainImages[swapImageCount];
	vkGetSwapchainImagesKHR(vGlobal.deviceWrapper.device, swapChain, &swapImageCount, swapChainImages);
	presentImages.resize(swapImageCount);
	
	for(size_t i = 0; i < swapImageCount; i++){
		presentImages[i] = createImageView2D(swapChainImages[i], presentSwapFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
	}
	vkAcquireNextImageKHR(vGlobal.deviceWrapper.device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableGuardSem, VK_NULL_HANDLE, &presentImageIndex);
}
void VWindow::showNextImage(uint32_t waitSemaphoreCount, const VkSemaphore* pWaitSemaphores){
	
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = waitSemaphoreCount;
	presentInfo.pWaitSemaphores = pWaitSemaphores;
	presentInfo.swapchainCount = 1;
	VkSwapchainKHR swapChains[] = {swapChain};
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &presentImageIndex;
	presentInfo.pResults = nullptr; // Optional
	
	vkQueuePresentKHR(pgcQueue->presentQueue, &presentInfo);
	
	vkAcquireNextImageKHR(vGlobal.deviceWrapper.device, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableGuardSem, VK_NULL_HANDLE, &presentImageIndex);
}
VWindow::~VWindow(){
	destroySemaphore(vGlobal.deviceWrapper.device, imageAvailableGuardSem);
}