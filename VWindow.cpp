#include "VWindow.h"
#include "VGlobal.h"
#include <limits>
#include <algorithm>


void VWindow::initializeWindow(){
	
	pgcQueue = global.deviceWrapper.requestPGCQueue();
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(640, 640, "Vulkan Test", NULL, NULL);
	VCHECKCALL(glfwCreateWindowSurface(global.vkinstance, window, NULL, (VkSurfaceKHR*)&surface), printf("Creation of Surface failed"));
	
	imageAvailableGuardSem = createSemaphore();
	
	{
		{
			uint32_t formatCount;
			global.physicalDevice.getSurfaceFormatsKHR(surface, &formatCount, nullptr);
			if (formatCount != 0) {
				vk::SurfaceFormatKHR formats[formatCount];
				global.physicalDevice.getSurfaceFormatsKHR(surface, &formatCount, formats);
				
				presentSwapFormat = formats[0];
				
				if (formatCount == 1 && formats[0].format == vk::Format::eUndefined) {
					presentSwapFormat = {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
				}
				for (size_t i = 0; i < formatCount; i++) {
					if (formats[i].format == vk::Format::eB8G8R8A8Unorm && formats[i].colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
						presentSwapFormat = formats[i];
						break;
					}
				}
			}
		}
		vk::PresentModeKHR chosenPresentationMode = vk::PresentModeKHR::eFifo;//can be turned into global flags of what is supported
		{
			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(global.physicalDevice, surface, &presentModeCount, nullptr);
			if (presentModeCount != 0) {
				vk::PresentModeKHR presentModes[presentModeCount];
				global.physicalDevice.getSurfacePresentModesKHR(surface, &presentModeCount, presentModes);
				chosenPresentationMode = vk::PresentModeKHR::eFifo;
				for (size_t i = 0; i < presentModeCount; i++) {
					if (presentModes[i] == vk::PresentModeKHR::eMailbox) {
						chosenPresentationMode = vk::PresentModeKHR::eMailbox;
						break;
					}else if(presentModes[i] == vk::PresentModeKHR::eImmediate){
						chosenPresentationMode = vk::PresentModeKHR::eImmediate;
					}
				}
			}
		}
		vk::SurfaceCapabilitiesKHR capabilities;//can be saved globally for each physicaldevice
		global.physicalDevice.getSurfaceCapabilitiesKHR(surface, &capabilities);
		{
			int width, height;
			glfwGetWindowSize(window, &width , &height);
			vk::Extent2D actualExtent = {width, height};
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
		vk::SwapchainCreateInfoKHR swapchainCreateInfo(vk::SwapchainCreateFlagsKHR(), surface, imageCount, presentSwapFormat.format, presentSwapFormat.colorSpace, swapChainExtend, 1, vk::ImageUsageFlagBits::eColorAttachment);

		if (pgcQueue->combinedPGQ) {
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
			swapchainCreateInfo.queueFamilyIndexCount = 1; // Optional
			uint32_t queueFamilyIndices[] = {pgcQueue->presentQId};
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices; // Optional
		} else {
			swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchainCreateInfo.queueFamilyIndexCount = 2;
			uint32_t queueFamilyIndices[] = {pgcQueue->presentQId, pgcQueue->graphicsQId};
			swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		swapchainCreateInfo.preTransform = capabilities.currentTransform;//VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR??
		swapchainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;//awesome blending with other windows of os
		swapchainCreateInfo.presentMode = chosenPresentationMode;
		swapchainCreateInfo.clipped = VK_TRUE;//clip pixels that are behind other windows
		swapchainCreateInfo.oldSwapchain = vk::SwapchainKHR();
		
		printf("SUPPORTED %d\n", global.physicalDevice.getSurfaceSupportKHR(pgcQueue->presentQId, surface));
		
		V_CHECKCALL(global.deviceWrapper.device.createSwapchainKHR(&swapchainCreateInfo, nullptr, &swapChain), printf("Creation of Swapchain failed\n"));
	}
	std::vector<vk::Image> swapChainImages = global.deviceWrapper.device.getSwapchainImagesKHR(swapChain);
	
	perPresentImageDatas.resize(swapChainImages.size());
	
	for(size_t i = 0; i < swapChainImages.size(); i++){
		perPresentImageDatas[i].presentImage = swapChainImages[i];
		perPresentImageDatas[i].presentImageView = createImageView2D(swapChainImages[i], 0, 1, presentSwapFormat.format, vk::ImageAspectFlagBits::eColor);
		
		perPresentImageDatas[i].graphicQCommandPool = createGraphicsCommandPool(vk::CommandPoolCreateFlagBits::eTransient);
		
		perPresentImageDatas[i].fence = global.deviceWrapper.device.createFence(vk::FenceCreateFlags());
	}
	global.deviceWrapper.device.acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableGuardSem, vk::Fence(), &presentImageIndex);
	perPresentImageDatas[presentImageIndex].firstShow = false;
}
void VWindow::showNextImage(uint32_t waitSemaphoreCount, const vk::Semaphore* pWaitSemaphores){
	
	vk::SwapchainKHR swapChains[] = {swapChain};
	vk::PresentInfoKHR presentInfo(waitSemaphoreCount, pWaitSemaphores, 1, swapChains, &presentImageIndex, nullptr);

	WindowPerPresentImageData* data = &perPresentImageDatas[presentImageIndex];
	
	pgcQueue->presentQueue.presentKHR(&presentInfo);
	
	pgcQueue->graphicsQueue.submit({}, data->fence);
	
	global.deviceWrapper.device.acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableGuardSem, vk::Fence(), &presentImageIndex);
	
	data = &perPresentImageDatas[presentImageIndex];
	
	if(!data->firstShow){
		global.deviceWrapper.device.waitForFences({data->fence}, true, std::numeric_limits<uint64_t>::max());
		global.deviceWrapper.device.resetFences({data->fence});
	}else
		data->firstShow = false;
	
	global.deviceWrapper.device.destroyCommandPool(data->graphicQCommandPool, nullptr);
	data->graphicQCommandPool = createGraphicsCommandPool(vk::CommandPoolCreateFlagBits::eTransient);
}


VWindow::~VWindow(){
	for(size_t i = 0; i < perPresentImageDatas.size(); i++){
		global.deviceWrapper.device.destroyCommandPool(perPresentImageDatas[i].graphicQCommandPool, nullptr);
		global.deviceWrapper.device.destroyFramebuffer(perPresentImageDatas[i].framebuffer, nullptr);
		if(i != presentImageIndex)
			global.deviceWrapper.device.waitForFences({perPresentImageDatas[i].fence}, true, std::numeric_limits<uint64_t>::max());
		global.deviceWrapper.device.destroyFence(perPresentImageDatas[i].fence, nullptr);
		global.deviceWrapper.device.destroyImageView(perPresentImageDatas[i].presentImageView, nullptr);
	}
	destroySemaphore(imageAvailableGuardSem);
	
	glfwDestroyWindow (window);
	global.deviceWrapper.device.destroySwapchainKHR(swapChain);
}