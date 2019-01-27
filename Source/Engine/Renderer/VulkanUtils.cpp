#include "VulkanUtils.hpp"

using namespace  KompotEngine::Renderer;

void KompotEngine::Renderer::createVkInstance(VkInstance &vkInstance, const std::string &windowName)
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = windowName.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0_u8t, 0_u8t, 1_u8t);
    applicationInfo.pEngineName = ENGINE_NAME.c_str();
    applicationInfo.engineVersion = VK_MAKE_VERSION(ENGINE_VESRION_MAJOR, ENGINE_VESRION_MINOR, ENGINE_VESRION_PATCH);
    applicationInfo.apiVersion = VK_API_VERSION_1_1;

    auto extensionsCount = 0_u32t;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
    std::vector<const char*> extensions(extensionsCount);
    for (auto i = 0_u32t; i < extensionsCount; ++i)
    {
        extensions[i] = glfwExtensions[i];
    }
    extensions.push_back("VK_EXT_debug_utils");

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &applicationInfo;
    instanceInfo.enabledLayerCount = static_cast<unsigned int>(validationLayers.size());
    instanceInfo.ppEnabledLayerNames = validationLayers.data();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    const auto resultCode = vkCreateInstance(&instanceInfo, nullptr, &vkInstance);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "vkCreateInstance failed with code " << resultCode << ". Terminated." << std::endl;
        std::terminate();
    }
}

void KompotEngine::Renderer::loadFuntions(VkInstance vkInstance)
{
    Log &log = Log::getInstance();

    pfn_vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>
                                          (vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT"));
    if (pfn_vkCreateDebugUtilsMessengerEXT == nullptr)
    {
        log << "vkGetInstanceProcAddr for vkCreateDebugUtilsMessengerEXT failed. Terminated." << std::endl;
        std::terminate();
    }

    pfn_vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>
                                          (vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT"));
    if (pfn_vkDestroyDebugUtilsMessengerEXT == nullptr)
    {
        log << "vkGetInstanceProcAddr for vkDestroyDebugUtilsMessengerEXT failed. Terminated." << std::endl;
        std::terminate();
    }
}

void KompotEngine::Renderer::setupDebugCallback(VkInstance vkInstance, VkDebugUtilsMessengerEXT &vkDebugMessenger)
{
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
    messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerCreateInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerCreateInfo.messageType = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                                      VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                      VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerCreateInfo.pfnUserCallback = Log::vulkanDebugCallback;

    VkResult debugMessengerCreatingCode = pfn_vkCreateDebugUtilsMessengerEXT(vkInstance, &messengerCreateInfo, nullptr, &vkDebugMessenger);
    if (debugMessengerCreatingCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "vkCreateDebugUtilsMessengerEXT failed wih code " << debugMessengerCreatingCode << std::endl;
    }
}

void KompotEngine::Renderer::selectPhysicalDevice(VkInstance vkInstance, VkPhysicalDevice &vkPhysicalDevice)
{
    Log &log = Log::getInstance();

    auto physicalDevicesCount = 0_u32t;
    vkEnumeratePhysicalDevices(vkInstance, &physicalDevicesCount, nullptr);

    if (physicalDevicesCount == 0_u32t)
    {
        log << "No physical devices found" << std::endl;
        std::terminate();
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
    vkEnumeratePhysicalDevices(vkInstance, &physicalDevicesCount, physicalDevices.data());

    std::string lastDeviceName;
    auto lastPhysicalDeviceMemorySize = 0_u32t;
    for (auto &physicalDevice : physicalDevices)
    {
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        lastDeviceName = physicalDeviceProperties.deviceName;

        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
        auto memoryHeaps = std::vector<VkMemoryHeap>(physicalDeviceMemoryProperties.memoryHeaps,
                                               physicalDeviceMemoryProperties.memoryHeaps + physicalDeviceMemoryProperties.memoryHeapCount);
        auto physicalDeviceMemorySize = 1_u32t;
        for (const auto& heap : memoryHeaps)
        {
            if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            {
                physicalDeviceMemorySize = static_cast<uint32_t>(heap.size);
                break;
            }
        }

        if (physicalDeviceProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            if (physicalDeviceMemorySize > lastPhysicalDeviceMemorySize)
            {
                vkPhysicalDevice = physicalDevice;
            }
        }
        else if (physicalDeviceProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            if (vkPhysicalDevice == nullptr)
            {
                vkPhysicalDevice = physicalDevice;
            }
        }
    }

    if (vkPhysicalDevice == nullptr)
    {
        log << "No situable devices found" << std::endl;
        std::terminate();
    }

    log << "Founded physical device \"" << lastDeviceName << "\"." << std::endl;
}

QueueFamilyIndices KompotEngine::Renderer::findQueueFamilies(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
{
    QueueFamilyIndices indices;

    auto queueFamiliesCount = 0_u32t;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamiliesCount, queueFamilies.data());

    for ( auto i = 0_u32t; i < queueFamilies.size(); ++i)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, vkSurface, &presentSupport);

        if (queueFamilies[i].queueCount > 0_u32t && presentSupport)
        {
            indices.presentFamilyIndex = i;
        }

        if (queueFamilies[i].queueCount > 0_u32t && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicFamilyIndex = i;
        }

        if (indices.isComplete())
        {
            break;
        }
    }

    if (!indices.isComplete())
    {
        Log &log = Log::getInstance();
        log << "Not all queue families found. Terminated." << std::endl;
        std::terminate();
    }

    return indices;
}

void KompotEngine::Renderer::createLogicalDeviceAndQueue(
        VkPhysicalDevice vkPhysicalDevice,
        VkDevice &vkDevice,
        VkQueue &graphicQueue,
        VkQueue &presentQueue,
        VkSurfaceKHR vkSurface)
{
    //VkQueueFamilyProperties
    const auto familiesIndecies = findQueueFamilies(vkPhysicalDevice, vkSurface);
    std::set<uint32_t> indices = {
        familiesIndecies.graphicFamilyIndex.value(),
        familiesIndecies.presentFamilyIndex.value()
    };
    const auto queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (const auto familyIndex : indices)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        //graphicQueueInfo.flags = ???
        queueCreateInfo.queueFamilyIndex = familyIndex;
        queueCreateInfo.queueCount = 1_u32t;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    std::vector<const char*> extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(indices.size());
    deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    deviceInfo.ppEnabledLayerNames = validationLayers.data();
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    deviceInfo.ppEnabledExtensionNames = extensions.data();
    deviceInfo.pEnabledFeatures = &physicalDeviceFeatures;

    const auto resultCode = vkCreateDevice(vkPhysicalDevice, &deviceInfo, nullptr, &vkDevice);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "vkCreateDevice failed. Terminated." << std::endl;
        std::terminate();
    }

    vkGetDeviceQueue(vkDevice, familiesIndecies.graphicFamilyIndex.value(), 0_u32t, &graphicQueue);
    vkGetDeviceQueue(vkDevice, familiesIndecies.presentFamilyIndex.value(), 0_u32t, &presentQueue);
}

