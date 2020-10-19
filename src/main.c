#include "SDL.h"
#include "SDL_vulkan.h"
#include <vulkan/vulkan.h>

#define DEBUG_BUILD 1
#define MAX_FRAMES_IN_FLIGHT 2

typedef struct {
    SDL_Window* window;
    VkInstance* instance;
    VkSurfaceKHR* surface;
    VkDevice* device;
    VkSwapchainKHR* swapchain;
    Uint32 image_count;
    VkImageView* imageviews;
    VkShaderModule* vertex_shader_module;
    VkShaderModule* fragment_shader_module;
    VkPipelineLayout* pipeline_layout;
    VkRenderPass* render_pass;
    VkPipeline* graphics_pipeline;
    VkFramebuffer* swapchain_framebuffers;
    VkCommandPool* command_pool;
    VkSemaphore* image_available_semaphores;
    VkSemaphore* render_finished_semaphores;
    VkFence* in_flight_fences;
    VkQueue* presentation_queue;
} cleanup_t;

SDL_bool check_error(SDL_bool is_error, const char* error_header, const char* error_text, cleanup_t* cleanup) {
    // TODO (18 Oct 2020 sam): Try converting this to some short-circuited check, so that function
    // is not called unless necessary.
    if (is_error) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, error_text);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, error_header, error_text, NULL);
        if (cleanup->presentation_queue)
            vkQueueWaitIdle(*cleanup->presentation_queue);
        if (cleanup->in_flight_fences)
            for (Uint32 i=0; i<MAX_FRAMES_IN_FLIGHT; i++)
                vkDestroyFence(*cleanup->device, cleanup->in_flight_fences[i], NULL);
        if (cleanup->image_available_semaphores)
            for (Uint32 i=0; i<MAX_FRAMES_IN_FLIGHT; i++)
                vkDestroySemaphore(*cleanup->device, cleanup->image_available_semaphores[i], NULL);
        if (cleanup->render_finished_semaphores)
            for (Uint32 i=0; i<MAX_FRAMES_IN_FLIGHT; i++)
                vkDestroySemaphore(*cleanup->device, cleanup->render_finished_semaphores[i], NULL);
        if (cleanup->command_pool)
            vkDestroyCommandPool(*cleanup->device, *cleanup->command_pool, NULL);
        if (cleanup->swapchain_framebuffers) {
            for (Uint32 i=0; i<cleanup->image_count; i++)
                vkDestroyFramebuffer(*cleanup->device, cleanup->swapchain_framebuffers[i], NULL);
        }
        if (cleanup->graphics_pipeline)
            vkDestroyPipeline(*cleanup->device, *cleanup->graphics_pipeline, NULL);
        if (cleanup->pipeline_layout)
            vkDestroyPipelineLayout(*cleanup->device, *cleanup->pipeline_layout, NULL);
        if (cleanup->render_pass)
            vkDestroyRenderPass(*cleanup->device, *cleanup->render_pass, NULL);
        if (cleanup->vertex_shader_module)
            vkDestroyShaderModule(*cleanup->device, *cleanup->vertex_shader_module, NULL);
        if (cleanup->fragment_shader_module)
            vkDestroyShaderModule(*cleanup->device, *cleanup->fragment_shader_module, NULL);
        if (cleanup->image_count > 0) {
            for (Uint32 i=0; i<cleanup->image_count; i++)
                vkDestroyImageView(*cleanup->device, cleanup->imageviews[i], NULL);
        }
        if (cleanup->swapchain)
            vkDestroySwapchainKHR(*cleanup->device, *cleanup->swapchain, NULL);
        if (cleanup->device) {
            vkDeviceWaitIdle(*cleanup->device);
            vkDestroyDevice(*cleanup->device, NULL);
        }
        if (cleanup->surface)
            vkDestroySurfaceKHR(*cleanup->instance, *cleanup->surface, NULL);
        if (cleanup->instance)
            vkDestroyInstance(*cleanup->instance, NULL);
        if (cleanup->window)
            SDL_DestroyWindow(cleanup->window);
        SDL_Quit();
        return SDL_TRUE;
    }
    return SDL_FALSE;
}

