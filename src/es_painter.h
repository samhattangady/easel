/* 
 * es_painter is the rendering system for easel.
 */

#include "SDL.h"
#include <vulkan/vulkan.h>
#include "es_warehouse.h"

typedef struct {
    SDL_Window* window;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkDevice device;
    VkPhysicalDevice physical_device;
    VkSwapchainKHR swapchain;
    Uint32 swapchain_image_count;
    Uint32 frame_index;
    VkImageView* swapchain_image_views;
    VkShaderModule vertex_shader_module;
    VkShaderModule fragment_shader_module;
    VkPipelineLayout pipeline_layout;
    VkRenderPass render_pass;
    VkPipeline graphics_pipeline;
    VkFramebuffer* swapchain_framebuffers;
    VkCommandPool command_pool;
    VkSemaphore* image_available_semaphores;
    VkSemaphore* render_finished_semaphores;
    VkFence* in_flight_fences;
    VkFence* images_in_flight;
    VkQueue presentation_queue;
    VkCommandBuffer* command_buffers;
    VkQueue graphics_queue;
    SDL_bool buffer_resized;
} EsPainter;

extern SDL_bool painter_initialise(EsPainter* painter);
extern SDL_bool painter_paint_frame(EsPainter* painter);
extern void painter_cleanup(EsPainter* painter);
extern Sint64 painter_read_shader_file(const char* filename, Uint32** buffer);
