#include "SDL.h"
#include "SDL_vulkan.h"
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define DEBUG_BUILD true

typedef struct {
    SDL_Window* window;
    VkInstance* instance;
    VkSurfaceKHR* surface;
    VkDevice* device;
    VkSwapchainKHR* swapchain;
} cleanup_t;

bool check_error(bool is_error, const char* error_header, const char* error_text, cleanup_t* cleanup) {
    if (is_error) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, error_text);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, error_header, error_text, NULL);
        if (cleanup->swapchain)
            vkDestroySwapchainKHR(*cleanup->device, *cleanup->swapchain, NULL);
        if (cleanup->device)
            vkDestroyDevice(*cleanup->device, NULL);
        if (cleanup->surface)
            vkDestroySurfaceKHR(*cleanup->instance, *cleanup->surface, NULL);
        if (cleanup->instance)
            vkDestroyInstance(*cleanup->instance, NULL);
        if (cleanup->window)
            SDL_DestroyWindow(cleanup->window);
        SDL_Quit();
        return true;
    }
    return false;
}

int main(int argc, char** argv) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    cleanup_t cleanup = { NULL, NULL, NULL };

    int init_success = SDL_Init(SDL_INIT_VIDEO);
    if (check_error(init_success != 0, "Error in SDL Initialisation.", SDL_GetError(), &cleanup))
        return -1;
    SDL_Window* window;
    window = SDL_CreateWindow("Easel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              1024, 768, SDL_WINDOW_VULKAN);
    if (check_error(window == NULL, "Error in SDL Window Creation.", SDL_GetError(), &cleanup))
        return -2;
    cleanup.window = window;

    VkInstance instance;
    VkResult result;
    VkApplicationInfo app_info;
    SDL_bool sdl_result;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = "Easel";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "chapliboy engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

#if DEBUG_BUILD
    const uint32_t validation_layers_count = 1;
    const char** validation_layers = (char**) malloc(validation_layers_count * sizeof(char*));
    validation_layers[0] =  "VK_LAYER_KHRONOS_validation";
    uint32_t layer_count;
    result = vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "There was an error setting up Vulkan. Could not get instance layer properties", &cleanup))
        return -3;
    VkLayerProperties* layer_properties = (VkLayerProperties*) malloc(layer_count * sizeof(VkLayerProperties));
    result = vkEnumerateInstanceLayerProperties(&layer_count, layer_properties);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "There was an error setting up Vulkan. Could not get instance layer properties", &cleanup))
        return -3;
    bool validation_available = false;
    for (int i=0; i<layer_count; i++) {
        for (int j=0; j<validation_layers_count; j++) {
            if (strcmp(validation_layers[j], layer_properties[i].layerName) == 0) {
                validation_available = true;
                break;
            }
        }
        if (validation_available)
            break;
    }
    if (check_error(!validation_available, "Error in Vulkan Setup.", "Validation layer not available.", &cleanup))
        return -3;