Sint64 read_shader_file(const char* filename, Uint32** buffer) {
    SDL_RWops* shader_file;
    shader_file = SDL_RWFromFile(filename, "rb");
    Sint64 file_size;
    size_t read_size;
    Sint64 result;
    if (shader_file == NULL)
        return -1;
    file_size = SDL_RWseek(shader_file, 0, RW_SEEK_END);
    if (file_size < 0) {
        SDL_RWclose(shader_file);
        return -2;
    }
    result = SDL_RWseek(shader_file, 0, RW_SEEK_SET);
    if (result < 0) {
        SDL_RWclose(shader_file);
        return -4;
    }
    Uint32* temp_buffer = (Uint32*) SDL_malloc(file_size * sizeof(Uint32));
    read_size = SDL_RWread(shader_file, temp_buffer, sizeof(Uint32), file_size);
    if ((Sint64)(read_size*sizeof(Uint32)) != file_size) {
        SDL_free(temp_buffer);
        SDL_RWclose(shader_file);
        return -5;
    }
    SDL_RWclose(shader_file);
    *buffer = temp_buffer;
    return file_size;
}

int main(int argc, char** argv) {
    argc; argv;
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    cleanup_t cleanup = { NULL };

    int init_success = SDL_Init(SDL_INIT_VIDEO);
    if (check_error(init_success != 0, "Error in SDL Initialisation.", SDL_GetError(), &cleanup))
        return -1;
    SDL_Window* window;
    window = SDL_CreateWindow("Easel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              1024, 768, SDL_WINDOW_VULKAN);
    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
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
    const Uint32 validation_layers_count = 1;
    const char** validation_layers = (char**) SDL_malloc(validation_layers_count * sizeof(char*));
    validation_layers[0] =  "VK_LAYER_KHRONOS_validation";
    Uint32 layer_count;
    result = vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "There was an error setting up Vulkan. Could not get instance layer properties", &cleanup))
        return -3;
    VkLayerProperties* layer_properties = (VkLayerProperties*) SDL_malloc(layer_count * sizeof(VkLayerProperties));
    result = vkEnumerateInstanceLayerProperties(&layer_count, layer_properties);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "There was an error setting up Vulkan. Could not get instance layer properties", &cleanup))
        return -3;
    SDL_bool validation_available = SDL_FALSE;
    for (Uint32 i=0; i<layer_count; i++) {
        for (Uint32 j=0; j<validation_layers_count; j++) {
            if (SDL_strcmp(validation_layers[j], layer_properties[i].layerName) == 0) {
                validation_available = SDL_TRUE;
                break;
            }
        }
        if (validation_available)
            break;
    }
    if (check_error(!validation_available, "Error in Vulkan Setup.", "Validation layer not available.", &cleanup))
        return -3;
    SDL_free(layer_properties);
