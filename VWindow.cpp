
#include "VGlobal.h"
#include "VWindow.h"
#include <limits>
#include <algorithm>
#include <map>
#include "DataWrapper.h"


std::map<GLFWwindow*, VWindow*> windowMap;

void framebuffer_size_callback (GLFWwindow* window, int width, int height) {
	auto it = windowMap.find (window);
	if (it != windowMap.end()) {
		it->second->handleResize (width, height);
	}
}

void VWindow::handleResize (uint32_t width, uint32_t height) {

	windowState = WindowState::eResized;
}

void VWindow::initializeWindow(VInstance* instance) {
	this->instance = instance;
	pgcQueue = instance->requestPGCQueue();

	glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow (640, 640, "Vulkan Test", NULL, NULL);
	VCHECKCALL (glfwCreateWindowSurface (global.vkinstance, window, NULL, (VkSurfaceKHR*) &surface), printf ("Creation of Surface failed"));

	windowMap.insert (std::make_pair (window, this));
	glfwSetFramebufferSizeCallback (window, framebuffer_size_callback);

	imageAvailableGuardSem = instance->createSemaphore();

	{
		uint32_t formatCount;
		instance->physicalDevice.getSurfaceFormatsKHR (surface, &formatCount, nullptr);
		if (formatCount != 0) {
			vk::SurfaceFormatKHR formats[formatCount];
			instance->physicalDevice.getSurfaceFormatsKHR (surface, &formatCount, formats);

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
	chosenPresentationMode = vk::PresentModeKHR::eFifo;//can be turned into global flags of what is supported
	{
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR (instance->physicalDevice, surface, &presentModeCount, nullptr);
		if (presentModeCount != 0) {
			vk::PresentModeKHR presentModes[presentModeCount];
			instance->physicalDevice.getSurfacePresentModesKHR (surface, &presentModeCount, presentModes);
			chosenPresentationMode = vk::PresentModeKHR::eFifo;
			for (size_t i = 0; i < presentModeCount; i++) {
				if (presentModes[i] == vk::PresentModeKHR::eMailbox) {
					chosenPresentationMode = vk::PresentModeKHR::eMailbox;
					break;
				} else if (presentModes[i] == vk::PresentModeKHR::eImmediate) {
					chosenPresentationMode = vk::PresentModeKHR::eImmediate;
				}
			}
		}
	}
	capabilities = instance->physicalDevice.getSurfaceCapabilitiesKHR (surface);
	{
		int width, height;
		glfwGetWindowSize (window, &width, &height);
		vk::Extent2D actualExtent = {width, height};
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			swapChainExtend = capabilities.currentExtent;
		} else {

			actualExtent.width = std::max (capabilities.minImageExtent.width, std::min (capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max (capabilities.minImageExtent.height, std::min (capabilities.maxImageExtent.height, actualExtent.height));

			swapChainExtend = actualExtent;
		}
	}
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = std::max<uint32_t> (capabilities.maxImageCount, V_MAX_PRESENTIMAGE_COUNT);
	}
	perPresentImageDatas.resize (imageCount);
	createSwapchain();
	windowState = WindowState::eInitialized;
}

void VWindow::createSwapchain() {

	for (WindowPerPresentImageData& data : perPresentImageDatas) {
		if (!data.firstShow) {
			instance->device.waitForFences ({data.fence}, true, std::numeric_limits<uint64_t>::max());
			instance->device.resetFences ({data.fence});
			data.firstShow = true;
		}
	}
	{
		vk::SwapchainCreateInfoKHR swapchainCreateInfo (vk::SwapchainCreateFlagsKHR(), surface, perPresentImageDatas.size(), presentSwapFormat.format, presentSwapFormat.colorSpace, swapChainExtend, 1,
		        vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, 0, nullptr, vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eImmediate, 0, swapChain);

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
		swapchainCreateInfo.oldSwapchain = swapChain;

		printf ("SUPPORTED %d\n", instance->physicalDevice.getSurfaceSupportKHR (pgcQueue->presentQId, surface));

		V_CHECKCALL (instance->device.createSwapchainKHR (&swapchainCreateInfo, nullptr, &swapChain), printf ("Creation of Swapchain failed\n"));
		instance->device.destroySwapchainKHR (swapchainCreateInfo.oldSwapchain);
	}

	for (WindowPerPresentImageData& data : perPresentImageDatas) {
		if (data.graphicQCommandPool) {
			instance->device.destroyCommandPool (data.graphicQCommandPool);
			data.graphicQCommandPool = vk::CommandPool();
		}
		if (data.fence) {
			instance->device.destroyFence (data.fence);
			data.fence = vk::Fence();
		}

		instance->device.destroyImageView (data.presentImageView);
		instance->device.destroyFramebuffer (data.framebuffer);
	}

	std::vector<vk::Image> swapChainImages = instance->device.getSwapchainImagesKHR (swapChain);

	perPresentImageDatas.resize (swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		perPresentImageDatas[i].presentImage = swapChainImages[i];
		perPresentImageDatas[i].graphicQCommandPool = instance->createGraphicsCommandPool (vk::CommandPoolCreateFlagBits::eTransient);
		perPresentImageDatas[i].fence = instance->device.createFence (vk::FenceCreateFlags());
	}
	swapChainExtend = capabilities.maxImageExtent;
	instance->device.acquireNextImageKHR (swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableGuardSem, vk::Fence(), &presentImageIndex);

	{
		//create/recreate depth image
		if (depthImageView) {
			instance->device.destroyImageView (depthImageView);
		}
		if (depthImage) {
			delete depthImage;
		}
		vk::Format depthFormat = instance->findDepthFormat();

		vk::Extent3D extent (swapChainExtend.width, swapChainExtend.height, 1);
		depthImage = new ImageWrapper (instance, extent, 1, 1, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlags (vk::ImageUsageFlagBits::eDepthStencilAttachment),
		                               vk::ImageAspectFlagBits::eDepth, vk::MemoryPropertyFlags (vk::MemoryPropertyFlagBits::eDeviceLocal));
		depthImageView = instance->createImageView2D (depthImage->image, 0, depthImage->mipMapLevels, depthImage->format, vk::ImageAspectFlags (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil));
		instance->transitionImageLayout (depthImage->image, depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal,
		                       vk::ImageAspectFlags (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil), getCurrentGraphicsCommandPool(), pgcQueue->graphicsQueue);

	}

	if (standardmodule.pipeline) {
		updateExtent (&instance->pipeline_module_builders.standard, &standardmodule, swapChainExtend);
	} else {
		standardmodule = createPipelineModule (&instance->pipeline_module_builders.standard, &instance->pipeline_module_layouts.standard, presentSwapFormat.format, swapChainExtend);
	}
	for (WindowPerPresentImageData& data : perPresentImageDatas) {
		data.presentImageView = instance->createImageView2D (data.presentImage, 0, 1, presentSwapFormat.format, vk::ImageAspectFlagBits::eColor);
		data.firstShow = true;
		vk::ImageView attachments[2] = {data.presentImageView, depthImageView};
		vk::FramebufferCreateInfo framebufferInfo (vk::FramebufferCreateFlags(), standardmodule.renderPass, 2, attachments, swapChainExtend.width, swapChainExtend.height, 1);
		data.framebuffer = instance->device.createFramebuffer (framebufferInfo, nullptr);
	}
	WindowPerPresentImageData* data = &perPresentImageDatas[presentImageIndex];
	
	pgcQueue->graphicsQueue.submit ({}, data->fence);
	data->firstShow = false;
	instance->device.waitForFences ({data->fence}, true, std::numeric_limits<uint64_t>::max());
	instance->device.resetFences ({data->fence});
}
void VWindow::setupFrame() {

	printf("Setup Frame\n");
	switch (windowState) {
	case WindowState::eUninitialized:
		printf ("Need to Initialize before setup\n");
		assert (false);
		return;
	case WindowState::eFramePrepared:
		printf ("Frame %d already prepared\n", presentImageIndex);
		assert (false);
		return;
	case WindowState::eInitialized: {

	} break;
	case WindowState::eNotVisible: {
		if(capabilities.maxImageExtent.width == 0 || capabilities.maxImageExtent.height == 0){
			return;//no change stay at WindowState::eNotVisible
		}
	}//fallthrough
	case WindowState::eResized: {
		capabilities = instance->physicalDevice.getSurfaceCapabilitiesKHR (surface);
		if (capabilities.maxImageExtent != swapChainExtend) {
			if(capabilities.maxImageExtent.width > 0 && capabilities.maxImageExtent.height > 0){
				printf ("WaslNats %dx%d\n", capabilities.maxImageExtent.width, capabilities.maxImageExtent.height);
				swapChainExtend = capabilities.maxImageExtent;
				createSwapchain();
			}else{
				printf ("WaslNats %dx%d\n", capabilities.maxImageExtent.width, capabilities.maxImageExtent.height);
				windowState = WindowState::eNotVisible;
				return;
			}
			break;
		}
	}
	case WindowState::eFramePresented: {

		instance->device.acquireNextImageKHR (swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableGuardSem, vk::Fence(), &presentImageIndex);

		WindowPerPresentImageData* data = &perPresentImageDatas[presentImageIndex];
		if (data->firstShow) {
			data->firstShow = false;
		} else {
			instance->device.waitForFences ({data->fence}, true, std::numeric_limits<uint64_t>::max());
			instance->device.resetFences ({data->fence});
		}
		instance->device.destroyCommandPool (data->graphicQCommandPool, nullptr);
		data->graphicQCommandPool = instance->createGraphicsCommandPool (vk::CommandPoolCreateFlagBits::eTransient);
	}
	break;
	}
	windowState = WindowState::eFramePrepared;
}
void VWindow::showNextFrame (uint32_t waitSemaphoreCount, const vk::Semaphore* pWaitSemaphores) {
	printf("Show Frame\n");
	if(windowState != WindowState::eNotVisible){
		vk::SwapchainKHR swapChains[] = {swapChain};
		vk::PresentInfoKHR presentInfo (waitSemaphoreCount, pWaitSemaphores, 1, swapChains, &presentImageIndex, nullptr);

		WindowPerPresentImageData* data = &perPresentImageDatas[presentImageIndex];

		pgcQueue->presentQueue.presentKHR (&presentInfo);
		pgcQueue->graphicsQueue.submit ({}, data->fence);

		windowState = WindowState::eFramePresented;
	}
}


VWindow::~VWindow() {

	instance->device.destroyImageView (depthImageView, nullptr);
	delete depthImage;

	for (size_t i = 0; i < perPresentImageDatas.size(); i++) {
		instance->device.destroyCommandPool (perPresentImageDatas[i].graphicQCommandPool, nullptr);
		instance->device.destroyFramebuffer (perPresentImageDatas[i].framebuffer, nullptr);
		if (i != presentImageIndex && perPresentImageDatas[presentImageIndex].firstShow)
			instance->device.waitForFences ({perPresentImageDatas[i].fence}, true, std::numeric_limits<uint64_t>::max());
		instance->device.destroyFence (perPresentImageDatas[i].fence, nullptr);
		instance->device.destroyImageView (perPresentImageDatas[i].presentImageView, nullptr);
	}

	deletePipelineModule (&instance->pipeline_module_builders.standard, standardmodule);

	instance->destroySemaphore (imageAvailableGuardSem);

	glfwDestroyWindow (window);
	instance->device.destroySwapchainKHR (swapChain);
}