void KompotEngine::Renderer::createSurface(VkInstance vkInstance, GLFWwindow *window, VkSurfaceKHR &vkSurface)
{
    const auto resultCode = glfwCreateWindowSurface(vkInstance, window, nullptr, &vkSurface);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "glfwCreateWindowSurface failed. Terminated" << std::endl;
        std::terminate();
    }
}

SwapchainSupportDetails KompotEngine::Renderer::getSwapchainDetails(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface)
{
    SwapchainSupportDetails swapchainDetails;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &swapchainDetails.capabilities);

    auto formatsCount = 0_u32t;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatsCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatsCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatsCount, formats.data());
    swapchainDetails.format = chooseSurfaceFormat(formats);


    auto presentModesCount = 0_u32t;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModesCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModesCount, presentModes.data());
    swapchainDetails.presentMode = choosePresentMode(presentModes);

    return swapchainDetails;
}

VkSurfaceFormatKHR KompotEngine::Renderer::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &surfaceFormats)
{
    if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for(const auto &surfaceFormat: surfaceFormats)
    {
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return surfaceFormat;
        }
    }

    return surfaceFormats[0];
}

VkExtent2D KompotEngine::Renderer::chooseExtent(const VkSurfaceCapabilitiesKHR &vkSurfaceCapabilities, uint32_t width, uint32_t height)
{
    if (vkSurfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return vkSurfaceCapabilities.currentExtent;
    }
    else
    {
        VkExtent2D extent = {width, height};
        extent.width = std::max(vkSurfaceCapabilities.minImageExtent.width, std::min(vkSurfaceCapabilities.maxImageExtent.width, extent.width));
        extent.height = std::max(vkSurfaceCapabilities.minImageExtent.height, std::min(vkSurfaceCapabilities.maxImageExtent.height, extent.height));
        return extent;
    }
}

VkPresentModeKHR KompotEngine::Renderer::choosePresentMode(const std::vector<VkPresentModeKHR> &presentModes)
{
    for (const auto &presentMode : presentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

void KompotEngine::Renderer::createSwapchain(VkDevice vkDevice, VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR vkSurface, uint32_t width, uint32_t height, VkSwapchainKHR &vkSwapchain)
{
    const auto swapchainDetails = getSwapchainDetails(vkPhysicalDevice, vkSurface);
    const auto queuefamilies = findQueueFamilies(vkPhysicalDevice, vkSurface);
    uint32_t queuefamiliesIndicies[] = {
        queuefamilies.graphicFamilyIndex.value(),
        queuefamilies.presentFamilyIndex.value()
    };

    VkSwapchainCreateInfoKHR swapchainInfo = {};
    swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface = vkSurface;
    swapchainInfo.minImageCount = swapchainDetails.capabilities.minImageCount + 1_u32t;
    swapchainInfo.imageFormat = swapchainDetails.format.format;
    swapchainInfo.imageColorSpace = swapchainDetails.format.colorSpace;
    swapchainInfo.imageExtent = chooseExtent(swapchainDetails.capabilities, width, height);
    swapchainInfo.imageArrayLayers = 1_u32t;
    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    //swapchainInfo.1
    if (queuefamilies.graphicFamilyIndex != queuefamilies.presentFamilyIndex)
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2_u32t;
        swapchainInfo.pQueueFamilyIndices = queuefamiliesIndicies;
    }
    else
    {
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    swapchainInfo.preTransform = swapchainDetails.capabilities.currentTransform;
    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.presentMode = swapchainDetails.presentMode;
    swapchainInfo.clipped = VK_TRUE;
    swapchainInfo.oldSwapchain = nullptr;

    const auto resultCode = vkCreateSwapchainKHR(vkDevice, &swapchainInfo, nullptr, &vkSwapchain);
    if (resultCode != VK_SUCCESS)
    {
        Log &log = Log::getInstance();
        log << "vkCreateSwapchainKHR failed. Terminated." << std::endl;
        std::terminate();
    }

}