#endif

    VkInstanceCreateInfo instance_create_info;
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext = NULL;
    instance_create_info.pApplicationInfo = &app_info;
    Uint32 required_extensions_count = 0;
    const char** required_extensions;
    sdl_result = SDL_Vulkan_GetInstanceExtensions(window, &required_extensions_count, NULL);
    if (check_error(!sdl_result, "Error in getting Required Vulkan Extensions", SDL_GetError(), &cleanup))
        return -4;
    required_extensions = (char**) SDL_malloc(required_extensions_count * sizeof(char*));
    sdl_result = SDL_Vulkan_GetInstanceExtensions(window, &required_extensions_count, required_extensions);
    if (check_error(!sdl_result, "Error in getting Required Vulkan Extensions", SDL_GetError(), &cleanup))
        return -4;
    Uint32 available_extensions_count;
    result = vkEnumerateInstanceExtensionProperties(NULL, &available_extensions_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get available instance extensions.", &cleanup))
        return -4;
    VkExtensionProperties* available_extensions = (VkExtensionProperties*) SDL_malloc(available_extensions_count * sizeof(VkExtensionProperties));
    result = vkEnumerateInstanceExtensionProperties(NULL, &available_extensions_count, available_extensions);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get available instance extensions.", &cleanup))
        return -4;
    Uint32 required_extensions_available = 0;
    for (Uint32 i=0; i<available_extensions_count; i++) {
        for (Uint32 j=0; j<required_extensions_count; j++) {
            if (SDL_strcmp(required_extensions[j], available_extensions[i].extensionName) == 0) {
                required_extensions_available++;
                break;
            }
        }
    }
    if (check_error(required_extensions_available != required_extensions_count, "Error in Vulkan Setup.", "The required extensions are not available.", &cleanup))
        return -4;
    instance_create_info.enabledExtensionCount = required_extensions_count;
    instance_create_info.ppEnabledExtensionNames = required_extensions;
    instance_create_info.flags = 0;
#if DEBUG_BUILD
    instance_create_info.enabledLayerCount = validation_layers_count;
    instance_create_info.ppEnabledLayerNames = validation_layers;
#else
    instance_create_info.enabledLayerCount = 0;
#endif
    SDL_Log("Creating Vulkan Instance\n");
    result = vkCreateInstance(&instance_create_info, NULL, &instance);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create vulkan instance.", &cleanup))
        return -5;
    cleanup.instance = &instance;

    VkSurfaceKHR surface;
    sdl_result = SDL_Vulkan_CreateSurface(window, instance, &surface);
    if (check_error(!sdl_result, "Error in getting Creating SDL Vulkan Surface", SDL_GetError(), &cleanup))
        return -6;
    cleanup.surface = &surface;

    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    Uint32 device_count = 0;
    result = vkEnumeratePhysicalDevices(instance, &device_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical devices.", &cleanup))
        return -6;
    if (check_error(device_count == 0, "Error in Vulkan Setup.", "Could not find a GPU with Vulkan support.", &cleanup))
        return -6;
    SDL_Log("Found %i Vulkan Devices\n", device_count);
    VkPhysicalDevice* devices = (VkPhysicalDevice*) SDL_malloc(device_count * sizeof(VkPhysicalDevice));
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

    const Uint32 required_device_extensions_count = 1;
    const char** required_device_extensions = (char**) SDL_malloc(required_device_extensions_count * sizeof(char*));
    required_device_extensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    Uint32 device_extensions_count;
    result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extensions_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get available device extensions.", &cleanup))
        return -6;
    VkExtensionProperties* device_extensions = (VkExtensionProperties*) SDL_malloc(device_extensions_count * sizeof(VkExtensionProperties));
    result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extensions_count, device_extensions);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get available device extensions.", &cleanup))
        return -6;
    Uint32 required_device_extensions_available = 0;
    for (Uint32 i=0; i<device_extensions_count; i++) {
        for (Uint32 j=0; j<required_device_extensions_count; j++) {
            if (SDL_strcmp(required_device_extensions[j], device_extensions[i].extensionName) == 0) {
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
    Uint32 surface_formats_count;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_formats_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical device surface formats.", &cleanup))
        return -6;
    if (check_error(surface_formats_count==0, "Error in Vulkan Setup.", "Device does not have surface formats", &cleanup))
        return -6;
    VkSurfaceFormatKHR* surface_formats = (VkSurfaceFormatKHR*) SDL_malloc(surface_formats_count * sizeof(VkSurfaceFormatKHR));
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_formats_count, surface_formats);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical device surface formats.", &cleanup))
        return -6;
    Uint32 present_modes_count;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_modes_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical device present modes.", &cleanup))
        return -6;
    if (check_error(present_modes_count==0, "Error in Vulkan Setup.", "Device does not have present modes", &cleanup))
        return -6;
    VkPresentModeKHR* present_modes = (VkPresentModeKHR*) SDL_malloc(present_modes_count * sizeof(VkPresentModeKHR));
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_modes_count, present_modes);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get physical device present modes.", &cleanup))
        return -6;
    VkSurfaceFormatKHR selected_surface_format = surface_formats[0];
    for (Uint32 i=0; i<surface_formats_count; i++) {
        if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            selected_surface_format = surface_formats[i];
            break;
        }
    }
    VkPresentModeKHR selected_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (Uint32 i=0; i<present_modes_count; i++) {
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

    Uint32 queue_family_count = 0;
    // This method does not return a VkResult
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
    SDL_Log("Found %i queue families.\n", queue_family_count);
    VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*) SDL_malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);
    int graphics_queue_family = -1;
    int presentation_queue_family = -1;
    for (Uint32 i=0; i<queue_family_count; i++) {
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
    // being set to SDL_FALSE.
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
    Uint32 graphics_queue_index = 0;
    Uint32 presentation_queue_index = 0;
    vkGetDeviceQueue(device, graphics_queue_family, graphics_queue_index, &graphics_queue);
    vkGetDeviceQueue(device, presentation_queue_family, presentation_queue_index, &presentation_queue);
    cleanup.presentation_queue = &presentation_queue;

    Uint32 queue_family_indices[2];
    queue_family_indices[0] = graphics_queue_family;
    queue_family_indices[1] = presentation_queue_family;
    Uint32 image_count = surface_capabilities.minImageCount + 1;
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

    Uint32 swapchain_image_count;
    result = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, NULL);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get swapchain images.", &cleanup))
        return -7;
    VkImage* swapchain_images = (VkImage*) SDL_malloc(swapchain_image_count * sizeof(VkImage));
    result = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_images);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not get swapchain images.", &cleanup))
        return -7;
    VkFormat swapchain_image_format = swapchain_create_info.imageFormat;
    VkExtent2D swapchain_extent = swapchain_create_info.imageExtent;
    VkImageView* swapchain_image_views = (VkImageView*) SDL_malloc(swapchain_image_count * sizeof(VkImageView));
    for (Uint32 i=0; i<swapchain_image_count; i++) {
        VkImageViewCreateInfo imageview_create_info;
        imageview_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageview_create_info.pNext = NULL;
        imageview_create_info.flags = 0;
        imageview_create_info.image = swapchain_images[i];
        imageview_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageview_create_info.format = swapchain_image_format;
        imageview_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageview_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageview_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageview_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageview_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageview_create_info.subresourceRange.baseMipLevel = 0;
        imageview_create_info.subresourceRange.levelCount = 1;
        imageview_create_info.subresourceRange.baseArrayLayer = 0;
        imageview_create_info.subresourceRange.layerCount = 1;
        result = vkCreateImageView(device, &imageview_create_info, NULL, &swapchain_image_views[i]);
        if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create imageview.", &cleanup))
            return -7;
    }
    cleanup.image_count = swapchain_image_count;
    cleanup.imageviews = swapchain_image_views;

    VkAttachmentDescription color_attachment;
    color_attachment.flags = 0;
    color_attachment.format = swapchain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass;
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;
    subpass.pResolveAttachments = 0;
    subpass.pDepthStencilAttachment = NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;
    VkRenderPass render_pass;
    VkSubpassDependency subpass_dependency;
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dependency.dependencyFlags= 0;
    VkRenderPassCreateInfo render_pass_create_info;
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.pNext = NULL;
    render_pass_create_info.flags = 0;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &color_attachment;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;
    result = vkCreateRenderPass(device, &render_pass_create_info, NULL, &render_pass);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create render pass.", &cleanup))
        return -7;
    cleanup.render_pass = &render_pass;


    SDL_Log("Loading shaders\n");
    Uint32* vertex_spirv;
    Sint64 vertex_shader_length;
    vertex_shader_length = read_shader_file("data/spirv/vertex.spv", &vertex_spirv);
    if (check_error(vertex_shader_length < 0, "Error in Vulkan Setup.", "Could not read vertex shader.", &cleanup))
        return -8;
    VkShaderModuleCreateInfo vertex_shader_module_create_info;
    vertex_shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertex_shader_module_create_info.pNext = NULL;
    vertex_shader_module_create_info.flags = 0;
    vertex_shader_module_create_info.codeSize = vertex_shader_length;
    vertex_shader_module_create_info.pCode = vertex_spirv;
    VkShaderModule vertex_shader_module;
    result = vkCreateShaderModule(device, &vertex_shader_module_create_info, NULL, &vertex_shader_module);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create vertex shader module.", &cleanup))
        return -8;
    cleanup.vertex_shader_module = &vertex_shader_module;
    Uint32* fragment_spirv;
    Sint64 fragment_shader_length;
    fragment_shader_length = read_shader_file("data/spirv/fragment.spv", &fragment_spirv);
    if (check_error(fragment_shader_length < 0, "Error in Vulkan Setup.", "Could not read fragment shader.", &cleanup))
        return -8;
    VkShaderModuleCreateInfo fragment_shader_module_create_info;
    fragment_shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragment_shader_module_create_info.pNext = NULL;
    fragment_shader_module_create_info.flags = 0;
    fragment_shader_module_create_info.codeSize = fragment_shader_length;
    fragment_shader_module_create_info.pCode = fragment_spirv;
    VkShaderModule fragment_shader_module;
    result = vkCreateShaderModule(device, &fragment_shader_module_create_info, NULL, &fragment_shader_module);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create fragment shader module.", &cleanup))
        return -8;
    cleanup.fragment_shader_module = &fragment_shader_module;
    VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info;
    vertex_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_create_info.pNext = NULL;
    vertex_shader_stage_create_info.flags = 0;
    vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_create_info.module = vertex_shader_module;
    vertex_shader_stage_create_info.pName = "main";
    vertex_shader_stage_create_info.pSpecializationInfo = NULL;
    VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info;
    fragment_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_stage_create_info.pNext = NULL;
    fragment_shader_stage_create_info.flags = 0;
    fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_stage_create_info.module = fragment_shader_module;
    fragment_shader_stage_create_info.pName = "main";
    fragment_shader_stage_create_info.pSpecializationInfo = NULL;
    VkPipelineShaderStageCreateInfo shader_stages[2];
    shader_stages[0] = vertex_shader_stage_create_info;
    shader_stages[1] = fragment_shader_stage_create_info;
    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info;
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state_create_info.pNext = NULL;
    vertex_input_state_create_info.flags = 0;
    vertex_input_state_create_info.vertexBindingDescriptionCount = 0;
    vertex_input_state_create_info.pVertexBindingDescriptions = NULL;
    vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
    vertex_input_state_create_info.pVertexAttributeDescriptions = NULL;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info;
    input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_create_info.pNext = NULL;
    input_assembly_state_create_info.flags = 0;
    input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapchain_extent.width;
    viewport.height = (float) swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = swapchain_extent;
    VkPipelineViewportStateCreateInfo viewport_state_create_info;
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.pNext = NULL;
    viewport_state_create_info.flags = 0;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;
    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info;
    rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_create_info.pNext = NULL;
    rasterization_state_create_info.flags = 0;
    rasterization_state_create_info.depthClampEnable = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterization_state_create_info.depthBiasEnable = VK_FALSE;
    rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
    rasterization_state_create_info.depthBiasClamp = 0.0f;
    rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;
    rasterization_state_create_info.lineWidth = 1.0f;
    VkPipelineMultisampleStateCreateInfo multisample_state_create_info;
    multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_create_info.pNext = NULL;
    multisample_state_create_info.flags = 0;
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state_create_info.sampleShadingEnable = VK_FALSE;
    multisample_state_create_info.minSampleShading = 1.0f;
    multisample_state_create_info.pSampleMask = NULL;
    multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
    multisample_state_create_info.alphaToOneEnable = VK_FALSE;
    VkPipelineColorBlendAttachmentState color_blend_attachment_state;
    color_blend_attachment_state.blendEnable = VK_FALSE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info;
    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.pNext = NULL;
    color_blend_state_create_info.flags = 0;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;
    color_blend_state_create_info.blendConstants[0] = 0.0f;
    color_blend_state_create_info.blendConstants[1] = 0.0f;
    color_blend_state_create_info.blendConstants[2] = 0.0f;
    color_blend_state_create_info.blendConstants[3] = 0.0f;
    VkPipelineLayout pipeline_layout;
    VkPipelineLayoutCreateInfo pipeline_layout_create_info;
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pNext = NULL;
    pipeline_layout_create_info.flags = 0;
    pipeline_layout_create_info.setLayoutCount = 0;
    pipeline_layout_create_info.pSetLayouts = NULL;
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = NULL;
    result = vkCreatePipelineLayout(device, &pipeline_layout_create_info, NULL, &pipeline_layout);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create pipeline layout.", &cleanup))
        return -8;
    cleanup.pipeline_layout = &pipeline_layout;

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info;
    graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.pNext = NULL;
    graphics_pipeline_create_info.flags = 0;
    graphics_pipeline_create_info.stageCount = 2;
    graphics_pipeline_create_info.pStages = shader_stages;
    graphics_pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
    graphics_pipeline_create_info.pTessellationState = NULL;
    graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
    graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    graphics_pipeline_create_info.pMultisampleState = &multisample_state_create_info;
    graphics_pipeline_create_info.pDepthStencilState = NULL;
    graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    graphics_pipeline_create_info.pDynamicState = NULL;
    graphics_pipeline_create_info.layout = pipeline_layout;
    graphics_pipeline_create_info.renderPass = render_pass;
    graphics_pipeline_create_info.subpass = 0;
    graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_create_info.basePipelineIndex = -1;
    VkPipeline graphics_pipeline;
    result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, NULL, &graphics_pipeline);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create graphics pipeline", &cleanup))
        return -8;
    cleanup.graphics_pipeline = &graphics_pipeline;

    VkFramebuffer* swapchain_framebuffers = (VkFramebuffer*) SDL_malloc(swapchain_image_count * sizeof(VkFramebuffer));
    for (Uint32 i=0; i<swapchain_image_count; i++) {
        VkImageView attachments[1];
        attachments[0] = swapchain_image_views[i];
        VkFramebufferCreateInfo framebuffer_create_info;
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.pNext = NULL;
        framebuffer_create_info.flags = 0;
        framebuffer_create_info.renderPass = render_pass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = attachments;
        framebuffer_create_info.width = swapchain_extent.width;
        framebuffer_create_info.height = swapchain_extent.height;
        framebuffer_create_info.layers = 1;
        result = vkCreateFramebuffer(device, &framebuffer_create_info, NULL, &swapchain_framebuffers[i]);
        if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create framebuffer", &cleanup))
            return -9;
    }
    cleanup.swapchain_framebuffers = swapchain_framebuffers;

    VkCommandPool command_pool;
    VkCommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = NULL;
    command_pool_create_info.flags = 0;
    command_pool_create_info.queueFamilyIndex = graphics_queue_family;
    result = vkCreateCommandPool(device, &command_pool_create_info, NULL, &command_pool);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create command_pool", &cleanup))
        return -9;
    cleanup.command_pool = &command_pool;
    VkCommandBuffer* command_buffers = (VkCommandBuffer*) SDL_malloc(swapchain_image_count * sizeof(VkCommandBuffer));
    VkCommandBufferAllocateInfo command_buffer_allocate_info;
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.pNext = NULL;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = swapchain_image_count;
    result = vkAllocateCommandBuffers(device, &command_buffer_allocate_info, command_buffers);
    if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not allocate command buffers", &cleanup))
        return -9;
    for (Uint32 i=0; i<swapchain_image_count; i++) {
        VkCommandBufferBeginInfo command_buffer_begin_info;
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.pNext = NULL;
        command_buffer_begin_info.flags = 0;
        command_buffer_begin_info.pInheritanceInfo = NULL;
        result = vkBeginCommandBuffer(command_buffers[i], &command_buffer_begin_info);
        if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not begin command buffers", &cleanup))
            return -9;
        VkRenderPassBeginInfo render_pass_begin_info;
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.pNext = NULL;
        render_pass_begin_info.renderPass = render_pass;
        render_pass_begin_info.framebuffer = swapchain_framebuffers[i];
        render_pass_begin_info.renderArea.offset.x = 0;
        render_pass_begin_info.renderArea.offset.y = 0;
        render_pass_begin_info.renderArea.extent = swapchain_extent;
        VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
        render_pass_begin_info.clearValueCount = 1;
        render_pass_begin_info.pClearValues = &clear_color;
        vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
        vkCmdDraw(command_buffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(command_buffers[i]);
        result = vkEndCommandBuffer(command_buffers[i]);
        if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not end command buffers", &cleanup))
            return -9;
    }

    SDL_Log("Creating Synchronisation Elements\n");
    VkSemaphore image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphoreCreateInfo semaphore_create_info;
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_create_info.pNext = NULL;
    semaphore_create_info.flags = 0;
    VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];
    VkFence* images_in_flight = (VkFence*) calloc(swapchain_image_count, sizeof(VkFence));
    VkFenceCreateInfo fence_create_info;
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.pNext = NULL;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (Uint32 i=0; i<MAX_FRAMES_IN_FLIGHT; i++) {
        result = vkCreateSemaphore(device, &semaphore_create_info, NULL, &image_available_semaphores[i]);
        if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create image semaphore", &cleanup))
            return -9;
        result = vkCreateSemaphore(device, &semaphore_create_info, NULL, &render_finished_semaphores[i]);
        if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create image semaphore", &cleanup))
            return -9;
        result = vkCreateFence(device, &fence_create_info, NULL, &in_flight_fences[i]);
        if (check_error(result != VK_SUCCESS, "Error in Vulkan Setup.", "Could not create fence", &cleanup))
            return -9;
    }
    cleanup.image_available_semaphores = image_available_semaphores;
    cleanup.render_finished_semaphores = render_finished_semaphores;
    cleanup.in_flight_fences = in_flight_fences;

    SDL_Log("Freeing memory stuffs\n");
