//
// Created by Gianni on 17/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_VULKAN_FUNCTIONS_HPP
#define VULKAN3DMODELVIEWER_VULKAN_FUNCTIONS_HPP

#include <vector>
#include <glfw/glfw3.h>
#include "vulkan_types.hpp"
#include "debug.hpp"


void createInstance(VulkanInstance& instance);
void destroyInstance(VulkanInstance& instance);

#endif //VULKAN3DMODELVIEWER_VULKAN_FUNCTIONS_HPP
