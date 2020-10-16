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
    VkDevice* device;
} cleanup_t;

void check_error(bool is_error, const char* error_header, const char* error_text, cleanup_t* cleanup) {
    if (is_error) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, error_text);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, error_header, error_text, NULL);
        if (cleanup->device)
            vkDestroyDevice(*cleanup->device, NULL);
        if (cleanup->instance)
            vkDestroyInstance(*cleanup->instance, NULL);
        if (cleanup->window)
            SDL_DestroyWindow(cleanup->window);
        SDL_Quit();
        exit(-1);
    }
}

int main(int argc, char** argv) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    cleanup_t cleanup = { NULL, NULL, NULL };

    int init_success = SDL_Init(SDL_INIT_VIDEO);
    check_error(init_success != 0, "Error in SDL Initialisation.", SDL_GetError(), &cleanup);
    SDL_Window* window;
    window = SDL_CreateWindow("Easel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              1024, 768, SDL_WINDOW_VULKAN);
    check_error(window == NULL, "Error in SDL Window Creation.", SDL_GetError(), &cleanup);
    cleanup.window = window;

    VkInstance instance;
    VkResult result;
    VkApplicationInfo app_info;
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
    check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "There was an error setting up Vulkan. Could not get instance layer properties", &cleanup);
    VkLayerProperties* layer_properties = (VkLayerProperties*) malloc(layer_count * sizeof(VkLayerProperties));
    result = vkEnumerateInstanceLayerProperties(&layer_count, layer_properties);
    check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "There was an error setting up Vulkan. Could not get instance layer properties", &cleanup);
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
    check_error(!validation_available, "Error in Vulkan Setup.", "Validation layer not available.", window, NULL);
#endif

    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.pApplicationInfo = &app_info;
    uint32_t required_extensions_count = 0;
    const char** required_extensions;
    SDL_bool get_extensions;
    get_extensions = SDL_Vulkan_GetInstanceExtensions(window, &required_extensions_count, NULL);
    check_error(!get_extensions, "Error in getting Required Vulkan Extensions", SDL_GetError(), &cleanup);
    required_extensions = (char**) malloc(required_extensions_count * sizeof(char*));
    get_extensions = SDL_Vulkan_GetInstanceExtensions(window, &required_extensions_count, required_extensions);
    check_error(!get_extensions, "Error in getting Required Vulkan Extensions", SDL_GetError(), &cleanup);
    uint32_t available_extensions_count;
    result = vkEnumerateInstanceExtensionProperties(NULL, &available_extensions_count, NULL);
    check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get available instance extensions.", &cleanup);
    VkExtensionProperties* available_extensions = (VkExtensionProperties*) malloc(available_extensions_count * sizeof(VkExtensionProperties));
    result = vkEnumerateInstanceExtensionProperties(NULL, &available_extensions_count, available_extensions);
    check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get available instance extensions.", &cleanup);
    uint32_t required_extensions_available = 0;
    for (int i=0; i<available_extensions_count; i++) {
        for (int j=0; j<required_extensions_count; j++) {
            if (strcmp(required_extensions[j], available_extensions[i].extensionName) == 0) {
                required_extensions_available++;
                break;
            }
        }
    }
    check_error(required_extensions_available != required_extensions_count, "Error in Vulkan Setup.", "The required extensions are not available.", &cleanup);
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
    check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create vulkan instance.", &cleanup);
    cleanup.instance = &instance;

    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    uint32_t device_count = 0;
    result = vkEnumeratePhysicalDevices(instance, &device_count, NULL);
    check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical devices.", &cleanup);
    check_error(device_count == 0, "Error in Vulkan Setup.", "Could not find a GPU with Vulkan support.", &cleanup);
    SDL_Log("Found %i Vulkan Devices\n", device_count);
    VkPhysicalDevice* devices = (VkPhysicalDevice*) malloc(device_count * sizeof(VkPhysicalDevice));
    result = vkEnumeratePhysicalDevices(instance, &device_count, devices);
    check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical devices.", &cleanup);
    if (device_count == 1)
        // TODO (16 Oct 2020 sam): Need to check if device is suitable
        // and has all required queue families etc.
        physical_device = devices[0];
    else {
        // TODO (16 Oct 2020 sam): Implement selection of best device
        // Would need to check queue families available etc.
    }
    check_error(physical_device == VK_NULL_HANDLE, "Error in Vulkan Setup.", "Could not find a suitable GPU.", &cleanup);
    uint32_t queue_family_count = 0;
    // This method does not return a VkResult
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
    SDL_Log("Found %i queue families.\n", queue_family_count);
    VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*) malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);
    int graphics_queue_family = -1;
    for (int i=0; i<queue_family_count; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_queue_family = i;
            break;
        }
    }
    SDL_Log("Graphics Queue Family is %i\n", graphics_queue_family);
    check_error(graphics_queue_family < 0, "Error in Vulkan Setup.", "Could not find queue family with graphics", &cleanup);

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
    device_create_info.enabledExtensionCount = 0;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.pEnabledFeatures = &device_features;
    result = vkCreateDevice(physical_device, &device_create_info, NULL, &device);
    check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create device.", &cleanup);
    cleanup.device = &device;
    VkQueue graphics_queue;
    uint32_t graphics_queue_index = 0;
    vkGetDeviceQueue(device, graphics_queue_family, graphics_queue_index, &graphics_queue);


    SDL_Log("Running Event Loop\n");
    SDL_Event e;
    while (true) {
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT) {
            SDL_Log("Program quit after %i ticks", e.quit.timestamp);
            break;
        } 
    }

    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);
    SDL_Log("Quitting Application\n");
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
