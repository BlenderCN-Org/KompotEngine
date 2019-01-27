#pragma once

#include "global.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <optional>

namespace KompotEngine
{

namespace Renderer
{

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR       format;
    VkPresentModeKHR         presentMode;
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicFamilyIndex;
    std::optional<uint32_t> presentFamilyIndex;

    bool isComplete() const
    {
        return graphicFamilyIndex.has_value() &&
               presentFamilyIndex.has_value();
    }

};

static PFN_vkCreateDebugUtilsMessengerEXT  pfn_vkCreateDebugUtilsMessengerEXT  = nullptr;
static PFN_vkDestroyDebugUtilsMessengerEXT pfn_vkDestroyDebugUtilsMessengerEXT = nullptr;

static std::vector<const char*> validationLayers {
    "VK_LAYER_LUNARG_standard_validation"
};

void createVkInstance(VkInstance&, const std::string&);
void loadFuntions(VkInstance);
void setupDebugCallback(VkInstance, VkDebugUtilsMessengerEXT&);
void selectPhysicalDevice(VkInstance, VkPhysicalDevice&);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice, VkSurfaceKHR);
void createLogicalDeviceAndQueue(VkPhysicalDevice, VkDevice&, VkQueue&, VkQueue&, VkSurfaceKHR);
void createSurface(VkInstance, GLFWwindow*, VkSurfaceKHR&);
SwapchainSupportDetails getSwapchainDetails(VkPhysicalDevice, VkSurfaceKHR);
VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>&);
VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR&, uint32_t, uint32_t);
void createSwapchain(VkDevice, VkPhysicalDevice, VkSurfaceKHR, uint32_t, uint32_t, VkSwapchainKHR&);

} // namespace Renderer

} //namespace KompotEngine