#if DEBUG_BUILD
    SDL_free(validation_layers);
#endif
    SDL_Log("Freeing after debug\n");
    SDL_free(devices);
    SDL_free(device_extensions);
    SDL_free(present_modes);
    SDL_free(surface_formats);
    SDL_free(queue_families);
    SDL_free((char*)required_device_extensions);
    SDL_free(swapchain_images);
    SDL_free(fragment_spirv);
    SDL_free(vertex_spirv);
    SDL_free((char*)required_extensions);
    SDL_free(available_extensions);

    SDL_Log("Running Event Loop\n");
    SDL_Event e;
    Uint32 frame_index = 0;
    while (SDL_TRUE) {
        vkWaitForFences(device, 1, &in_flight_fences[frame_index], VK_TRUE, UINT64_MAX);
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT) {
            SDL_Log("Program quit after %i ticks", e.quit.timestamp);
            break;
        } 
        // draw frame.
        Uint32 image_index;
        result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_available_semaphores[frame_index], VK_NULL_HANDLE, &image_index);
        if (check_error(result != VK_SUCCESS, "Error in Rendering.", "Could not acquire next image", &cleanup))
            return -10;
        if (images_in_flight[image_index] != VK_NULL_HANDLE)
            vkWaitForFences(device, 1, &images_in_flight[image_index], VK_TRUE, UINT64_MAX);
        images_in_flight[image_index] = in_flight_fences[frame_index];
        VkSemaphore wait_semaphores[1];
        wait_semaphores[0] = image_available_semaphores[frame_index];
        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signal_semaphores[1];
        signal_semaphores[0] = render_finished_semaphores[frame_index];
        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = NULL;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffers[image_index];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_semaphores;
        vkResetFences(device, 1, &in_flight_fences[frame_index]);
        result = vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[frame_index]);
        if (check_error(result != VK_SUCCESS, "Error in Rendering.", "Could not submit to graphics queue", &cleanup))
            return -10;
        VkSwapchainKHR swapchains[1];
        swapchains[0] = swapchain;
        VkPresentInfoKHR present_info;
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.pNext = NULL;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapchains;
        present_info.pImageIndices = &image_index;
        present_info.pResults = NULL;
        result = vkQueuePresentKHR(presentation_queue, &present_info);
        if (check_error(result != VK_SUCCESS, "Error in Rendering.", "Could not present queue", &cleanup))
            return -10;
        frame_index = (frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    vkQueueWaitIdle(presentation_queue);
    vkDeviceWaitIdle(device);
    for (Uint32 i=0; i<MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, image_available_semaphores[i], NULL);
        vkDestroySemaphore(device, render_finished_semaphores[i], NULL);
        vkDestroyFence(device, in_flight_fences[i], NULL);
    }
    vkDestroyCommandPool(device, command_pool, NULL);
    for (Uint32 i=0; i<swapchain_image_count; i++)
        vkDestroyFramebuffer(device, swapchain_framebuffers[i], NULL); 
    vkDestroyPipeline(device, graphics_pipeline, NULL);
    vkDestroyPipelineLayout(device, pipeline_layout, NULL);
    vkDestroyRenderPass(device, render_pass, NULL);
    vkDestroyShaderModule(device, vertex_shader_module, NULL);
    vkDestroyShaderModule(device, fragment_shader_module, NULL);
    for (Uint32 i=0; i<swapchain_image_count; i++)
        vkDestroyImageView(device, swapchain_image_views[i], NULL); 
    vkDestroySwapchainKHR(device, swapchain, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);
    SDL_free(command_buffers);
    SDL_free(swapchain_image_views);
    SDL_free(swapchain_framebuffers);
    SDL_Log("Quitting Application\n");
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
