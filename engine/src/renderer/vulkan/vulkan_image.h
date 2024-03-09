#ifndef VULKAN_IMAGE_HEADER_INCLUDED
#define VULKAN_IMAGE_HEADER_INCLUDED

#include "./vulkan_types.inl"

namespace ns::vulkan {

/**
 * Create an image
 * @param context The Vulkan context.
 * @param image_type Image type.
 * @param width Image width.
 * @param height Image height.
 * @param format Image format.
 * @param tiling Image tiling.
 * @param usage Image usage.
 * @param memory_flags The memory properties of the image.
 * @param create_view Whether to create an image view.
 * @param view_aspect_flags Aspect flags of the view.
 * @param out_image The created image.
 */
void image_create(Context *context, VkImageType image_type, u32 width,
                  u32 height, VkFormat format, VkImageTiling tiling,
                  VkImageUsageFlags usage, VkMemoryPropertyFlags memory_flags,
                  bool create_view, VkImageAspectFlags view_aspect_flags,
                  Image *out_image);

/**
 * Create an image view
 * @param context The Vulkan context.
 * @param format The format of the image.
 * @param image Image to create the view for.
 * @param aspect_flags Aspect flags of the view.
 * @param out_view The created image view.
 */
void image_view_create(Context *context, VkFormat format, Image *image,
                       VkImageAspectFlags aspect_flags);

/**
 * Transition an image from old_layout to new_layout
 * @param context The Vulkan context.
 * @param command_buffer Command buffer to use.
 * @param image Image to transition.
 * @param format Format of the image.
 * @param old_layout Old layout of the image.
 * @param new_layout New layout of the image.
 */
void image_transition_layout(Context *context, CommandBuffer *command_buffer,
                             Image *image, VkFormat format,
                             VkImageLayout old_layout,
                             VkImageLayout new_layout);

/**
 * Copy data in buffer to provided image.
 * @param context The Vulkan context.
 * @param image Image to copy the buffer's data to.
 * @param buffer Buffer whose data will be copied.
 * @param command_buffer Command buffer to use.
 */
void image_copy_from_buffer(Context *context, Image *image, VkBuffer buffer,
                            CommandBuffer *command_buffer);

/**
 * Destroy an image
 * @param context The Vulkan context.
 * @param image The image to destroy.
 */
void image_destroy(Context *context, Image *image);

} // namespace ns::vulkan

#endif // VULKAN_IMAGE_HEADER_INCLUDED