#endif

    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.pApplicationInfo = &app_info;
    uint32_t required_extensions_count = 0;
    const char** required_extensions;
    sdl_result = SDL_Vulkan_GetInstanceExtensions(window, &required_extensions_count, NULL);
    if (check_error(!sdl_result, "Error in getting Required Vulkan Extensions", SDL_GetError(), &cleanup))
        return -4;
    required_extensions = (char**) malloc(required_extensions_count * sizeof(char*));
    sdl_result = SDL_Vulkan_GetInstanceExtensions(window, &required_extensions_count, required_extensions);
    if (check_error(!sdl_result, "Error in getting Required Vulkan Extensions", SDL_GetError(), &cleanup))
        return -4;
    uint32_t available_extensions_count;
    result = vkEnumerateInstanceExtensionProperties(NULL, &available_extensions_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get available instance extensions.", &cleanup))
        return -4;
    VkExtensionProperties* available_extensions = (VkExtensionProperties*) malloc(available_extensions_count * sizeof(VkExtensionProperties));
    result = vkEnumerateInstanceExtensionProperties(NULL, &available_extensions_count, available_extensions);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get available instance extensions.", &cleanup))
        return -4;
    uint32_t required_extensions_available = 0;
    for (int i=0; i<available_extensions_count; i++) {
        for (int j=0; j<required_extensions_count; j++) {
            if (strcmp(required_extensions[j], available_extensions[i].extensionName) == 0) {
                required_extensions_available++;
                break;
            }
        }
    }
    if (check_error(required_extensions_available != required_extensions_count, "Error in Vulkan Setup.", "The required extensions are not available.", &cleanup))
        return -4;
    create_info.enabledExtensionCount = required_extensions_count;
    create_info.ppEnabledExtensionNames = required_extensions;
    create_info.flags = 0;
#if DEBUG_BUILD
    create_info.enabledLayerCount = validation_layers_count;
    create_info.ppEnabledLayerNames = validation_layers;
#else
    create_info.enabledLayerCount = 0;
#endif
    SDL_Log("Creating Vulkan Instance\n");
    result = vkCreateInstance(&create_info, NULL, &instance);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create vulkan instance.", &cleanup))
        return -5;
    cleanup.instance = &instance;

    VkSurfaceKHR surface;
    sdl_result = SDL_Vulkan_CreateSurface(window, instance, &surface);
    if (check_error(!sdl_result, "Error in getting Creating SDL Vulkan Surface", SDL_GetError(), &cleanup))
        return -6;
    cleanup.surface = &surface;

    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    uint32_t device_count = 0;
    result = vkEnumeratePhysicalDevices(instance, &device_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical devices.", &cleanup))
        return -6;
    if (check_error(device_count == 0, "Error in Vulkan Setup.", "Could not find a GPU with Vulkan support.", &cleanup))
        return -6;
    SDL_Log("Found %i Vulkan Devices\n", device_count);
    VkPhysicalDevice* devices = (VkPhysicalDevice*) malloc(device_count * sizeof(VkPhysicalDevice));
    result = vkEnumeratePhysicalDevices(instance, &device_count, devices);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical devices.", &cleanup))
        return -6;
    if (device_count == 1)
        // Since there is only one device available, we select that. Later on, we check
        // whether it has all the functionality that we need.
        physical_device = devices[0];
    else {
        // TODO (16 Oct 2020 sam): Implement selection of best device
        // Would need to check queue families available etc.
        // Just defaulting to first one for now.
        physical_device = devices[0];
    }
    if (check_error(physical_device == VK_NULL_HANDLE, "Error in Vulkan Setup.", "Could not find a suitable GPU.", &cleanup))
        return -6;

    const uint32_t required_device_extensions_count = 1;
    const char** required_device_extensions = (char**) malloc(required_device_extensions_count * sizeof(char*));
    required_device_extensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    uint32_t device_extensions_count;
    result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extensions_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get available device extensions.", &cleanup))
        return -6;
    VkExtensionProperties* device_extensions = (VkExtensionProperties*) malloc(device_extensions_count * sizeof(VkExtensionProperties));
    result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extensions_count, device_extensions);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get available device extensions.", &cleanup))
        return -6;
    uint32_t required_device_extensions_available = 0;
    for (int i=0; i<device_extensions_count; i++) {
        for (int j=0; j<required_device_extensions_count; j++) {
            if (strcmp(required_device_extensions[j], device_extensions[i].extensionName) == 0) {
                required_device_extensions_available++;
                break;
            }
        }
    }
    if (check_error(required_device_extensions_available != required_device_extensions_count, "Error in Vulkan Setup.", "The required device extensions are not available.", &cleanup))
        return -6;
    VkSurfaceCapabilitiesKHR surface_capabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical device surface capabilities.", &cleanup))
        return -6;
    uint32_t surface_formats_count;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_formats_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical device surface formats.", &cleanup))
        return -6;
    if (check_error(surface_formats_count==0, "Error in Vulkan Setup.", "Device does not have surface formats", &cleanup))
        return -6;
    VkSurfaceFormatKHR* surface_formats = (VkSurfaceFormatKHR*) malloc(surface_formats_count * sizeof(VkSurfaceFormatKHR));
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_formats_count, surface_formats);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical device surface formats.", &cleanup))
        return -6;
    uint32_t present_modes_count;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_modes_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical device present modes.", &cleanup))
        return -6;
    if (check_error(present_modes_count==0, "Error in Vulkan Setup.", "Device does not have present modes", &cleanup))
        return -6;
    VkPresentModeKHR* present_modes = (VkPresentModeKHR*) malloc(present_modes_count * sizeof(VkPresentModeKHR));
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_modes_count, present_modes);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical device present modes.", &cleanup))
        return -6;
    VkSurfaceFormatKHR selected_surface_format = surface_formats[0];
    for (int i=0; i<surface_formats_count; i++) {
        if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            selected_surface_format = surface_formats[i];
            break;
        }
    }
    VkPresentModeKHR selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (int i=0; i<present_modes_count; i++) {
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            selected_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }
    VkExtent2D swap_extent;
    if (surface_capabilities.currentExtent.width != UINT32_MAX)
        swap_extent = surface_capabilities.currentExtent;
    else {
        SDL_Log("Calculating the swap extent.");
        // TODO (17 Oct 2020 sam): Vulkan tutorial does some random fancy shit here
        // See what this should be.
        swap_extent = surface_capabilities.maxImageExtent;
    }

    uint32_t queue_family_count = 0;
    // This method does not return a VkResult
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
    SDL_Log("Found %i queue families.\n", queue_family_count);
    VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*) malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);
    int graphics_queue_family = -1;
    int presentation_queue_family = -1;
    for (int i=0; i<queue_family_count; i++) {
        if (presentation_queue_family < 0) {
            VkBool32 supports_presentaion;
            result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_presentaion);
            if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not check queue family presentation support.", &cleanup))
                return -6;
            if (supports_presentaion)
                presentation_queue_family = i;
        }
        if (graphics_queue_family < 0) {
            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphics_queue_family = i;
        }
        if (presentation_queue_family >= 0 && graphics_queue_family >= 0)
            break;
    }
    SDL_Log("Graphics Queue Family is %i\n", graphics_queue_family);
    SDL_Log("Presentation Queue Family is %i\n", presentation_queue_family);
    if (check_error(graphics_queue_family < 0, "Error in Vulkan Setup.", "Could not find queue family with graphics", &cleanup))
        return -6;
    if (check_error(presentation_queue_family < 0, "Error in Vulkan Setup.", "Could not find queue family with presentaition support", &cleanup))
        return -6;
    // TODO (17 Oct 2020 sam): Currently, we're assuming the same queue family for both.
    // If this is not the case, we would have to update the queue creation that comes next.
    if (check_error(graphics_queue_family != presentation_queue_family, "Error in Vulkan Setup.", "Currently do not support different presentation and grahics queue families", &cleanup))
        return -6;
    
    VkDevice device;
    VkDeviceQueueCreateInfo queue_create_info;
    float graphics_queue_priority = 1.0f;
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.pNext = NULL;
    queue_create_info.flags = 0;
    queue_create_info.queueFamilyIndex = graphics_queue_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &graphics_queue_priority;
    // TODO (16 Oct 2020 sam): Figure out whether all features here are correctly
    // being set to false.
    VkPhysicalDeviceFeatures device_features = {VK_FALSE};
    VkDeviceCreateInfo device_create_info;
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = NULL;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = 1;
#if DEBUG_BUILD
    device_create_info.enabledLayerCount = validation_layers_count;
    device_create_info.ppEnabledLayerNames = validation_layers;
