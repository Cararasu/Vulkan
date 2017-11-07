#include "VWindow.h"
#include "VGlobal.h"
#include <limits>
#include <algorithm>


void VWindow::initializeWindow(){
	
	pgcQueue = vGlobal.deviceWrapper.requestPGCQueue();
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(640, 640, "Vulkan Test", NULL, NULL);
	VCHECKCALL(glfwCreateWindowSurface(vGlobal.vkinstance, window, NULL, (VkSurfaceKHR*)&surface), printf("Creation of Surface failed"));
	
	imageAvailableGuardSem = createSemaphore();
	
	{
		{
			uint32_t formatCount;
			vGlobal.physicalDevice.getSurfaceFormatsKHR(surface, &formatCount, nullptr);
			if (formatCount != 0) {
				vk::SurfaceFormatKHR formats[formatCount];
				vGlobal.physicalDevice.getSurfaceFormatsKHR(surface, &formatCount, formats);
				
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
			vkGetPhysicalDeviceSurfacePresentModesKHR(vGlobal.physicalDevice, surface, &presentModeCount, nullptr);
			if (presentModeCount != 0) {
				vk::PresentModeKHR presentModes[presentModeCount];
				vGlobal.physicalDevice.getSurfacePresentModesKHR(surface, &presentModeCount, presentModes);
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
		vGlobal.physicalDevice.getSurfaceCapabilitiesKHR(surface, &capabilities);
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
		
		printf("SUPPORTED %d\n", vGlobal.physicalDevice.getSurfaceSupportKHR(pgcQueue->presentQId, surface));
		
		V_CHECKCALL(vGlobal.deviceWrapper.device.createSwapchainKHR(&swapchainCreateInfo, nullptr, &swapChain), printf("Creation of Swapchain failed\n"));
	}
	std::vector<vk::Image> swapChainImages = vGlobal.deviceWrapper.device.getSwapchainImagesKHR(swapChain);
	
	presentImages.resize(swapChainImages.size());
	for(size_t i = 0; i < swapChainImages.size(); i++){
		presentImages[i] = createImageView2D(swapChainImages[i], presentSwapFormat.format, vk::ImageAspectFlagBits::eColor);
	}
	vGlobal.deviceWrapper.device.acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableGuardSem, vk::Fence(), &presentImageIndex);
}
void VWindow::showNextImage(uint32_t waitSemaphoreCount, const vk::Semaphore* pWaitSemaphores){
	
	vk::SwapchainKHR swapChains[] = {swapChain};
	vk::PresentInfoKHR presentInfo(waitSemaphoreCount, pWaitSemaphores, 1, swapChains, &presentImageIndex, nullptr);

	pgcQueue->presentQueue.presentKHR(&presentInfo);
	vGlobal.deviceWrapper.device.acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableGuardSem, vk::Fence(), &presentImageIndex);
	
	//vGlobal.deviceWrapper.requestTransferQueue()->waitForFinish();
	//pgcQueue->waitForFinish();
	
	printf("Destroy TransferPool %d\n", presentImageIndex);
	vGlobal.deviceWrapper.device.destroyCommandPool(tranferQCommandPools[presentImageIndex], nullptr);
	tranferQCommandPools[presentImageIndex] = createTransferCommandPool(vk::CommandPoolCreateFlagBits::eTransient);
	printf("---------\n");
	printf("Destroy GraphicsPool %d\n", presentImageIndex);
	vGlobal.deviceWrapper.device.destroyCommandPool(graphicQCommandPools[presentImageIndex], nullptr);
	graphicQCommandPools[presentImageIndex] = createGraphicsCommandPool(vk::CommandPoolCreateFlagBits::eTransient);
	printf("---------\n");
}
VWindow::~VWindow(){
	destroySemaphore(imageAvailableGuardSem);
}