#else
    device_create_info.enabledLayerCount = 0;
#endif
    device_create_info.enabledExtensionCount = required_device_extensions_count;
    device_create_info.ppEnabledExtensionNames = required_device_extensions;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.pEnabledFeatures = &device_features;
    result = vkCreateDevice(physical_device, &device_create_info, NULL, &device);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create device.", &cleanup))
        return -7;
    cleanup.device = &device;
    VkQueue graphics_queue;
    VkQueue presentation_queue;
    uint32_t graphics_queue_index = 0;
    uint32_t presentation_queue_index = 0;
    vkGetDeviceQueue(device, graphics_queue_family, graphics_queue_index, &graphics_queue);
    vkGetDeviceQueue(device, presentation_queue_family, presentation_queue_index, &presentation_queue);

    uint32_t queue_family_indices[] = { graphics_queue_family, presentation_queue_family };
    uint32_t image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount>0 && image_count>surface_capabilities.maxImageCount)
        image_count = surface_capabilities.maxImageCount;
    SDL_Log("Image Count: %i\n", image_count);
    VkSwapchainCreateInfoKHR swapchain_create_info;
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = NULL;
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = selected_surface_format.format;
    swapchain_create_info.imageColorSpace = selected_surface_format.colorSpace;
    swapchain_create_info.imageExtent = swap_extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (graphics_queue_family != presentation_queue_family) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 0;
        swapchain_create_info.pQueueFamilyIndices = NULL;
    }
    swapchain_create_info.preTransform = surface_capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = selected_present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain;
    result = vkCreateSwapchainKHR(device, &swapchain_create_info, NULL, &swapchain);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create swapchain.", &cleanup))
        return -7;
    cleanup.swapchain = &swapchain;

    uint32_t swapchain_image_count;
    result = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get swapchain images.", &cleanup))
        return -7;
    VkImage* swapchain_images = (VkImage*) malloc(swapchain_image_count * sizeof(VkImage));
    result = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get swapchain images.", &cleanup))
        return -7;
    VkFormat swapchain_image_format = swapchain_create_info.imageFormat;
    VkExtent2D swapchain_extent = swapchain_create_info.imageExtent;


    SDL_Log("Running Event Loop\n");
    SDL_Event e;
    while (true) {
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT) {
            SDL_Log("Program quit after %i ticks", e.quit.timestamp);
            break;
        } 
    }

    vkDestroySwapchainKHR(device, swapchain, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);
    SDL_Log("Quitting Application\n");
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
